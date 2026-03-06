/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>

#include "tuya_unpack.h"
#include "tuya_verify.h"
#include "property.h"
#include "notifier.h"
#include "handler.h"
#include "http/http_inf.h"
#include "swupgrade_async.h"

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>

#define UNPACK_VER	"1.0.3"

#define minlen(a,b) ((a)>(b)?(b):(a))

#include "bsp_sha256.h"
/*#include "sha256.h"*/
/* for img write ahead check */
static sha2_context sha2_ctx;
static unsigned char img_hmac[32];
static sha2_context sha2_ctx_3;
static unsigned char img_hmac_3[32];
static int imgversion_flag=0;

static int initflag=0;
static int allocflag=0;

int loglevel = TRACELEVEL;

// common staff
#define TUYA_STREAM_BUFF_SIZE   (128*1024)

struct tuya_upgrade_stream_buff{
    int legacy_size;
    int legacy_cnt;
    int legacy_offset;
    char *legacy_buff;
};
static struct tuya_upgrade_stream_buff streambuf;
static int sizeunit=0;
static unsigned int streamcnt=0;

static int extract_image_to_file(char *buf, int len, char *dst_file)
{
	int fd;
	int ret;

	if (!buf || len <= 0 || !dst_file)
		return -1;

	remove(dst_file);

	fd = open(dst_file, O_WRONLY|O_CREAT, 0600);
	if (fd < 0) {
		ERROR("open file %s failed. errno %d", dst_file, errno);
		return -ENOENT;
	}

	ret = write(fd, buf, len);
	if (ret != len) {
		ERROR("wirte file %s failed. errno %d", dst_file, errno);
		return -1;

	}

	close(fd);

	TRACE("extract img to %s OK", dst_file);

	return 0;
}

static int tuya_verify_img(char *buf, int buf_len, unsigned char sum[32])
{
	if (get_verify_sign_mode() == 0)
		return 0;

	unsigned char tmp[32] = {0};
	//sha256sum(buf, buf_len, tmp);
	bsp_verify_sha256(buf, buf_len, tmp);

	if (memcmp(sum, tmp, 32) == 0)
		return 0;
	else
		return -1;
}

static int tuya_check_file(char *buf, int buf_len, unsigned char sum[32])
{
	return tuya_verify_img(buf, buf_len, sum);
}

int get_max_zone_size(ota_head *phead)
{
	int i;
	int size = 0;
	int temp = 0;

	for (i = 0; i < phead->zone_num; i++) {
		temp = phead->zone[i].image_size;
		size = (temp > size)?temp:size;
	}

	return size;
}

static int compare_version(char *ver_dev, char *ver_img)
{
	char *vd = ver_dev;
	char *vi = ver_img;
	int loop = 3;
	int cmp = 0;

	if (!ver_dev || !ver_img || strlen(ver_dev) < 5 || strlen(ver_img) < 5)
		return 0;

	while (loop--) {
		cmp = atoi(vd) - atoi(vi);
		if (cmp > 0)
			return -1;
		else if (cmp < 0)
			return 0;
		vd = strchr(vd, '.') + 1;
		vi = strchr(vi, '.') + 1;
	}

	return -1;
}

int cmp_img_version(ota_zone *pzone)
{
	int ret;
	char key[PROP_MAX_LEN] = {0};
	char value[PROP_MAX_LEN] = {0};

	if (!strncmp(pzone->name, "backup", 32))
		return -1;

	/* if not set, ingore version compare */
	if(get_version_compare_mode() <= 0) {
            return 0;	
	}

	snprintf(key, PROP_MAX_LEN, "persist.%s.version", pzone->name);

	ret = prop_get(key, value);
	if (ret < 0)
		return 0;

	return compare_version(value, pzone->version);
}


int update_img_version(ota_zone *pzone)
{
	char key[PROP_MAX_LEN] = {0};
	char value[PROP_MAX_LEN] = {0};

	snprintf(key, PROP_MAX_LEN, "persist.%s.version", pzone->name);
	if (strlen(pzone->version) > 0)
		snprintf(value, PROP_MAX_LEN, "%s", pzone->version);
	else
		snprintf(value, PROP_MAX_LEN, "%s", "0.0.0");
	prop_set(key, value);

	return 0;
}

static void get_real_device_path(char *s)
{
	char tmp_s[64] = {0};

	if (!strncmp(s, "/dev/", 5))
		return ;

	snprintf(tmp_s, sizeof(tmp_s), "/dev/%s", s);

	strncpy(s, tmp_s, 64);
}

// single-copy staff

static int find_backup_zone_idx(ota_image *pimg)
{
	int i;
	char *name;

	for (i = 0; i < pimg->head.zone_num; i++) {
		name = pimg->head.zone[i].name;
		if (!strncmp(name, "backup", 32))
			return i;
	}

	return -1;
}

static int set_boot_restore_marker(int dst, int src, int len)
{
	char value[PROP_MAX_LEN] = {0};

	snprintf(value, PROP_MAX_LEN, "%d", dst);
	prop_set("persist.ota.dst", value);
	snprintf(value, PROP_MAX_LEN, "%d", src);
	prop_set("persist.ota.src", value);
	snprintf(value, PROP_MAX_LEN, "%d", len);
	prop_set("persist.ota.len", value);

	return 0;
}

int single_copy_handler(ota_image *pimg)
{
	struct installer_handler *hnd;
	int ret;
	int bk_idx;
	int idx;
	int offs = 0;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1; 
	}

	idx = pimg->zone_idx;

	/* write backup first */
	bk_idx = find_backup_zone_idx(pimg);
	if (bk_idx < 0) {
		TRACE("find backup zone failed.");
		return -1;
	}
	get_real_device_path(pimg->head.zone[bk_idx].device[0]);
	ret = hnd->installer(pimg->head.zone[bk_idx].device[0], 0, pimg);
	if (ret != 0) {
                TRACE("install %s to backup failed.", pimg->head.zone[idx].name);
		return -1;
        }

	/* set restore marker */
	set_boot_restore_marker(
		pimg->head.zone[idx].offset,
		pimg->head.zone[bk_idx].offset,
		pimg->head.zone[idx].image_size);
 
	if (pimg->head.zone[idx].copy_in_boot) {
		update_img_version(&pimg->head.zone[idx]);
		return 0;
	}

	/* write real zone */
	offs = pimg->head.zone[idx].device_offset;
	get_real_device_path(pimg->head.zone[idx].device[0]);
	ret = hnd->installer(pimg->head.zone[idx].device[0], offs, pimg);
	if (ret != 0) {
                TRACE("install %s to %s failed.", pimg->head.zone[idx].name,
				pimg->head.zone[idx].device[0]);
		return -1;
        }

	update_img_version(&pimg->head.zone[idx]);

	/* clear restore marker */
	set_boot_restore_marker(0, 0, 0);

        return 0;
}

// double-copy staff

static int get_active_slot()
{
	int ret;
	char value[PROP_MAX_LEN] = {0};

	ret = prop_get("persist.ota.slot", value);
	if (ret < 0) {
                TRACE("use default slot 0.");
		return 0;
	}

	return atoi(value);
}

static int invert_active_slot()
{
	int slot;
	char value[PROP_MAX_LEN] = {0};

	slot = get_active_slot();
	slot = slot?0:1;

	snprintf(value, PROP_MAX_LEN, "%d", slot);

	prop_set("persist.ota.slot", value);

	return 0;
}

int double_copy_handler(ota_image *pimg)
{
	struct installer_handler *hnd;
	int slot = 0;
	int idx;
	int ret;
	int offs = 0;

	idx = pimg->zone_idx;
	slot = get_active_slot();
	slot = slot?0:1;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1; 
	}

	offs = pimg->head.zone[idx].device_offset;
	get_real_device_path(pimg->head.zone[idx].device[slot]);
	ret = hnd->installer(pimg->head.zone[idx].device[slot], offs, pimg);
	if (ret != 0) {
		TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
	}

	update_img_version(&pimg->head.zone[idx]);

	return 0;
}

