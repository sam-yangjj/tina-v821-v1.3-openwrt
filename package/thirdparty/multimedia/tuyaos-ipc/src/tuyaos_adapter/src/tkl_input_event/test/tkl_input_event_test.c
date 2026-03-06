/*
 * Copyright (C) 2021 Tuya Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "tkl_input_event.h"

static const char *optstr = "t:";

static void __show_usage(const char *prog)
{
	printf("Usage: %s [OPTION]\n", prog);
	printf("    -t <timeout> timeout=\n");
	printf("        -1 : block indefinitely(default)\n");
	printf("        0  : non-block\n");
	printf("        >0 : timeout in milliseconds\n");
	printf("e.g. Non blocking detection event\n");
	printf("    %s -t 0\n", prog);
}

int main(int argc, char **argv)
{
	int ch;
	int ret = -1;
	int k;
	int count = 0;
	int timeout = -1;
	TUYA_INPUT_EVENT_T events[MAX_DEVICES];

	printf("%s %s %s\n", argv[0], __DATE__, __TIME__);

	while ((ch = getopt(argc, argv, optstr)) != -1) {
		switch (ch) {
			case 't':
				timeout = atoi(optarg);
				printf("timeout:%d\n", timeout);
				break;
			default:
				__show_usage(argv[0]);
            	return -1;
		}
	}

	ret = tkl_input_event_init();
	if (ret < 0) {
		printf("hal_input_event_init failed, ret = %d\n", ret);
		return -1;
	}

	for (k=0; k < 100; k++) {
		ret =  tkl_input_event_recv(events, &count, timeout);
		if (ret == -145) {
			printf("no input event detected\n");
			sleep(1);
		} else if (ret == 0) {
			int i = 0;
			for (i = 0; i < count; i++)
				printf("type:%d code:%d value:%d\n", events[i].type, events[i].code, events[i].value);
		} else {
			printf("hal_input_event_recv failed, ret = %d\n", ret);
			tkl_input_event_deinit();
			return -1;
		}
	}
	tkl_input_event_deinit();

	return 0;
}
