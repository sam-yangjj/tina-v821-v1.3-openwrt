/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <getopt.h>

#include "progress_ipc.h"

#define PSPLASH_MSG_SIZE	64

static struct option long_options[] = {
	{"wait", no_argument, NULL, 'w'},
	{NULL, 0, NULL, 0}
};

static void psplash_write_fifo(char *pipe, char *buf)
{
	fprintf(stderr, "%s\n", buf);
}

static void psplash_progress(char *pipe, struct progress_msg *pmsg)
{
	char *buf;

	buf = malloc(PSPLASH_MSG_SIZE);

	if (!buf)
		return;

	switch (pmsg->status) {
	case SUCCESS:
	case FAILURE:
		snprintf(buf, PSPLASH_MSG_SIZE - 1, "MSG %s",
			 pmsg->status == SUCCESS ? "SUCCESS" : "FAILURE");
		psplash_write_fifo(pipe, buf);

		sleep(5);

		snprintf(buf, PSPLASH_MSG_SIZE - 1, "QUIT");
		psplash_write_fifo(pipe, buf);
		free(buf);
		return;
		break;
	default:
		break;
	}

	snprintf(buf, PSPLASH_MSG_SIZE - 1, "PROGRESS %d", pmsg->cur_percent);
	psplash_write_fifo(pipe, buf);

	free(buf);
}

int main(int argc, char **argv)
{
	int connfd = -1;
	struct progress_msg msg;
	int opt_w = 0;
	int c;

	while ((c = getopt_long(argc, argv, "w:", long_options, NULL)) != EOF) {
		switch (c) {
		case 'w':
			opt_w = 1;
			break;
		default:
			exit(1);
			break;
		}
	}
		
	while (1) {
		if (connfd < 0)
			connfd = progress_ipc_connect(opt_w);

		if (progress_ipc_receive(&connfd, &msg) == -1)
			continue;

		psplash_progress(NULL, &msg);

		switch (msg.status) {
		case SUCCESS:
		case FAILURE:
			fprintf(stdout, "\n\n");
			fprintf(stdout, "%s !\n", msg.status == SUCCESS
					? "SUCCESS" : "FAILURE");
			break;
		default:
			break;
		}
	}
}