// recovery staff

static int get_active_system()
{
	int ret;
	char value[PROP_MAX_LEN] = {0};

	ret = prop_get("in.recovery.mode", value);
	if (ret < 0) {
        TRACE("%s: get active system failed.", __func__);
		return -1;
	}

	if (strncmp(value, "true", 32))
		return 0;
	else
		return 1;
}

/* non persist value, must be set at system start up */
static int set_boot_recovery_marker(int mark)
{
	prop_set("persist.boot.recovery", mark?"true":"false");

	return 0;
}

int recovery_handler(ota_image *pimg)
{
	struct installer_handler *hnd;
	int mode;
	int idx;
	int ret;
	int offs;
	char *name;

	idx = pimg->zone_idx;
	name = pimg->head.zone[idx].name;
	mode = get_active_system();

	if ((mode && !strncmp(name, "recovery", 32)) ||
			(!mode && strncmp(name, "recovery", 32))) {
		TRACE("mode: %d name: %s", mode, name);
		return 0;
	}

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1;
	}

	offs = pimg->head.zone[idx].device_offset;
	get_real_device_path(pimg->head.zone[idx].device[0]);
	ret = hnd->installer(pimg->head.zone[idx].device[0], offs, pimg);
	if (ret != 0) {
                TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
        }

	update_img_version(&pimg->head.zone[idx]);

	/* in normal system, recovey zone write ok, reboot to recovery */
	if (!strncmp(name, "recovery", 32)) {
		set_boot_recovery_marker(1);
                TRACE("reboot to recovery");
		system("reboot -f");
	}

	return 0;
}

static void upgrade_done(ota_image *pimg)
{
	char *upg_mode;

	upg_mode = pimg->head.ota_mode;

	if (!strncmp(upg_mode, "double-copy", 32))
		invert_active_slot();

	if (!strncmp(upg_mode, "recovery", 32))
		set_boot_recovery_marker(0);

	TRACE("ota ok\n");
}

int install_single_image(ota_image *pimg)
{
	char *upg_mode;

	upg_mode = pimg->head.ota_mode;

	INFO("------> flash type: [%s] ", pimg->head.flash_type);
	INFO("------> in [%s] mode, installing [%s]", upg_mode,
			pimg->head.zone[pimg->zone_idx].name);

	if (!strncmp(upg_mode, "single-copy", 32))
		return single_copy_handler(pimg);

	if (!strncmp(upg_mode, "double-copy", 32))
		return double_copy_handler(pimg);

	if (!strncmp(upg_mode, "recovery", 32))
		return recovery_handler(pimg);

	return -1;
}

static int excute_shell_cmd(const char* cmd, char* buffer, int* len)
{
	FILE *fp = NULL;
	int i = 0;

	if (!cmd || !buffer || !len)
		return -1;

	fp = popen(cmd, "r");
	if (fp == NULL)
		return -1;

	while (fgets(buffer, *len, fp) != NULL) {
		// should do something
	}

	for (i = 0; i < *len; i++) {
		if (buffer[i] == '\n' || buffer[i] == '\r') {
			buffer[i] = '\0';
			break;
		}
	}

	buffer[*len - 1] = '\0';
	*len = strlen(buffer);

	pclose(fp);
	fp = NULL;

	return 0;
}

int install_single_file(char *buf, ota_file *file)
{
	int ret;
	char cmd[256] = {0};
	char buffer[128] = {0};
	int len = sizeof(buffer);

	if (!buf || !file || !file->image_size || !strlen(file->storage_path))
		return -1;
	
	ret = extract_image_to_file(buf, file->image_size, file->storage_path);
	if (ret < 0)
		return -1;

	snprintf(cmd, sizeof(cmd), "%s -v %s -f %s", file->installer,
			file->version, file->storage_path);
	return excute_shell_cmd(cmd, buffer, &len);
}

static int get_zone_size_by_index(ota_image *pimg, int idx)
{
	int i;
	int size = 0;
	int temp = 0;
	char *name = NULL;
	char *upg_mode;

	upg_mode = pimg->head.ota_mode;

	for (i = 0; i < idx; i++) {
		name = pimg->head.zone[i].name;
		if (!strncmp(name, "recovery", 32))
			continue;
		temp = pimg->head.zone[i].image_size;
		if (!strncmp(upg_mode, "single-copy", 32) &&
				!pimg->head.zone[i].copy_in_boot)
			temp += temp;
		size += temp;
	}

	return size;
}

static int update_progress(ota_image *pimg, int len)
{
	int idx;
	long long new_per;
	char *name;
	int size;
	
	if (pimg == NULL || len < 0)
		return -1;

	idx = pimg->zone_idx;
	name = pimg->head.zone[idx].name;

	if (!strncmp(name, "backup", 32) || !strncmp(name, "recovery", 32))
		return -1;

	if (pimg->total_len <= 0)
		return -1;

	size = get_zone_size_by_index(pimg, idx);

	new_per = size + len;
	if (!strncmp(pimg->head.ota_mode, "single-copy", 32) &&
			!pimg->head.zone[idx].copy_in_boot)
        new_per = new_per * 100 / (pimg->total_len - pimg->head.zone[idx].image_size);
	else
        new_per = new_per * 100 / pimg->total_len;

	return new_per;
}

void update_writein_progress(ota_image *pimg, int write_len)
{
	int new_per = 0;

	new_per = update_progress(pimg, write_len);

	//TRACE(" wr_percent %d new_per %d", pimg->wr_percent, new_per);

	if (pimg->wr_percent >= new_per)
		return ;

	pthread_mutex_lock(&pimg->prog_mutex);
	pimg->wr_percent = new_per;
	pthread_mutex_unlock(&pimg->prog_mutex);
}

void update_loadin_progress(ota_image *pimg, int load_len)
{
	int new_per = 0;

	new_per = update_progress(pimg, load_len);

	//TRACE(" ld_percent %d new_per %d", pimg->ld_percent, new_per);
	
	if (pimg->ld_percent >= new_per)
		return ;

	pthread_mutex_lock(&pimg->prog_mutex);
	pimg->ld_percent = new_per;
	pthread_mutex_unlock(&pimg->prog_mutex);
}

void *progress_notifier(void *data)
{
	ota_image *pimg = data;
	int new_per = 0;
	int old_per = 0;
	int timer = 0;

	while (!pimg->abort) {
		sleep(1);
		timer++;
		pthread_mutex_lock(&pimg->prog_mutex);
		new_per = (pimg->ld_percent + pimg->wr_percent) / 2;
		pthread_mutex_unlock(&pimg->prog_mutex);
		if (new_per > 100)
			new_per = 100;
		if (old_per >= new_per && timer < 50) {
			old_per = new_per;
			continue;
		}
		if (pimg->progress_cb)
			pimg->progress_cb(new_per, pimg->user_data);
		timer = 0;
		old_per = new_per;
	}

	/* send again in case of missing */
	if (new_per < 100) {
		pthread_mutex_lock(&pimg->prog_mutex);
		new_per = (pimg->ld_percent + pimg->wr_percent) / 2;
		pthread_mutex_unlock(&pimg->prog_mutex);
		if (pimg->progress_cb)
			pimg->progress_cb(new_per, pimg->user_data);
	}

    return data;
}

static pthread_t start_thread(void *(* start_routine) (void *), void *arg)
{
        int ret;
        pthread_t id;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setstacksize(&attr, 1024 * 1024);

        ret = pthread_create(&id, &attr, start_routine, arg);
        if (ret) {
                ERROR("create thread failed.");
                return -1;
        }

        pthread_attr_destroy(&attr);

        return id;
}

void start_progress_notifier(ota_image *pimg)
{
	pimg->abort = 0;

	pthread_mutex_init(&pimg->prog_mutex, NULL);

	pimg->progress_daemon = start_thread(progress_notifier, pimg);
}

