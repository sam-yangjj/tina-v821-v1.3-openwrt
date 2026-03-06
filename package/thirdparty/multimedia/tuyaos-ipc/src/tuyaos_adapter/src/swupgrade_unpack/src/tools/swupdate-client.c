#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include "network_ipc.h"

#define BUFF_SIZE	16384
char buf[BUFF_SIZE];
int fd = STDIN_FILENO;

static int send_file(const char* filename)
{
	int rc;

	if (filename && (fd = open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "Unable to open %s\n", filename);
		return EXIT_FAILURE;
	}

	rc = swupgrade_ipc_open();
	if (rc < 0) {
		fprintf(stderr, "swupdate_async_start returns %d\n", rc);
		close(fd);
		return EXIT_FAILURE;
	}

	while (1) {
		rc = read(fd, buf, sizeof(buf));
		if (rc < 0) {
			fprintf(stderr, "read file failed %d\n", rc);
			close(fd);
			return EXIT_FAILURE;
		}

		if (rc == 0) {
			fprintf(stderr, "read all file %d\n", rc);
			break;
		}
		int ret = swupgrade_ipc_write(buf, rc);
	}

	swupgrade_ipc_close(NULL, 0, 1);

	if (filename)
		close(fd);

	return 0;
}

int main(int argc, char *argv[])
{
	send_file(argv[1]);
	while (1) {
		sleep(20);
		printf("sleep swupgade client\n");
	}
}

