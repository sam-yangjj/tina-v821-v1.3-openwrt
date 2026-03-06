/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "tuya_unpack.h"
#include "notifier.h"
#include "swupgrade_progress.h"
#include "swupgrade_network.h"

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>

//#define SWUPGRADE_ASYNC_TEST	1

#define MAX_IMAGE_FNAME 512
#define min(a,b) (((a)<(b))?(a):(b))



static pthread_t network_daemon;

static void usage(char *programname)
{
	char ver[32] = {0};
	tuya_upgrade_version(ver);
	fprintf(stdout, "%s (compiled %s)\n", programname, __DATE__);
	fprintf(stdout, "Version %s \n", ver);
	fprintf(stdout, "Usage %s [OPTION]\n", programname);
	fprintf(stdout,
			" -h, --help              : print this help and exit\n"
			" -d, --dump              : dump ota firmware information\n"
			" -e, --extract           : extract zones from ota firmware\n"
			" -p, --path              : extract zones to path\n"
			" -f, --file              : local upgrade\n"
			" -u, --url               : http upgrade\n"
			" -r, --reboot            : reboot when upgrade done\n"
			" -c, --cleanup           : cleanup fw file when upgrade done\n"
			" -n, --network           : upgrade by ipc\n"
			" -m, --mandate           : ignore version number\n"
			" -s, --sign              : verify signature\n"
			" -k, --packets           : packets ota\n"
			" -v, --version_check     : version check\n"
	       );
}

static struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"dump", required_argument, NULL, 'd'},
	{"extract", required_argument, NULL, 'e'},
	{"path", optional_argument, NULL, 'p'},
	{"file", required_argument, NULL, 'f'},
	{"url", required_argument, NULL, 'u'},
	{"reboot", no_argument, NULL, 'r'},
	{"cleanup", no_argument, NULL, 'c'},
	{"network", no_argument, NULL, 'n'},
	{"mandate", no_argument, NULL, 'm'},
	{"sign", no_argument, NULL, 's'},
	{"packects", no_argument, NULL, 'k'},
	{"version_check", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};

static pthread_t start_thread(void *(* start_routine) (void *), void *arg)
{
	int ret;
	pthread_t id;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	//pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024 * 1024);

	ret = pthread_create(&id, &attr, start_routine, arg);
	if (ret) {
		ERROR("create thread failed.");
		return -1;
	}

	pthread_attr_destroy(&attr);

	return id;
}

#define STM_FEED_LEN	2048

static int upgrade_status = 0;

static int stream_upgrade_status(int percent, void *user_data)
{
	TRACE("-------> %d", percent);
	upgrade_status = percent;
	return 0;
}

static int stream_upgrade_ota(char *fw_name)
{
    int fw_fd;
	char *buf = NULL;
	int len, blocksz;
	int ret;
	ota_stream *obj = NULL;
	int nonblock = 0;

	printf("start stream upgrade ota test.\n");

#ifdef SWUPGRADE_ASYNC_TEST
    nonblock = 1;
#endif

    fw_fd = open(fw_name, O_RDONLY);
    if (fw_fd < 0) {
        ERROR("FW %s not found: %d", fw_name, errno);
        return -ENOENT;
    }

	obj = device_upgrade_open(!nonblock, stream_upgrade_status, NULL);
	if (!obj) {
		close(fw_fd);
		ERROR("upgrade init failed.");
		return -1;
	}

	TRACE("before streaming...\n");

	while (1) {
        srand((unsigned)time(NULL));
        len = rand()%(64*1024);
        //len = len*1024;
        if(!len)
            continue;
        printf("%s %d len=%d\n", __func__, __LINE__, len);
        buf = calloc(1, len);
        if (!buf)
        {
            printf("%s %d malloc fail\n", __func__, __LINE__);
            ret = -ENOMEM;
            goto OUT_ONE;
        }

        ret = produce_stream(fw_fd, buf, len);
		if (ret <= 0) {
			break;
		}

        TRACE("device_upgrade_feed ...");
		ret = device_upgrade_feed(obj, buf, ret);
		if (ret) {
			ERROR("upgrade feed failed.");
			break;
		}
        free(buf);
        buf=NULL;
	}

    if(buf)
        free(buf);
OUT_ONE:
    //upgrade_done(&obj->img);    
	device_upgrade_close(obj);

#if 0
	while (upgrade_status >= 0 && upgrade_status < 100) {
		sleep(2);
		TRACE("waitting upgrade thread done");
	}
#endif

	return ret;
}