void stop_progress_notifier(ota_image *pimg)
{
	pimg->abort = 1;

	pthread_join(pimg->progress_daemon, NULL);

	pthread_mutex_destroy(&pimg->prog_mutex);
}

// install by stream staff

ota_stream *device_upgrade_open(int block, ota_progress cb, void *user_data)
{
	int ret;
	ota_stream *obj;

	initflag=0;
	allocflag=0;

	printf("############ %s %d initflag=%d allocflag=%d\n", __func__, __LINE__, initflag, allocflag);
	obj = calloc(1, sizeof(ota_stream));
	if (!obj)
		return NULL;

	obj->non_block = !block;
	if (get_verify_sign_mode() == 1)
        obj->len_owe = OTA_HEAD_SIZE + sizeof(ota_sign);
	else
        obj->len_owe = OTA_HEAD_SIZE + sizeof(ota_sign); // for compatible old board

	obj->buf = calloc(1, obj->len_owe);
	if (!obj->buf) {
		free(obj);
		return NULL;
	}

	obj->img.progress_cb = cb;
	obj->img.user_data = user_data;
	start_progress_notifier(&obj->img);

	if (block)
		return obj;

	/* for non-block upgrade */
	ret = swupgrade_async_start(obj);
	if (ret) {
		ERROR("start async upgrade failed.");
		free(obj->buf);
		free(obj);
		return NULL;
	}

	return obj;
}


static int single_copy_handler_bk2real(ota_image *pimg, int bk_idx, int idx, int offset, int len, char *buf)
{
	struct installer_handler *hnd;
	int ret;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1;
	}
	ret = hnd->installer_bk(pimg, bk_idx, idx, offset, len, buf);
	if (ret != 0) {
        TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
    }

	return 0;
}

static int double_copy_handler_wh(ota_image *pimg, int offset, char *data, int len)
{
	struct installer_handler *hnd;
	int slot = 0;
	int idx;
	int ret;
	int offs = 0;

	idx = pimg->zone_idx;
    slot = get_active_slot();
	slot = slot?0:1;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1;
	}

	offs = pimg->head.zone[idx].device_offset + offset;
	get_real_device_path(pimg->head.zone[idx].device[slot]);
	ret = hnd->installer_wh(pimg->head.zone[idx].device[slot], pimg, offs, data, len);//(char *dev, ota_image *pimg, int dev_offs, char *data, int length)
	if (ret != 0) {
        TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
    }

	return 0;
}


#if 0
static int single_copy_handler_wh(ota_image *pimg, int bk_idx, int offset, char *data, int len)
{
	struct installer_handler *hnd;
	int idx;
	int ret;
	int offs = 0;
	int img_size = 0;

	idx = pimg->zone_idx;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1;
	}

	offs = pimg->head.zone[bk_idx].device_offset + offset;
	img_size = pimg->head.zone[idx].image_size;
	pimg->img_buf = data;
	pimg->head.zone[idx].image_size = len;
	get_real_device_path(pimg->head.zone[bk_idx].device[0]);
	ret = hnd->installer(pimg->head.zone[bk_idx].device[0], offs, pimg);
	if (ret != 0) {
        TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
    }

	pimg->head.zone[idx].image_size = img_size;

	return 0;
}
#else
static int single_copy_handler_wh(ota_image *pimg, int bk_idx, int offset, char *data, int len)
{
	struct installer_handler *hnd;
	int idx;
	int ret;
	int offs = 0;

	idx = pimg->zone_idx;

	hnd = find_handler(pimg);
	if (!hnd) {
		TRACE("ota mode %s not supported.", pimg->head.ota_mode);
		return -1;
	}

	offs = pimg->head.zone[bk_idx].device_offset + offset;
	//img_size = pimg->head.zone[idx].image_size;
	//pimg->img_buf = data;
	//pimg->head.zone[idx].image_size = len;
	get_real_device_path(pimg->head.zone[bk_idx].device[0]);
	ret = hnd->installer_wh(pimg->head.zone[bk_idx].device[0], pimg, offs, data, len);
	if (ret != 0) {
        TRACE("install %s failed.", pimg->head.zone[idx].name);
		return -1;
    }

	//pimg->head.zone[idx].image_size = img_size;

	return 0;
}
#endif

int produce_stream(int fd, char *buf, int len)
{
    int ret=0;

	ret = read(fd, buf, len);
	if (ret < 0) {
		ERROR("Failure in read %d: %s", fd, strerror(errno));
		ret = -EFAULT;
	} else if (ret == 0) {
		TRACE("read all of the file");
	}

    return ret;
}

static int set_flash_bitfilt_marker(char *dev)
{
       char value[PROP_MAX_LEN] = {0};

       snprintf(value, PROP_MAX_LEN, "%s", dev);
       prop_set("persist.ota.bitflit", value);

       return 0;
}

static int verify_otabin_onflash(ota_image *pimg, int blk_idx, char *buf, int image_size, char *sha256)
{
    char *blkdev;
    int blkfd;
    int left;
    int offset;
    int len, rdlen;
    char *rdbuf=NULL;
    int ret=-1;
    int erase_sz = 64*1024; // not care for read

/*    ERROR("###################On verify_otabin_onflash blk_idx=%d image_size=%d blkdev=%s", \
        blk_idx, image_size, pimg->head.zone[blk_idx].device[0]); */

    erase_sz = get_device_geometry(pimg->head.zone[blk_idx].device[0]);

    left = image_size;
    offset = pimg->head.zone[blk_idx].device_offset;

    blkdev =  pimg->head.zone[blk_idx].device[0];
    if (blkdev == NULL || pimg == NULL)
           return -1;

    if ((blkfd = open(blkdev, O_RDWR)) < 0) {
        ERROR("%s: %s: %s", __func__, blkdev, strerror(errno));
        return -ENODEV;
    }

    //
    lseek(blkfd, offset, SEEK_SET);

    memset(img_hmac_3, 0, 32);
    bsp_sha2_starts(&sha2_ctx_3, 0);

    len = erase_sz;
    while(left)
    {
        if (left < erase_sz)
            len = left;

        rdlen = len;
        rdbuf = buf;

        //mtd_read
        while (rdlen) {
            errno = 0;
            ret = read(blkfd, (void *)rdbuf, rdlen);
            if (ret < 0) {
                if (errno == EINTR)
                    continue;
                ERROR("cannot read %d bytes: %s", rdlen, strerror(errno));
                ret = -1;
                break;
            }

            if (ret == 0) {
                ERROR("cannot read %d bytes: %s", rdlen, strerror(errno));
                ret = -1;
                break;
            }

            rdlen -= ret;
            rdbuf += ret;
        }

        if (ret < 0) {
            ERROR("read fail! ");
            ret = -ENOSPC;
            goto erase_out;
        }

        bsp_sha2_update(&sha2_ctx_3, (unsigned char *)buf, len);

        left -= (len);

    }

	bsp_sha2_finish(&sha2_ctx_3, img_hmac_3);



    if (memcmp(sha256, img_hmac_3, 32) != 0) {
        ERROR("bad hmac, abort upgrade! ret=%d", ret);
        ret = -ENOSPC;
        set_flash_bitfilt_marker(blkdev);
    }
    else
    {
        ret=0;
    }

erase_out:
	close(blkfd);
    return ret;
}

