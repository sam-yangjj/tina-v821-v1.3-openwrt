/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdbool.h>
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

#define WRITE_SIZE	(6*1024)

void nand_flash_handler(void);

/* Check whether buffer is filled with character 'pattern' */
static inline int buffer_check_pattern(unsigned char *buffer, size_t size,
                                       unsigned char pattern)
{
        /* Invalid input */
        if (!buffer || (size == 0))
                return 0;

        /* No match on first byte */
        if (*buffer != pattern)
                return 0;

        /* First byte matched and buffer is 1 byte long, OK. */
        if (size == 1)
                return 1;

        /*
         * Check buffer longer than 1 byte. We already know that buffer[0]
         * matches the pattern, so the test below only checks whether the
         * buffer[0...size-2] == buffer[1...size-1] , which is a test for
         * whether the buffer is filled with constant value.
         */
        return !memcmp(buffer, buffer + 1, size - 1);
}

/*
 * Writing to the NAND must take into account ECC errors
 * and BAD sectors.
 * This is not required for NOR flashes
 * The function reassembles nandwrite from mtd-utils
 * dropping all options that are not required here.
 */

static void erase_buffer(void *buffer, size_t size)
{
	const uint8_t kEraseByte = 0xff;

	if (buffer != NULL && size > 0)
		memset(buffer, kEraseByte, size);
}

static int nand_flash_erase(char *dev, int offs, int len)
{
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
	unsigned int eb=0, eb_start=0, eb_cnt=0;
	int noskipbad = 0;
	int fd;
	int ret=-1;

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

	eb_start = offs / mtd.eb_size;
	//eb_cnt = (mtd.size / mtd.eb_size) - eb_start;
	eb_cnt = (len + mtd.eb_size - 1) / mtd.eb_size;
    

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

		if (mtd_erase(mtd_desc, &mtd, fd, eb) != 0) {
			ERROR("%s: MTD Erase failure", dev);
			continue;
		}
	}

erase_out:
	close(fd);
	libmtd_close(mtd_desc);

	return ret;
}

int install_nand_flash_image_wh(char *dev, ota_image *pimg, int dev_offs, char *data, int length)
{
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
	bool baderaseblock = false;
	long long offs=0;
	long long blockstart = -1;
	unsigned char *filebuf = NULL;
	size_t filebuf_max = 0;
	long long mtdoffset = 0;
    long long mtdgodbk = 0;
	int fd = -1;
	bool failed = true;
	int ret=-1;

	if (dev == NULL || pimg == NULL)
		return -1;

	offs = dev_offs;

	mtd_desc = libmtd_open();
	if (mtd_desc == NULL) {
		ERROR("can't initialize libmtd");
		return -1;
	}

	if (mtd_get_dev_info(mtd_desc, dev, &mtd) < 0) {
		ERROR("mtd_get_dev_info failed");
		libmtd_close(mtd_desc);
		return -1;
	}

	if (offs & (mtd.min_io_size - 1)) {
		ERROR("The start address is not page-aligned !\n"
			"The pagesize of this NAND Flash is 0x%x.\n",
			mtd.min_io_size);
		libmtd_close(mtd_desc);
		return -1;
	}

    if(length > mtd.eb_size) {
        ERROR("%s invlaid length=%d\n", __func__, length);
    	libmtd_close(mtd_desc);
        return -1;
    }

	if ((fd = open(dev, O_RDWR)) < 0) {
		ERROR("%s: %s: %s", __func__, dev, strerror(errno));
		libmtd_close(mtd_desc);
		return -ENODEV;
	}

	filebuf_max = mtd.eb_size;
	filebuf = calloc(1, filebuf_max);
	erase_buffer(filebuf, filebuf_max);

    blockstart = 0;
    mtdgodbk = 0;
	do {
		ret = mtd_is_bad(&mtd, fd, blockstart / mtd.eb_size);
		if (ret < 0) {
			ERROR("%s: MTD get bad block failed", dev);
            failed = true;
			goto closeall;
		} else if (ret == 1) {
			baderaseblock = true;
		}
        else {
            mtdgodbk += mtd.eb_size;
        }

		if (baderaseblock) {
            mtdoffset += mtd.eb_size;
			if (mtdoffset+offs >= mtd.size) {
				ERROR("too many bad blocks, quit");
                failed = true;
				goto closeall;
			}
		}

        blockstart +=  mtd.eb_size;
		baderaseblock = false;
	} while (mtdgodbk<=offs);

    offs = blockstart - mtd.eb_size;

    /* erase eb */
	ret = nand_flash_erase(dev, offs, mtd.eb_size);
	if (ret < 0)
		goto closeall;

	ret = 0;
    memcpy(filebuf, data, length);
	/* Write out data */
	ret = mtd_write(mtd_desc, &mtd, fd, offs / mtd.eb_size,
			offs % mtd.eb_size,
			filebuf,
			mtd.eb_size,
			NULL,
			0,
			MTD_OPS_PLACE_OOB);
	if (ret) {
		if (errno != EIO) {
			ERROR("%s: MTD write failure", dev);
            failed = true;
			goto closeall;
		}

		TRACE("Marking block at %08llx bad",
				offs & (~mtd.eb_size + 1));
	}

	failed = false;

closeall:
	free(filebuf);
	close(fd);
	libmtd_close(mtd_desc);

	if (failed) {
		ERROR("Installing image into %s failed", dev);
		return -1;
	}

	return 0;
}
#if 0
__attribute__((constructor))
void nand_flash_handler(void)
{
	register_handler("nand", install_nand_flash_image);
}
#endif
