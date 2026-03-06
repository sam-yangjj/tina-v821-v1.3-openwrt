/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
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

void amlogic_nand_flash_handler(void);

static int nand_erase_rk(char *dev, int offset)
{
	int fd;
	int ret = 0;
	struct stat st;
	mtd_info_t meminfo;
	erase_info_t erase;

	TRACE("begin erase device %s\n", dev);

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		ERROR("open %s failed!\n", dev);
		return -1;
	}

	//check is a char device
	ret = fstat(fd, &st);
	if (ret < 0) {
		ERROR("fstat %s failed!\n", dev);
		close(fd);
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		ERROR("%s: not a char device", dev);
		close(fd);
		return -1;
	}

	//get meminfo
	ret = ioctl(fd, MEMGETINFO, &meminfo);
	if (ret < 0) {
		ERROR("get MEMGETINFO failed!\n");
		close(fd);
		return -1;
	}

	TRACE("part size: %d\n", meminfo.size / 1024 / 1024);

	erase.length = meminfo.erasesize;

	for (erase.start = offset; erase.start < offset + meminfo.size;
			erase.start += meminfo.erasesize) {
		loff_t bpos = erase.start;

		//check bad block
		ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
		if (ret > 0) {
			TRACE("mtd: not erasing bad block at 0x%08llx\n", bpos);
			continue;  // Don't try to erase known factory-bad blocks.
		}

		if (ret < 0) {
			ERROR("MEMGETBADBLOCK error");
			close(fd);
			return -1;
		}

		//erase
		if (ioctl(fd, MEMERASE, &erase) < 0) {
			ERROR("mtd: erase failure at 0x%08llx\n", bpos);
			close(fd);
			return -1;
		}
	}

	TRACE("erase over\n");

	close(fd);

	return 0;
}

static unsigned int next_good_eraseblock(int fd, struct mtd_info_user *meminfo,
		unsigned int block_offset)
{
	while (1) {

		loff_t offs;

		if (block_offset >= meminfo->size) {
			TRACE("not enough space in MTD device");
			return block_offset; /* let the caller exit */
		}

		offs = block_offset;
		if (ioctl(fd, MEMGETBADBLOCK, &offs) == 0)
			return block_offset;

		/* ioctl returned 1 => "bad block" */

		TRACE("Skipping bad block at 0x%08x\n", block_offset);

		block_offset += meminfo->erasesize;
	}

	return 0;
}

static int nand_write_rk(char *dev, int offs, ota_image *pimg)
{
	mtd_info_t meminfo;
	unsigned int blockstart;
	unsigned int limit = 0;
	int size = 0;
	int ret = 0;
	int offset = offs;
	char *file_buf;
	unsigned long len;
	int idx;

	idx = pimg->zone_idx;
	file_buf = pimg->img_buf;
	len = pimg->head.zone[idx].image_size;

	int fd = open(dev, O_WRONLY);
	if (fd < 0) {
		ERROR("open %s failed!\n", dev);
		return -1;
	}

	ret = ioctl(fd, MEMGETINFO, &meminfo);
	if (ret < 0) {
		ERROR("get MEMGETINFO failed!\n");
		close(fd);
		return -1;
	}

	limit = meminfo.size;

	/* check offset page aligned */
	if (offset & (meminfo.writesize - 1)) {
		ERROR("start address is not page aligned");
		close(fd);
		return -1;
	}

	/* malloc buffer for read */
	char *buf = (char *)malloc(meminfo.writesize);
	if (buf == NULL) {
		ERROR("malloc %d size buffer failed!\n", meminfo.writesize);
		close(fd);
		return -1;
	}

	/* if offset in a bad block, get next good block */
	blockstart = offset & ~(meminfo.erasesize - 1);
	if (offset != blockstart) {
		unsigned int tmp;
		tmp = next_good_eraseblock(fd, &meminfo, blockstart);
		if (tmp != blockstart) {
			offset = tmp;
		}
	}

	while (offset < limit) {
		blockstart = offset & ~(meminfo.erasesize - 1);
		if (blockstart == offset) {
			offset = next_good_eraseblock(fd, &meminfo, blockstart);
			TRACE("Writing at 0x%08x\n", offset);
			if (offset >= limit) {
				ERROR("offset(%d) over limit(%d)\n", offset, limit);
				close(fd);
				free(buf);
				return -1;
			}
		}

		lseek(fd, offset, SEEK_SET);

		if (len < meminfo.writesize) {
			/* zero pad to end of write block */
			memcpy(buf, file_buf, len);
			memset(buf + len, 0, meminfo.writesize - len);
		} else {
			memcpy(buf, file_buf, meminfo.writesize);
		}

		size = write(fd, buf, meminfo.writesize);
		if (size != meminfo.writesize) {
			ERROR("write err, need:%d, real:%d\n",
					meminfo.writesize, size);
			close(fd);
			free(buf);
			return -1;
		}

		if (len < meminfo.writesize) {
			file_buf += len;
			len = 0;
		} else {
			len -= meminfo.writesize;
			file_buf += meminfo.writesize;
		}

		offset += meminfo.writesize;

		update_writein_progress(pimg, pimg->head.zone[idx].image_size - len);

		if (len == 0) {
			TRACE("write ok!\n");
			break;
		}
	}

	free(buf);
	close(fd);

	return 0;
}

int install_amlogic_nand_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer)
{
	int ret;
    char *dev;

    dev = pimg->head.zone[idx].device[0];

	if (dev == NULL || pimg == NULL)
		return -1;

	ret = nand_erase_rk(dev, offset);
	if (ret < 0) {
		ERROR("erase %s failed.\n", dev);
		return -1;
	}

	ret = nand_write_rk(dev, offset, pimg);
	if (ret < 0) {
		ERROR("write %s failed.\n", dev);
		return -1;
	}

	return 0;
}
#if 0
__attribute__((constructor))
void amlogic_nand_flash_handler(void)
{
	register_handler("amlogic-nand", install_amlogic_nand_flash_image);
}
#endif