int install_single_image_wh(ota_stream *obj, char *data, int data_len)
{
	int cpy_len = 0;
	int bk_idx;
	int idx;
    int ret;

	idx = obj->img.zone_idx;

	/* write backup first */
	bk_idx = find_backup_zone_idx(&obj->img);
	if (bk_idx < 0) {
		TRACE("find backup zone failed.");
		return -1;
	}
	
	if (obj == NULL || data == NULL || data_len <= 0)
		return 0;

	if (obj->status == GET_IMG && obj->len_earn == 0) {
		memset(img_hmac, 0, 32);
		bsp_sha2_starts(&sha2_ctx, 0);
	}

	if (obj->len_owe <= data_len)
		cpy_len = obj->len_owe;
	else
		cpy_len = data_len;

	if (obj->status == GET_IMG) {
		bsp_sha2_update(&sha2_ctx, (unsigned char *)data, cpy_len);
		if (cmp_img_version(&obj->img.head.zone[obj->img.zone_idx]) == 0)
		{
        	/* write backup first */
            if (single_copy_handler_wh(&(obj->img), bk_idx, obj->len_earn, data, cpy_len) < 0)
				return -1;
        }
        else
        {
            imgversion_flag=1;
        }
	} else {
		memcpy(obj->buf + obj->len_earn, data, cpy_len);
	}
	obj->len_owe -= cpy_len;
	obj->len_earn += cpy_len;
	
	update_loadin_progress(&obj->img, obj->len_earn);

	if (obj->status == GET_IMG) {
		update_writein_progress(&obj->img, obj->len_earn);
		if (obj->len_owe == 0) {
            if(imgversion_flag==1)
            {
                printf("%s %d skip upgrade %s\n", __func__, __LINE__, obj->img.head.zone[idx].name);
                imgversion_flag=0;
                return 0;
            }
			bsp_sha2_finish(&sha2_ctx, img_hmac);
			if (memcmp(obj->img.head.zone[obj->img.zone_idx].sha256, img_hmac, 32) != 0) {
				ERROR("bad hmac, abort upgrade!");
				return -1;
			} else {
                if(verify_otabin_onflash(&(obj->img), bk_idx, data, obj->img.head.zone[idx].image_size, obj->img.head.zone[idx].sha256))
                {
                    return -1;
                }

                //update_img_version(&(obj->img.head.zone[obj->img.zone_idx]));
            	/* set restore marker */
            	set_boot_restore_marker(
            		obj->img.head.zone[idx].offset,
            		obj->img.head.zone[bk_idx].offset,
            		obj->img.head.zone[idx].image_size);
             
            	if (obj->img.head.zone[idx].copy_in_boot) {
            		update_img_version(&obj->img.head.zone[idx]);
                    TRACE("copy_in_boot=1");
            		return 0;
            	}

                /* write real zone */
                ret = single_copy_handler_bk2real(&(obj->img), bk_idx, idx, obj->img.head.zone[idx].device_offset, obj->img.head.zone[idx].image_size, data);
                if(ret)
                {
                    TRACE("single_copy_handler_bk2real ret=-1");
                    return ret;
                }

                if(verify_otabin_onflash(&(obj->img), idx, data, obj->img.head.zone[idx].image_size, obj->img.head.zone[idx].sha256))
                {
                    return -1;
                }

            	update_img_version(&obj->img.head.zone[idx]);

            	/* clear restore marker */
            	set_boot_restore_marker(0, 0, 0);

			}
		}
	}


	return cpy_len;
}

int install_double_image_wh(ota_stream *obj, char *data, int data_len)
{
	int cpy_len = 0;
	
	if (obj == NULL || data == NULL || data_len <= 0)
		return 0;

	if (obj->status == GET_IMG && obj->len_earn == 0) {
		memset(img_hmac, 0, 32);
		bsp_sha2_starts(&sha2_ctx, 0);
	}

	if (obj->len_owe <= data_len)
		cpy_len = obj->len_owe;
	else
		cpy_len = data_len;

	if (obj->status == GET_IMG) {
		bsp_sha2_update(&sha2_ctx, (unsigned char *)data, cpy_len);
		if (cmp_img_version(&obj->img.head.zone[obj->img.zone_idx]) == 0)
		{
            if (double_copy_handler_wh(&(obj->img), obj->len_earn, data, cpy_len) < 0)
				return -1;
        }
	} else {
		memcpy(obj->buf + obj->len_earn, data, cpy_len);
	}
	obj->len_owe -= cpy_len;
	obj->len_earn += cpy_len;
	
	update_loadin_progress(&obj->img, obj->len_earn);

	if (obj->status == GET_IMG) {
		update_writein_progress(&obj->img, obj->len_earn);
		if (obj->len_owe == 0) {
			bsp_sha2_finish(&sha2_ctx, img_hmac);
			if (memcmp(obj->img.head.zone[obj->img.zone_idx].sha256, img_hmac, 32) != 0) {
				ERROR("bad hmac, abort upgrade!");
				return -1;
			} else {
                if(verify_otabin_onflash(&(obj->img), obj->img.zone_idx, data, obj->img.head.zone[obj->img.zone_idx].image_size, obj->img.head.zone[obj->img.zone_idx].sha256))
                {
                    return -1;
                }
				update_img_version(&(obj->img.head.zone[obj->img.zone_idx]));
			}
		}
	}

	return cpy_len;
}

int consume_stream(ota_stream *obj, char *data, int data_len)
{
    char *upg_mode;

    upg_mode = obj->img.head.ota_mode;

	if (!strncmp(upg_mode, "single-copy", 32))
		return install_single_image_wh(obj, data, data_len);

	if (!strncmp(upg_mode, "double-copy", 32))
		return install_double_image_wh(obj, data, data_len);

	//if (!strncmp(upg_mode, "recovery", 32))
	//	return install_recovery_image_wh(obj, data, data_len);

	return -1;
}

int get_device_geometry(char *dev)
{
    int dev_fd;
    int ret;
    struct mtd_info_user mtd;
    char *string;
    const char ch = '/';

    string = strrchr((const char *)dev, ch);

    string++;
    if(!strncmp(string, "mtd", 3))
    {

        /* get some info about the flash device */
            dev_fd = open (dev, O_RDONLY);
            if (dev_fd < 0)
            {
                ERROR("MTDDEV %s not found: %d", dev, errno);
                ret = -ENOENT;
                goto RET_ONE;
            }
            ret = ioctl(dev_fd, MEMGETINFO, &mtd);
            if (ret < 0)
            {
                TRACE("ioctl(): This doesn't seem to be a valid MTD flash device!\n");
                close(dev_fd);
                goto RET_ONE;
            }
            ret = (mtd.erasesize);
            close(dev_fd);
    }
    else if(!strncmp(string, "rkflash", 7))
    {
        ret = 64*1024;
    }
    else
        ret = 4*1024;

RET_ONE:
    return ret;
}

static int format_sign(unsigned char *data, int len, unsigned char* sign_data, int sign_len);
static int upgrade_ota_init_state(ota_stream *obj, int *sizeunit)
{
	int ret;
	ota_sign *psign = NULL;
    unsigned char sign_hex[64] = {0};
    int blocksz;

#if 0
	ret = read(fw_fd, obj->buf, obj->len_owe);
	if (ret < 0) {
		printf("Failure in read %d: %s", fw_fd, strerror(errno));
        ret = -EFAULT;
	    goto RET;
	} else if (ret == 0) {
		printf("read all of the file");
		goto RET;
	}
#endif

	if (get_verify_sign_mode() == 1)
	{
        psign = (ota_sign *)(obj->buf + OTA_HEAD_SIZE);

        if(psign->len == 0) {
           ERROR("%s:psign->len ==0",__func__);
	   ret = -1;
           goto RET;
	}

        if (format_sign(psign->data, psign->len, sign_hex, 64) < 0) {
            ret = -1;
	    goto RET;
	}
        ret = tuya_verify_data((unsigned char *)obj->buf, OTA_HEAD_SIZE, sign_hex);
	    if (ret < 0)
	    {
            printf("tuya_verify_data fail!\n");
            ret = -1;
            goto RET;
	    }
	}

	memcpy(&obj->img.head, obj->buf, sizeof(ota_head));
	free(obj->buf);
    obj->buf=NULL;

    get_real_device_path(obj->img.head.zone[0].device[0]);
	blocksz = get_device_geometry(obj->img.head.zone[0].device[0]);
    if(blocksz<0)
    {
        printf("can not get geometry!\n");
        ret=-1;
        goto RET;
    }
    printf("get_device_geometry is %d\n", blocksz);
    *sizeunit = blocksz;
    obj->buf = calloc(1, *sizeunit);
	if (!obj->buf)
	{
        ret = -ENOMEM;
        goto RET;
	}

	obj->len_owe = obj->img.head.zone[0].image_size;
    obj->len_earn = 0;
    obj->status = GET_IMG;
	obj->img.total_len = get_zone_size_by_index(&obj->img,
			obj->img.head.zone_num);
	return 0;
RET:
    free(obj->buf);
    obj->buf=NULL;
    return ret;
}

