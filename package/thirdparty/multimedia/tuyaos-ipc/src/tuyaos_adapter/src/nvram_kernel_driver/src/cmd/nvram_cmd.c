#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include "bcmnvram.h"

static void usage(char *programname)
{
        printf("%s (compiled %s)\n", programname, __DATE__);
        printf("Usage: %s get key\n", programname);
        printf("Usage: %s set key value [OPTION]\n", programname);
        printf(
                        " -t, --temp       : temp kv not write to flash when commit\n"
                        " -p, --persist    : write to flash immediately when set\n"
                        " -l, --lock       : kv can not be reset/clear when set\n"
                        " -h, --help       : print this help and exit\n"
               );
        printf("Usage: %s commit\n", programname);
}

#if 0
static struct option long_options[] = {
        {"temp", no_argument, NULL, 't'},
        {"persist", no_argument, NULL, 'p'},
        {"lock", no_argument, NULL, 'l'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
};
#endif

static void nvram_dump(char *header)
{
        char *name, *value, *end, *eq;

        name = header;
        end = (char *) header + NVRAM_SPACE - 2;
        end[0] = end[1] = '\0';
        for (; *name; name = value + strlen(value) + 1) {
                if (!(eq = strchr(name, '=')))
                        break;
                *eq = '\0';
                value = eq + 1;
		printf("[%s]: [%s]\n", name, value);
                *eq = '=';
        }
}

#if 0
int nvram_parse_attr(int argc, char **argv)
{
        int opt_t = 0;
        int opt_p = 0;
        int opt_l = 0;
        int c;
	int attr = 0;

        while ((c = getopt_long(argc, argv, "tplh",
                                long_options, NULL)) != EOF) {
                switch (c) {
                        case 't':
                                opt_t = 1;
                                break;
                        case 'p':
                                opt_p = 1;
                                break;
                        case 'l':
                                opt_l = 1;
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

	if (opt_t && (opt_p || opt_l))
		return -1;

	attr = (opt_t?ATTR_TEMP:0) | (opt_p?ATTR_PERSIST:0) | (opt_l?ATTR_LOCK:0);

	//printf("t: %d p: %d l: %d attr: 0x%x\n", opt_t, opt_p, opt_l, attr);

	return attr;
}
#else
int nvram_parse_attr(int argc, char **argv)
{
	int opt_t = 0;
	int opt_p = 0;
	int opt_l = 0;
	int i;
	int attr = 0;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--temp") || !strcmp(argv[i], "-t"))
			opt_t = 1;
		else if (!strcmp(argv[i], "--persist") || !strcmp(argv[i], "-p"))
			opt_p = 1;
		else if (!strcmp(argv[i], "--lock") || !strcmp(argv[i], "-l"))
			opt_l = 1;
		else
			return -1;
	}

	if (opt_t && (opt_p || opt_l))
		return -1;

	attr = (opt_t?ATTR_TEMP:0) | (opt_p?ATTR_PERSIST:0) | (opt_l?ATTR_LOCK:0);

//	printf("t: %d p: %d l: %d attr: 0x%x\n", opt_t, opt_p, opt_l, attr);

	return attr;
}
#endif

int main(int argc, char **argv)
{
        int ret = -1;
	char value[NVRAM_MAX_VALUE_LEN] = {0};
	char name[NVRAM_MAX_PARAM_LEN] = {0};
	int attr = 0;

	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	if (!strcmp(argv[1], "get") && argc == 3) {

		ret = nvram_get(argv[2], value);
		if (!ret)
			printf("%s\n", value);

		return ret;
	}

	if (!strcmp(argv[1], "set") && argc >= 4) {

		if (strlen(argv[2]) > NVRAM_MAX_PARAM_LEN ||
                                strlen(argv[3]) > NVRAM_MAX_VALUE_LEN)
                        return -1;

		memcpy(name, argv[2], strlen(argv[2]));
		memcpy(value, argv[3], strlen(argv[3]));

		if (argc > 4) {
#if 0
			attr = nvram_parse_attr(argc, &argv[0]);
#else
			attr = nvram_parse_attr(argc-4, &argv[4]);
#endif
			if (attr < 0) {
                                usage(argv[0]);
				return -1;
			}
		}

		ret = nvram_set_with_attr(name, value, attr);

		return ret;
	}

	if (!strcmp(argv[1], "unset") && argc == 3) {

		ret = nvram_unset(argv[2]);

		return ret;
	}

	if (!strcmp(argv[1], "commit") && argc == 2) {

		ret = nvram_commit();

		return ret;
	}

	if (!strcmp(argv[1], "show") && argc == 2) {

		char *buf = calloc(NVRAM_FLASH_SIZE, 1);
		if (!buf)
			return -1;
		nvram_getall(buf, NVRAM_FLASH_SIZE, 1);
		nvram_dump(buf);
		free(buf);

		return 0;
	}

	if (!strcmp(argv[1], "clear") && argc == 2) {

		ret = nvram_clear();

		return ret;
	}

	if (!strcmp(argv[1], "erase") && argc == 2) {

		ret = nvram_erase();

		return ret;
	}

	usage(argv[0]);

	return -1;
}
