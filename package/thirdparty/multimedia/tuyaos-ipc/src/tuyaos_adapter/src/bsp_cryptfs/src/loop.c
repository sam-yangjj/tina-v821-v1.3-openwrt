#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/loop.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif

char *loopdev_create(const char *target_dev)
{
	int fd_ctl, fd_tgt, fd_lo;
	int idx;
	char *loopdev;

	fd_ctl = open("/dev/loop-control", O_RDWR | O_CLOEXEC);
	if (fd_ctl < 0) {
		printf("Open /dev/loop-control failed\n");
		return NULL;
	}

	idx = ioctl(fd_ctl, LOOP_CTL_GET_FREE);
	if (idx < 0) {
		printf("Can't get free loop device\n");
		loopdev = NULL;
		goto err_free;
	}

	asprintf(&loopdev, "/dev/loop%d", idx);

	fd_tgt = open(target_dev, O_RDWR | O_CLOEXEC);
	if (fd_tgt < 0) {
		printf("Open device %s failed\n", target_dev);
		free(loopdev);
		loopdev = NULL;
		goto err_free;
	}

	fd_lo = open(loopdev, O_RDWR | O_CLOEXEC);
	if (fd_lo < 0) {
		printf("Open loop device %s failed\n", loopdev);
		free(loopdev);
		loopdev = NULL;
		goto err_loop;
	}

	if (ioctl(fd_lo, LOOP_SET_FD, fd_tgt) < 0) {
		printf("Setup loop device failed\n");
		free(loopdev);
		loopdev = NULL;
	}

	close(fd_lo);
err_loop:
	close(fd_tgt);
err_free:
	close(fd_ctl);
	return loopdev;
}

void loopdev_destroy(char *loopdev)
{
	int fd;

	fd = open(loopdev, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		printf("Failed to open loop (%s)", loopdev);
		return;
	}

	if (ioctl(fd, LOOP_CLR_FD, 0) < 0) {
		printf("Failed to destroy loop (%s)", loopdev);
	}

	close(fd);
	free(loopdev);
}