static int device_upgrade_update_statue(ota_stream *obj)
{
	int idx;
	int ret = 0;
	unsigned char *sum;

    switch (obj->status) {
    case GET_IMG:
        idx = ++obj->img.zone_idx;
        if (obj->img.zone_idx < obj->img.head.zone_num) {
            if ((!strncmp(obj->img.head.zone[idx].name, "backup", 32)) && (obj->img.zone_idx < (obj->img.head.zone_num - 1)))
                idx = ++obj->img.zone_idx;
            else if((!strncmp(obj->img.head.zone[idx].name, "backup", 32)) && (obj->img.zone_idx == (obj->img.head.zone_num - 1)))
            {
                obj->img.zone_idx = obj->img.head.zone_num;
                obj->len_owe = obj->img.head.file[0].image_size;
                obj->status = GET_FILE;
            }
            obj->len_owe = obj->img.head.zone[idx].image_size;
        } else {
            obj->len_owe = obj->img.head.file[0].image_size;
            obj->status = GET_FILE;
        }
        obj->len_earn = 0;
        ret = 0;
        break;
    case GET_FILE:
        idx = obj->img.file_idx;
        sum = (unsigned char *)obj->img.head.file[idx].sha256;
        ret = tuya_check_file(obj->buf, obj->len_earn, sum);
        ret |= install_single_file(obj->buf, &obj->img.head.file[idx]);
        idx = ++obj->img.file_idx;
        obj->len_owe = obj->img.head.file[idx].image_size;
        obj->len_earn = 0;
        ret = 0;
        break;
    default:
        TRACE("why");
        ret = 0;
        break;
    }

    if (obj->img.zone_idx == obj->img.head.zone_num &&
            obj->img.file_idx == obj->img.head.file_num) {
        upgrade_done(&obj->img);
        obj->abort = 1;
        ret=1;
    }

    return ret;
}

static int device_upgrade_control_state(ota_stream *obj, char *data, int data_len)
{
	int ret = 0;
    int loopcnt=0;
    int tmpcnt=0;
    int consume_before=0;
    int consume_after=0;
    int owe=0;
    int headrconsume=0;


	if (obj == NULL || obj->abort == 1)
		return -1;

    if(!allocflag)
    {
	    streambuf.legacy_size = TUYA_STREAM_BUFF_SIZE;

        streambuf.legacy_cnt = 0;
        streambuf.legacy_offset= 0;
        streambuf.legacy_buff = calloc(1, streambuf.legacy_size);
        if (!streambuf.legacy_buff)
            return -ENOMEM;
        allocflag=1;
    }

    if(!initflag)
    {
        /* gather ota header */
        if((streambuf.legacy_cnt+data_len)<obj->len_owe)
        {
            memcpy(streambuf.legacy_buff+streambuf.legacy_cnt, data, data_len);
            streambuf.legacy_cnt += data_len;
            return 0;
        }
        else
        {
            headrconsume = obj->len_owe-streambuf.legacy_cnt;
            memcpy(streambuf.legacy_buff+streambuf.legacy_cnt, data, headrconsume);
            streambuf.legacy_cnt += headrconsume;
        }

        data += headrconsume;
        data_len -= headrconsume;

        memcpy(obj->buf, streambuf.legacy_buff, obj->len_owe);
        streambuf.legacy_cnt -= obj->len_owe;

        ret = upgrade_ota_init_state(obj, &sizeunit);
        if(ret || (sizeunit<=0))
        {
            ret = -1;
            return ret;
        }
        initflag=1;
    }

    /* consume all data */
    while(data_len)
    {
        if(obj->len_owe>= sizeunit)
            owe = sizeunit;
        else
            owe = obj->len_owe;

        /* 1. consume raw streambuf */
        if(streambuf.legacy_cnt>0)
        {
            if(streambuf.legacy_cnt+data_len < owe)
            {
                memcpy(streambuf.legacy_buff+streambuf.legacy_cnt, data, data_len);
                streambuf.legacy_cnt += data_len;
                return 0;
            }
            else
            {
                consume_before = owe-streambuf.legacy_cnt;
                memcpy(streambuf.legacy_buff+streambuf.legacy_cnt, data, consume_before);
                streambuf.legacy_cnt += consume_before;
                memcpy(obj->buf, streambuf.legacy_buff, owe);
                streambuf.legacy_cnt -= owe;
                data += consume_before;
                data_len -= consume_before;

                ret = consume_stream(obj, obj->buf, owe);
                if (ret < 0) {
                    obj->abort = 1;
                    ret = -1;
                    printf("%s %d ret=%d\n", __func__, __LINE__, ret);
                    return ret;
                }

                if(obj->len_owe == 0)         /* unpdate obj->owe */
                {
                    printf("%s %d data_len=%d\n", __func__, __LINE__, data_len);
                    ret = device_upgrade_update_statue(obj);
                    if(!ret)
                        continue;
                    else
                    {
                        ret=0;
                        break;
                    }
                }
            }
        }

        /* 2. consume feed data */
        consume_after = data_len;

        if(obj->len_owe>= sizeunit)
            owe = sizeunit;
        else
            owe = obj->len_owe;

        if(consume_after<owe)
        {
            /* save to streambuf */
            memcpy(streambuf.legacy_buff+streambuf.legacy_cnt, data, consume_after);
            streambuf.legacy_cnt += consume_after;
            return 0;
        }

        tmpcnt=0;
        while((obj->len_owe != 0) && ((consume_after >= sizeunit) || (consume_after >= obj->len_owe)))
        {
            /* send to obj */
            if(obj->len_owe>= sizeunit)
                loopcnt = sizeunit;
            else
            {
                printf("%s %d obj->len_owe=%d\n", __func__, __LINE__, obj->len_owe);
                loopcnt = obj->len_owe;
            }

            memcpy(obj->buf, data+tmpcnt, loopcnt);
            tmpcnt+=loopcnt;
            consume_after-=loopcnt;

            ret = consume_stream(obj, obj->buf, loopcnt);
            if (ret < 0) {
                obj->abort = 1;
                ret = -1;
                printf("%s %d ret=%d\n", __func__, __LINE__, ret);
                return ret;
            }
        }

        data += (data_len-consume_after);
        data_len = consume_after;

        if(obj->len_owe>0)
        {
            //printf("%s %d obj->owe=%d data_len=%d\n",__func__, __LINE__, obj->len_owe, data_len);
        }
        else if(obj->len_owe==0)         /* unpdate obj->owe */
        {
            ret = device_upgrade_update_statue(obj);
            if(ret)
            {
                ret=0;
                break;
            }
        }
    }

    if (ret)
    	TRACE("--------> ret %d", ret);

	return ret;
}

void device_upgrade_set_stmcnt(unsigned int length)
{
    if(length<(4*1024*1024))
        streamcnt = length;//max 4MB
}

int device_upgrade_feed(ota_stream *obj, char *data, int data_len)
{
	int ret;
	
	if (obj == NULL || data == NULL || data_len <= 0)
		return -1;
	
	if (obj->non_block)
		ret = swupgrade_async_write(obj, data, data_len);
	else
		ret = device_upgrade_control_state(obj, data, data_len);
	
	return ret;
}

