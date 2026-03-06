#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "cryptfs.h"
#include "bcmnvram.h"

#define MASTER_KEY_NAME "persist.cryptfs.key"

void uasge(char *prog)
{
	printf("Usage:\n");
	printf("\t%s -c [mapper name] [blkdev]\n", prog);
	printf("\t%s -d [mapper name]\n", prog);
}

int main(int argc, char **argv)
{
	int opt;
	int enable_flags = 0;
	char *name=NULL, *blkdev;
	char key[65];

	while ((opt = getopt(argc, argv, "c:d:")) != -1) {
		switch (opt) {
		case 'c':
			enable_flags = 1;
			name = optarg;
			break;
		case 'd':
			enable_flags = 0;
			name = optarg;
			break;
		default:
			uasge(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (enable_flags) {
		if (optind >= argc) {
			printf("Block device is needed!\n");
			uasge(argv[0]);
			exit(EXIT_FAILURE);
		}

		blkdev = argv[optind];

		if (nvram_get(MASTER_KEY_NAME, key) < 0) {
			printf("Get crypto key failed\n");
			return -1;
		}
		key[64] = '\0';

		if (cryptfs_enable(blkdev, name, key) < 0) {
			printf("Enable cryptfs %s failed\n", name);
			return -1;
		}
	} else {
		if (cryptfs_disable(name) < 0)
			return -1;
	}

	return 0;
}
