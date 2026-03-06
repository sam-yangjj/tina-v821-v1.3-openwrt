/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <libmtd.h>
#include <mtd/mtd-user.h>
#include "notifier.h"
#include "handler.h"

#define EMPTY_BYTE	0xFF
#define WRITE_SIZE	(64*1024)
#define SLEEP_INTERVAL	(5*1000)

void nor_flash_handler(void);

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

static int nor_flash_erase(char *dev, int offs, int len)
{
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
	unsigned int eb=0, eb_start=0, eb_cnt=0;
	int noskipbad = 0;
	int fd;
	int ret=0;
	int i;
	unsigned char *buf = NULL;

	mtd_desc = libmtd_open();
	if (mtd_desc == NULL) {
		ERROR("can't initialize libmtd");
		return -1;
	}

	if ((fd = open(dev, O_RDWR)) < 0) {
		ERROR("%s: %s: %s", __func__, dev, strerror(errno));
		libmtd_close(mtd_desc);
		return -ENODEV;
	}

	if (mtd_get_dev_info(mtd_desc, dev, &mtd) < 0) {
		ERROR("mtd_get_dev_info failed");
		ret = -1;
		goto erase_out;
	}

	buf = (unsigned char *)calloc(1, mtd.eb_size);
	if (!buf) {
		ERROR("No memory for buffer of %d bytes", mtd.eb_size);
		ret = -ENOMEM;
		goto erase_out;
	}

	eb_start = offs / mtd.eb_size;
	//eb_cnt = (mtd.size / mtd.eb_size) - eb_start;
	eb_cnt = (len + mtd.eb_size -1) / mtd.eb_size;

	for (eb = eb_start; eb < eb_start + eb_cnt; eb++) {
		/* Always skip bad sectors */
		if (!noskipbad) {
			ret = mtd_is_bad(&mtd, fd, eb);
			if (ret > 0) {
				TRACE("skip bad block");
				continue;
			} else if (ret < 0) {
				if (errno == EOPNOTSUPP) {
					noskipbad = 1;
				} else {
					ERROR("%s: MTD get bad block failed", dev);
					ret = -EFAULT;
					goto erase_out;
				}
			}
		}
		/* Unlock memory if required */
		if (mtd_is_locked(&mtd, fd, eb) > 0) {
			if (mtd_unlock(&mtd, fd, eb) != 0) {
				if (errno != EOPNOTSUPP) {
					TRACE("%s: MTD unlock failure", dev);
					continue;
				}
			}
		}
		/*
		 * In case of NOR flash, check if the flash
		 * is already empty. This can save
		 * an amount of time because erasing
		 * a NOR flash is very time expensive.
		 * NAND flash is always erased.
		 */
		if (mtd_read(&mtd, fd, eb, 0, buf, mtd.eb_size) != 0) {
			ERROR("%s: MTD Read failure", dev);
			ret  = -EIO;
			goto erase_out;
		}
		/* check if already empty */
		for (i = 0; i < mtd.eb_size; i++) {
			if (buf[i] != EMPTY_BYTE)
				break;
		}
		/* skip erase if empty */
		if (i == mtd.eb_size)
			continue;

		if (mtd_erase(mtd_desc, &mtd, fd, eb) != 0) {
			ERROR("%s: MTD Erase failure", dev);
			continue;
		}
#if defined(ENABLE_FLASH_ERASE_WRITE_SLEEP) && (ENABLE_FLASH_ERASE_WRITE_SLEEP==1)
		usleep(SLEEP_INTERVAL);
#endif
	}

erase_out:

	if (buf)
		free(buf);
	close(fd);
	libmtd_close(mtd_desc);

	return ret;
}

int install_nor_flash_image_wh(char *dev, ota_image *pimg, int offs, char *data, int length)
{
	int ret=-1;
	int fdout;
	char *buf;
	int len;
	int left = 0;

	if (dev == NULL || data == NULL)
		return -1;

        //TRACE("installing %s ...", dev);

	ret = nor_flash_erase(dev, offs, length);
	if (ret < 0)
		return -1;

        if ((fdout = open(dev, O_RDWR)) < 0) {
                ERROR("%s: %s: %s", __func__, dev, strerror(errno));
                return -1;
        }

	if (offs > 0) {
		//TRACE("offset has been defined: %d bytes", offs);
		if (lseek(fdout, offs, SEEK_SET) < 0) {
			ERROR("offset argument: seek failed");
			close(fdout);
			return -EFAULT;
		}
	}

	buf = data;
	len = WRITE_SIZE;
	left = length;

	while (left) {
		if (left < WRITE_SIZE)
			len = left;
		ret = copy_write(&fdout, buf, len);
		if (ret < 0) {
			ret = -ENOSPC;
			break;
		}
		buf = buf + len;
		left = left - len;
		update_writein_progress(pimg, length - left);
#if defined(ENABLE_FLASH_ERASE_WRITE_SLEEP) && (ENABLE_FLASH_ERASE_WRITE_SLEEP==1)
		usleep(SLEEP_INTERVAL);
#endif
	}

	close(fdout);
	return ret;
}
#if 0
__attribute__((constructor))
void nor_flash_handler(void)
{
	register_handler("nor", install_nor_flash_image);
}
#endif