static int format_sign(unsigned char *data, int len, unsigned char* sign_data, int sign_len)
{
	int i;
	unsigned char sign1=0, sign2=0;
	unsigned char sign[160] = {0};
	int s_index;

	if (data == NULL || len > 320 || sign_data == NULL || sign_len < 64) {
            fprintf(stderr, "invalid param\n");
	    return -1;
	}

	if (get_verify_sign_mode() == 0)
               return 0;
	
	for (i = 0; i < len; i = i + 2) {
		if ((*(data + i) >= '0') && (*(data + i) <= '9'))
			sign1 = *(data + i) - '0';
		if ((*(data + i) >= 'a') && (*(data + i) <= 'f'))
			sign1 = *(data + i) - 'a' + 10;
		if ((*(data + i + 1) >= '0') && (*(data + i + 1) <= '9'))
			sign2 = *(data + i + 1) - '0';
		if ((*(data + i + 1) >= 'a') && (*(data + i + 1) <= 'f'))
			sign2 = *(data + i + 1) - 'a' + 10;
		sign[i/2] = sign1 * 16 + sign2;
	}
	
	/* r len */
	if (sign[3] == 0x21) {
		memcpy(sign_data, &(sign[5]), 32);
		s_index = 38;
	} else {
		memcpy(sign_data, &(sign[4]), 32);
		s_index = 37;
	}
	
	/* s len */
	if (sign[s_index] == 0x21)
		memcpy(&(sign_data[32]), &(sign[s_index+2]), 32);
	else
		memcpy(&(sign_data[32]), &(sign[s_index+1]), 32);

	return 0;
}

int device_upgrade_feed_internal(ota_stream *obj, char *data, int data_len)
{
	int ret = 0;
	char *buf;
	int len;
	unsigned char *sum;
	int idx;
	ota_sign *psign = NULL;
    unsigned char sign_hex[64] = {0};

	buf = data;
	len = data_len;

	if (obj == NULL || obj->abort == 1)
		return -1;

	while (!obj->abort) {
		ret = consume_stream(obj, buf, len);
		if (ret < 0) {
			obj->abort = 1;
			return -1;
		}

		if (obj->len_owe > 0)
			return 0;
		buf += ret;
		len -= ret;
		ret = 0;
		switch (obj->status) {
        case GET_HEAD:
			if (get_verify_sign_mode() == 1) {
				psign = (ota_sign *)(obj->buf + OTA_HEAD_SIZE);

        			if(psign->len == 0) {
           				ERROR("%s:psign->len ==0",__func__);
					return -1;
				}

                               if (format_sign(psign->data, psign->len, sign_hex, 64) < 0) {
                                   return -1;
	                        }

				ret = tuya_verify_data((unsigned char *)obj->buf, OTA_HEAD_SIZE, sign_hex);
				if (ret < 0)
					return -1;
			}
			memcpy(&obj->img.head, obj->buf, sizeof(ota_head));
			free(obj->buf);
#ifndef ENABLE_WRITE_AHEAD
			ret = get_max_zone_size(&obj->img.head);
			obj->buf = calloc(1, ret);
			if (!obj->buf)
				return -ENOMEM;
#endif
			obj->len_owe = obj->img.head.zone[0].image_size;
            obj->len_earn = 0;
            obj->status = GET_IMG;
			obj->img.total_len = get_zone_size_by_index(&obj->img,
					obj->img.head.zone_num);
			ret = 0;
            break;
        case GET_IMG:
			idx = obj->img.zone_idx;
#ifndef ENABLE_WRITE_AHEAD
			ret = cmp_img_version(&obj->img.head.zone[idx]);
			if (ret == 0) {
				sum = (unsigned char *)obj->img.head.zone[idx].sha256;
				ret = tuya_verify_img(obj->buf, obj->len_earn, sum);
				if (ret < 0)
					break;
				obj->img.img_buf = obj->buf;
				ret |= install_single_image(&obj->img);
				if (ret < 0)
					break;
			}
#endif
			if (obj->img.zone_idx < obj->img.head.zone_num) {
				idx = ++obj->img.zone_idx;
				obj->len_owe = obj->img.head.zone[idx].image_size;
			} else {
				obj->len_owe = obj->img.head.file[0].image_size;
				obj->status = GET_FILE;
			}
			obj->len_earn = 0;
			ret = 0;
            break;
        case GET_FILE:
			idx = obj->img.file_idx;
			sum = (unsigned char *)obj->img.head.file[idx].sha256;
			ret = tuya_check_file(obj->buf, obj->len_earn, sum);
			ret |= install_single_file(obj->buf, &obj->img.head.file[idx]);
			idx = ++obj->img.file_idx;
			obj->len_owe = obj->img.head.file[idx].image_size;
			obj->len_earn = 0;
			ret = 0;
			break;
		default:
			TRACE("why");
			break;
		}

		if (obj->img.zone_idx == obj->img.head.zone_num &&
				obj->img.file_idx == obj->img.head.file_num) {
			upgrade_done(&obj->img);
			obj->abort = 1;
			break;
		}
#ifdef ENABLE_FLASH_ERASE_128KB_BLOCK
		return 0;
#endif
	}

	if (ret)
		TRACE("--------> ret %d", ret);

	return ret;
}

int device_upgrade_close(struct ota_stream *obj)
{
	if (!obj)
		return -1;

	printf("################## %s %d\n", __func__, __LINE__);
	obj->abort = 1;

	stop_progress_notifier(&obj->img);

	if (obj->non_block)
		swupgrade_async_close(obj);

	free(obj->buf);
	free(obj);

    if(streambuf.legacy_buff)
    {
        free(streambuf.legacy_buff);
        streambuf.legacy_buff=NULL;
    }

	return 0;
}

// local install staff

static int extract_and_verify_head(int fdin, ota_head *phead, off_t *offs)
{
	char *buf = NULL;
	int len;
	int ret;
	ota_sign *psign = NULL;
        unsigned char sign_hex[64] = {0};

	len = OTA_HEAD_SIZE + sizeof(ota_sign);
	buf = calloc(1, len);
	if (!buf)
		return -ENOMEM;

	ret = read(fdin, buf, len);
	if (ret <= 0 || ret != len) {
		ERROR("Failure in read %d: %s", fdin, strerror(errno));
		free(buf);
		return -EFAULT;
	}

	if(strncmp(buf, "TUYA_OTA", 8))
	{
		printf("invliad ota file!\n");
		free(buf);
		return -1;
	}

	if (get_verify_sign_mode() == 1) {
		psign = (ota_sign *)(buf + OTA_HEAD_SIZE);

		if(psign->len == 0) {
	            ERROR("%s:psign->len ==0",__func__);
		    free(buf);
		    return -1;
		}

                if (format_sign(psign->data, psign->len, sign_hex, 64) < 0) {
		     free(buf);
                     return -1;
	        }

		ret = tuya_verify_data((unsigned char *)buf, OTA_HEAD_SIZE, sign_hex);
		if (ret < 0) {
			free(buf);
			return -1;
		}
	}

	memcpy(phead, buf, sizeof(ota_head));
	*offs += len;

	free(buf);
	return 0;
}

static int extract_and_verify_img(int fdin, ota_image *pimg, off_t *offs)
{
	int ret;
	int len;
	int idx;
	int img_size;
	unsigned char *sum;

	if (lseek(fdin, *offs, SEEK_SET) < 0) {
		ERROR("seek file corrupted: %s", strerror(errno));
		return -1;
	}

	idx = pimg->zone_idx;
	img_size = pimg->head.zone[idx].image_size;
	len = read(fdin, pimg->img_buf, img_size);
	if (len <= 0 || len != img_size) {
		ERROR("Failure in read %d: %s", fdin, strerror(errno));
		return -EFAULT;
	}

	sum = (unsigned char *)pimg->head.zone[idx].sha256;
	ret = tuya_verify_img(pimg->img_buf, len, sum);
	if (ret < 0) {
		ERROR("verify image %s failed.\n", pimg->head.zone[idx].name);
		return -1;
	}

	*offs += len;

	return 0;
}