static int stream_interface_parse_head(char *fw_name)
{
	int fw_fd;	
    char *buf = NULL;
	int len;
	int ret;
    unsigned char *magic;

	fw_fd = open(fw_name, O_RDONLY);
	if (fw_fd < 0) {
		ERROR("FW %s not found: %d", fw_name, errno);
		return -ENOENT;
	}

	len = STM_FEED_LEN;
	buf = calloc(1, len);
	if (!buf)
		return -ENOMEM;

	ret = read(fw_fd, buf, len);
	if (ret < 0) {
		ERROR("Failure in read %d: %s", fw_fd, strerror(errno));
		free(buf);
		ret = -EFAULT;
        return ret;
	} else if (ret == 0) {
		free(buf);
		TRACE("read all of the file");
        return ret;
	}

    magic = (unsigned char *)buf;

    if(!strncmp(magic, "TUYA_OTA", 8))
    {
        //TODO
        ret=1;
    }
    else if(!strcmp(magic, "DIFF"))
    {
        ret=2;
    }
    else
    {
        ret=0;
		printf("invalid update file magic!\n");
    }

	free(buf);
    close(fw_fd);
    return ret;
}

static int stream_interface_test(char *fw_name)
{
	int fw_fd;
	char *buf = NULL;
	int len;
	int ret;
	ota_stream *obj = NULL;
	int nonblock = 0;

	printf("start stream upgrade test");


    ret = stream_interface_parse_head(fw_name);
    switch (ret)
    {
        case 1:
            stream_upgrade_ota(fw_name);
            break;
        case 2:
            break;
        default:
            return -1;
    }

#if 0
	while (upgrade_status >= 0 && upgrade_status < 100) {
		sleep(2);
		TRACE("waitting upgrade thread done");
	}
#endif

	return 0;
}

int main(int argc, char **argv)
{
	int opt_f = 0;
	int opt_d = 0;
	int opt_e = 0;
	int opt_p = 0;
	int opt_u = 0;
	int opt_r = 0;
	int opt_c = 0;
	int opt_n = 0;
	int opt_m = 0;
	int opt_s = 0;
	int opt_k = 0;
	int opt_v = 0;
	int c;
	int ret = -1;
	char fw_fname[MAX_IMAGE_FNAME] = {0};
	char fw_url[MAX_IMAGE_FNAME] = {0};
	char dst_path[MAX_IMAGE_FNAME] = {0};

	while ((c = getopt_long(argc, argv, "smrcnhkve:d:f:u:p:",
					long_options, NULL)) != EOF) {
		switch (c) {
			case 'f':
				strncpy(fw_fname, optarg, sizeof(fw_fname));
				opt_f = 1;
				break;
			case 'u':
				strncpy(fw_url, optarg, sizeof(fw_url));
				opt_u = 1;
				break;
			case 'd':
				strncpy(fw_fname, optarg, sizeof(fw_fname));
				opt_d = 1;
				break;
			case 'e':
				strncpy(fw_fname, optarg, sizeof(fw_fname));
				opt_e = 1;
				break;
			case 'p':
				strncpy(dst_path, optarg, sizeof(dst_path));
				opt_p = 1;
				break;
			case 'n':
				opt_n = 1;
				break;
			case 'c':
				opt_c = 1;
				break;
			case 'r':
				opt_r = 1;
				break;
			case 'm':
				opt_m = 1;
				break;
			case 's':
				opt_s = 1;
				break;
			case 'v':
				opt_v = 1;
				break;
			case 'k':
				opt_k = 1;
				break;

			case 'h':
				usage(argv[0]);
				exit(0);
				break;
			default:
				usage(argv[0]);
				exit(1);
				break;
		}
	}

	if (opt_u == 0 && opt_f == 0 && opt_d == 0 && opt_e == 0 && opt_n == 0) {
		usage(argv[0]);
		exit(1);
	}

	if (opt_v) {
		/* Set version compare  mode */
                ret = set_version_compare_mode(1);
	}else {
	
                ret = set_version_compare_mode(0);
	}

#ifndef TUYA_UNSIGN_PACK
	ret = set_verify_sign_mode(1);
#else
	ret = set_verify_sign_mode(0);
#endif

	if (opt_s) {
		/* Set verification signature mode */
		ret = set_verify_sign_mode(1);
	}else {
		ret = set_verify_sign_mode(0);
	}

	if (opt_u) {
		progress_post *post;
		ret = install_from_url(fw_url, NULL);
	}

	if (opt_f) {
            if(opt_k) {
	        ret = stream_interface_test(fw_fname);
	    }else{
	        ret = install_from_file(fw_fname); 
	    }
	}

	if (opt_d) {
		ret = tuya_upgrade_dump(fw_fname);
	}

	if (opt_e) {
		ret = tuya_upgrade_extract(fw_fname, dst_path);
	}

	//if (opt_f || opt_n)
	if (opt_n)
		start_thread(progress_bar_thread, NULL);

	if (opt_n) {
		network_daemon = start_thread(network_initializer, NULL);
		pthread_join(network_daemon, NULL);
        }

	if (opt_f && opt_c && !ret) {
		remove(fw_fname);
	}

	if (opt_r && (opt_u || opt_f) && !ret)
		system("reboot -f");

	return ret;
}
