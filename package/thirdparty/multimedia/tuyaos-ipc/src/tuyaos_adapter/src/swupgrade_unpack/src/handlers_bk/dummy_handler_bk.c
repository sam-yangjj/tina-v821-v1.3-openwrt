/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "notifier.h"
#include "handler.h"

#define WRITE_SIZE	(6*1024)

void dummy_handler(void);

static int copy_write(void *out, const void *buf, unsigned int len)
{
	int ret;
	int fd = (out != NULL) ? *(int *)out : -1;

	while (len) {
		errno = 0;
		ret = write(fd, buf, len);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			ERROR("cannot write %d bytes: %s", len, strerror(errno));
			return -1;
		}

		if (ret == 0) {
			ERROR("cannot write %d bytes: %s", len, strerror(errno));
			return -1;
		}

		len -= ret;
		buf += ret;
	}

        return 0;
}

int install_nothing_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer)
{
	int ret=-1;
	int fdout;
	char *buf;
	int len;
	int left = 0;
    char *dev;

    dev = pimg->head.zone[idx].device[0];
    
	if (dev == NULL || pimg == NULL)
		return -1;

	fdout = open("/dev/null", O_WRONLY);
	if (fdout < 0) {
		TRACE("Device %s cannot be opened: %s", "/dev/null", strerror(errno));
		return -1;
	}

	buf = pimg->img_buf;
	len = WRITE_SIZE;
	idx = pimg->zone_idx;
	left = pimg->head.zone[idx].image_size;

	while (left) {
		if (left < WRITE_SIZE)
			len = left;
		ret = copy_write(&fdout, buf, len);
		if (ret < 0)
			break;
		buf = buf + len;
		left = left - len;
		update_writein_progress(pimg, pimg->head.zone[idx].image_size - left);
	}

	close(fdout);
	return ret;
}
#if 0
__attribute__((constructor))
void dummy_handler(void)
{
	register_handler("dummy", install_nothing);
}
#endif