static int extract_and_verify_file(int fdin, ota_image *pimg, off_t *offs)
{
	int ret;
	int len;
	int idx;
	int img_size;
	unsigned char *sum;

	if (lseek(fdin, *offs, SEEK_SET) < 0) {
		ERROR("seek file corrupted: %s", strerror(errno));
		return -1;
	}

	idx = pimg->file_idx;
	img_size = pimg->head.file[idx].image_size;
	len = read(fdin, pimg->img_buf, img_size);
	if (len <= 0 || len != img_size) {
		ERROR("Failure in read %d: %s", fdin, strerror(errno));
		return -EFAULT;
	}

	sum = (unsigned char *)pimg->head.file[idx].sha256;
	ret = tuya_check_file(pimg->img_buf, len, sum);
	if (ret < 0) {
		ERROR("verify file %s failed.\n", pimg->head.file[idx].name);
		return -1;
	}

	*offs += len;

	return 0;
}

static int print_upgrade_status(int percent, void *user_data)
{
	INFO("-------------------------> %d", percent);

	return 0;
}

int install_from_file(char *fw_name)
{
	int fw_fd;
	off_t pos;
	ssize_t len;
	ota_head head;
	ota_image image;
	int ret;
	int i;

	fw_fd = open(fw_name, O_RDONLY);
	if (fw_fd < 0) {
		ERROR("FW %s not found: %d", fw_name, errno);
		return -ENOENT;
	}

	pos = 0;
	ret = extract_and_verify_head(fw_fd, &head, &pos);
	if (ret < 0) {
		close(fw_fd);
		return -1;
	}

	memset(&image, 0, sizeof(ota_image));
	len = get_max_zone_size(&head);
	image.zone_idx = 0;
	memcpy(&image.head, &head, sizeof(ota_head));
	image.img_buf = calloc(1, len);
	if (!image.img_buf) {
		ERROR("out of memory\n");
		close(fw_fd);
		return -ENOMEM;
	}

	image.progress_cb = print_upgrade_status;
	image.user_data = NULL;

	image.total_len = get_zone_size_by_index(&image, image.head.zone_num);

	start_progress_notifier(&image);

	for (i = 0; i < head.zone_num; i++) {
		image.zone_idx = i;
		update_loadin_progress(&image, head.zone[i].image_size);

		ret = cmp_img_version(&head.zone[i]);
		if (ret < 0) {
			pos += head.zone[i].image_size;
			update_writein_progress(&image, head.zone[i].image_size);
			continue;
		}

		//image.zone_idx = i;
		ret = extract_and_verify_img(fw_fd, &image, &pos);
		//update_loadin_progress(&image, head.zone[i].image_size);
		ret |= install_single_image(&image);
		if (ret < 0) {
			ERROR("extract|install image failed.\n");
			close(fw_fd);
			free(image.img_buf);
			return -1;
		}
	}

	/* install file */
	for (i = 0; i < head.file_num; i++) {
		image.file_idx = i;
		ret = extract_and_verify_file(fw_fd, &image, &pos);
		ret |= install_single_file(image.img_buf, &image.head.file[i]);
		if (ret < 0) {
			ERROR("extract|install file failed.\n");
			close(fw_fd);
			free(image.img_buf);
			return -1;
		}
	}

	upgrade_done(&image);

	stop_progress_notifier(&image);

	close(fw_fd);
	free(image.img_buf);
	return 0;
}

// url install staff

#if defined(ENABLE_URL_INSTALL) && (ENABLE_URL_INSTALL==1)
static int get_data_cb(char *buf, int len, void *user_data)
{
	int cpylen;
	ota_data *data = (ota_data *)user_data;

	if (!buf || len < 0 || !user_data)
		return -1;

	cpylen = len;

	if (data->total_len - data->cur_len < len)
		cpylen = data->total_len - data->cur_len;

	memcpy(data->buf + data->cur_len, buf, cpylen);

	data->cur_len += cpylen;

	update_loadin_progress((ota_image *)data->priv_data, cpylen);

	return 0;
}

static int get_and_verify_head(char *url, ota_head *phead, int *offs)
{
	ota_data data;
	int ret;
	ota_sign *psign = NULL;
        unsigned char sign_hex[64] = {0};

	memset(&data, 0, sizeof(data));
	data.cur_len = 0;
	data.total_len = OTA_HEAD_SIZE + sizeof(ota_sign);
	data.buf = calloc(1, data.total_len);
	if (!data.buf)
		return -ENOMEM;

	ret = tuya_http_get_file(url, get_data_cb, 0, data.total_len, (void*)&data);
	if (ret) {
		ERROR("Failure get head from url %s, err: %d", url, ret);
		free(data.buf);
		return -EFAULT;
	}

	if (get_verify_sign_mode() == 1) {
		psign = (ota_sign *)(data.buf + OTA_HEAD_SIZE);
                if(psign->len == 0) {
                    ERROR("%s:psign->len ==0",__func__);
		    free(data.buf);
		    return -1;
	        }

                if (format_sign(psign->data, psign->len, sign_hex, 64) < 0) {
		    free(data.buf);
		    return -1;
		}
		ret = tuya_verify_data((unsigned char *)data.buf, OTA_HEAD_SIZE, sign_hex);
		if (ret < 0) {
			free(data.buf);
			return -1;
		}
	}

	memcpy(phead, data.buf, sizeof(ota_head));
	*offs += data.total_len;

	free(data.buf);
	return 0;
}

static int get_and_verify_img(char *url, ota_image *pimg, int *offs)
{
	ota_data data;
	int ret;
	int idx;
	unsigned char *sum;

	idx = pimg->zone_idx;
	data.cur_len = 0;
	data.total_len = pimg->head.zone[idx].image_size;
	data.buf = pimg->img_buf;
	data.priv_data = (void*)pimg;

	ret = tuya_http_get_file(url, get_data_cb, *offs, data.total_len, (void*)&data);
	if (ret) {
		ERROR("Failure get head from url %s, err: %d", url, ret);
		return -EFAULT;
	}

	sum = (unsigned char *)pimg->head.zone[idx].sha256;
	ret = tuya_verify_img(pimg->img_buf, data.total_len, sum);
	if (ret < 0) {
		ERROR("verify image %s failed.\n", pimg->head.zone[idx].name);
		return -1;
	}

	*offs += data.total_len;

	return 0;
}

static int get_and_check_file(char *url, ota_image *pimg, int *offs)
{
	ota_data data;
	int ret;
	int idx;
	unsigned char *sum;

	idx = pimg->file_idx;
	data.cur_len = 0;
	data.total_len = pimg->head.file[idx].image_size;
	data.buf = pimg->img_buf;
	data.priv_data = (void*)pimg;

	ret = tuya_http_get_file(url, get_data_cb, *offs, data.total_len, (void*)&data);
	if (ret) {
		ERROR("Failure get head from url %s, err: %d", url, ret);
		return -EFAULT;
	}

	sum = (unsigned char *)pimg->head.file[idx].sha256;
	ret = tuya_check_file(pimg->img_buf, data.total_len, sum);
	if (ret < 0) {
		ERROR("verify file %s failed.\n", pimg->head.file[idx].name);
		return -1;
	}

	*offs += data.total_len;

	return 0;
}

static int get_resp_cb(char *buf, int size, void *user_data)
{
        DEBUG("%s\n", buf);
        return 0;
}

static int post_upgrade_status(int percent, void *user_data)
{
	int ret = 0;
	char final_url[256] = {0};
	char final_data[128] = {0};
	progress_post *post;

	post = (progress_post *)user_data;
	if (!post)
		return -1;

	ret = ty_make_full_url(post->url, post->api_name, post->api_ver,
			post->aes_key, post->dev_id, final_url);
	if (ret)
		return -1;

	ret = ty_make_encode_data(percent, final_data, post->aes_key);
	if (ret)
		return -1;

	ret = tuya_http_inf_post(final_url, get_resp_cb, (unsigned char *)final_data,
			strlen(final_data), NULL, NULL);

	return ret;
}

