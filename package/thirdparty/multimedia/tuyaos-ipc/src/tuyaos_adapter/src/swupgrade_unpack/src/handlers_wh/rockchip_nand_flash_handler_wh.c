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

#define LBA_TRANSFER_SIZE	(1024*1024)

void rockchip_nand_flash_handler(void);

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

	/* Fix: oom on rk3308 */
	sync();
	return 0;
}

int install_rockchip_nand_flash_image_wh(char *dev, ota_image *pimg, int offs, char *data, int length)
{
	int ret=-1;
	int fdout;
	char *buf;
	int len;
	int left = 0;

	if (dev == NULL || pimg == NULL)
		return -1;

	fdout = open(dev, O_WRONLY);
	if (fdout < 0) {
		TRACE("Device %s cannot be opened: %s", dev, strerror(errno));
		return -1;
	}

	lseek(fdout, offs, SEEK_SET);

	buf = data;
	len = LBA_TRANSFER_SIZE;
	left = length;

	while (left) {
		if (left < LBA_TRANSFER_SIZE)
			len = left;
		//TRACE("-------------> left: %d, offs: %d", left, offs);
		ret = copy_write(&fdout, buf, len);
		if (ret < 0)
			break;
		buf = buf + len;
		left = left - len;
#ifndef ENABLE_WRITE_AHEAD
		update_writein_progress(pimg, length - left);
#endif
	}

	close(fdout);
	return ret;
}
#if 0
__attribute__((constructor))
void rockchip_nand_flash_handler(void)
{
	register_handler("rockchip-nand", install_rockchip_nand_flash_image);
}
#endif