int install_from_url(char *url, progress_post *post)
{
	int i;
	int ret = 0;
	int idx = 0;
	int len = 0;
	int pos = 0;
	ota_head head;
	ota_image image;

	ret = get_and_verify_head(url, &head, &pos);
	if (ret < 0)
		return -1;

	memset(&image, 0, sizeof(ota_image));
	len = get_max_zone_size(&head);
	image.zone_idx = 0;
	memcpy(&image.head, &head, sizeof(ota_head));
	image.img_buf = calloc(1, len);
	if (!image.img_buf) {
		ERROR("out of memory\n");
		return -ENOMEM;
	}

	image.progress_cb = post_upgrade_status;
	image.user_data = (void*)post;

	image.total_len = get_zone_size_by_index(&image, image.head.zone_num);

	start_progress_notifier(&image);

	for (i = 0; i < head.zone_num; i++) {
		ret = cmp_img_version(&head.zone[i]);
		if (ret < 0) {
			pos += head.zone[i].image_size;
			continue;
		}

		image.zone_idx = i;
		ret = get_and_verify_img(url, &image, &pos);
		ret |= install_single_image(&image);
		if (ret < 0) {
			ERROR("extract|install image failed.\n");
			free(image.img_buf);
			return -1;
		}
	}

	/* install file */
	for (i = 0; i < head.file_num; i++) {
		image.file_idx = i;
		ret = get_and_check_file(url, &image, &pos);
		ret |= install_single_file(image.img_buf, &image.head.file[idx]);
		if (ret < 0) {
			ERROR("extract|install file failed.\n");
			free(image.img_buf);
			return -1;
		}
	}

	upgrade_done(&image);

	stop_progress_notifier(&image);

	free(image.img_buf);
	return 0;
}
#else
int install_from_url(char *url, progress_post *post)
{
	ERROR("ENABLE_URL_INSTALL is not enabled when build\n");
	return -1;
}
#endif

// debug functions

int tuya_upgrade_dump(char *fw_name)
{
	int i, j;
	int fw_fd;
	off_t pos;
	ota_head head;
	int ret;

	fw_fd = open(fw_name, O_RDONLY);
	if (fw_fd < 0) {
		ERROR("FW %s not found: %d", fw_name, errno);
		return -ENOENT;
	}

	pos = 0;
	ret = extract_and_verify_head(fw_fd, &head, &pos);
	if (ret < 0) {
		close(fw_fd);
		return -1;
	}

	printf("magic: %s\n", head.magic);
	printf("flash_type: %s\n", head.flash_type);
	printf("ota mode: %s\n", head.ota_mode);
	printf("zone_num: %d\n", head.zone_num);
	for (i = 0; i < head.zone_num; i++) {
		printf("\tname: %s\n", head.zone[i].name);
		printf("\tversion: %s\n", head.zone[i].version);
		printf("\toffset: 0x%x\n", head.zone[i].offset);
		printf("\timage_size: %d\n", head.zone[i].image_size);
		printf("\tdevice[0]: %s\n", head.zone[i].device[0]);
		printf("\tdevice[1]: %s\n", head.zone[i].device[1]);
		printf("\tdevice_offset: 0x%x\n", head.zone[i].device_offset);
		printf("\tfilesystem: %s\n", head.zone[i].filesystem);
		printf("\tcopy_in_boot: %d\n", head.zone[i].copy_in_boot);
		printf("\thash: ");
		for (j = 0; j < 32; j++)
			printf("%02hhx", head.zone[i].sha256[j]);
		printf("\n");
		printf("\t-----------------------------\n");
	}

	printf("file_num: %d\n", head.file_num);
	for (i = 0; i < head.file_num; i++) {
		printf("\tname: %s\n", head.file[i].name);
		printf("\tversion: %s\n", head.file[i].version);
		printf("\timage_size: %d\n", head.file[i].image_size);
		printf("\tstorage_path: %s\n", head.file[i].storage_path);
		printf("\tinstaller: %s\n", head.file[i].installer);
		printf("\thash: ");
		for (j = 0; j < 32; j++)
			printf("%02hhx", head.file[i].sha256[j]);
		printf("\n");
		printf("\t-----------------------------\n");
	}

	close(fw_fd);

	return 0;
}

int tuya_upgrade_extract(char *fw_name, char *dst_path)
{
	int fw_fd;
	off_t pos;
	ssize_t len;
	ota_head head;
	ota_image image;
	int ret;
	int i;
	char temp_path[128]= {0};
	char full_path[128] = {0};

	if (dst_path && strlen(dst_path))
		strncpy(temp_path, dst_path, 128);
	else
		strncpy(temp_path, "/tmp/", 128);

	fw_fd = open(fw_name, O_RDONLY);
	if (fw_fd < 0) {
		ERROR("FW %s not found: %d", fw_name, errno);
		return -ENOENT;
	}

	pos = 0;
	ret = extract_and_verify_head(fw_fd, &head, &pos);
	if (ret < 0) {
		close(fw_fd);
		return -1;
	}

	memset(&image, 0, sizeof(ota_image));
	len = get_max_zone_size(&head);
	image.zone_idx = 0;
	memcpy(&image.head, &head, sizeof(ota_head));
	image.img_buf = calloc(1, len);
	if (!image.img_buf) {
		ERROR("out of memory\n");
		close(fw_fd);
		return -ENOMEM;
	}

	for (i = 0; i < head.zone_num; i++) {
		if (!image.head.zone[i].image_size)
			continue;
		image.zone_idx = i;
		ret = extract_and_verify_img(fw_fd, &image, &pos);
		snprintf(full_path, 128, "%s/%s-0x%x.bin", temp_path,
				image.head.zone[i].name,
				image.head.zone[i].device_offset);
		if (ret < 0) {
			ERROR("extract image failed.\n");
			free(image.img_buf);
			return -1;
		}
		extract_image_to_file(image.img_buf,
				image.head.zone[i].image_size, full_path);
	}

	for (i = 0; i < head.file_num; i++) {
		if (!image.head.file[i].image_size)
			continue;
		image.file_idx = i;
		ret = extract_and_verify_file(fw_fd, &image, &pos);
		snprintf(full_path, 128, "%s/%s", temp_path,
				image.head.file[i].name);
		if (ret < 0) {
			ERROR("extract file failed.\n");
			free(image.img_buf);
			return -1;
		}
		extract_image_to_file(image.img_buf,
				image.head.file[i].image_size, full_path);
	}

	free(image.img_buf);

	TRACE(" Done.");

	return 0;
}

int tuya_upgrade_version(char ver[32])
{
	if (!ver)
		return -1;

	strncpy(ver, UNPACK_VER, 32);

	return 0;
}

#define VERSION_COMPARE_KEY "temp.version.compare"

int set_version_compare_mode(int mode)
{
	int ret;

	if(mode) {
	    ret = prop_set_temp(VERSION_COMPARE_KEY,"on");
	}else {
	    ret = prop_set_temp(VERSION_COMPARE_KEY,"off");
	}

	return ret;
}

int get_version_compare_mode(void)
{
	int status = 0; 
	char value[8] = {0};

	prop_get(VERSION_COMPARE_KEY, value);

	if (strstr(value, "on"))
	    status = 1; // version compare 

	return status; 
}

int set_verify_sign_mode(int mode)
{
	if (mode)
		return prop_set_temp("temp.verify.sign", "on");
	else
		return prop_set_temp("temp.verify.sign", "off");
}

int get_verify_sign_mode()
{
	char value[8] = {0};

	prop_get("temp.verify.sign", value);
	if (strstr(value, "on"))
		return 1; // verify signature
	else
		return 0; // not verify signature
}
