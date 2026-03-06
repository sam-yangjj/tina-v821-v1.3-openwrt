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

static int fake_offs = 0;

static int fake_read(ota_image *pimg, char *buf, int len)
{
	int cpylen;
	int idx;
	int imglen;

	idx = pimg->zone_idx;
	imglen = pimg->head.zone[idx].image_size;
	cpylen = len;

	if (imglen - fake_offs < cpylen)
		cpylen = imglen - fake_offs;

	memcpy(buf, pimg->img_buf + fake_offs, cpylen);

	fake_offs += cpylen;

	return cpylen;
}

static int nand_flash_erase(char *dev, int offs)
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
	eb_cnt = (mtd.size / mtd.eb_size) - eb_start;

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

int install_nand_flash_image(char *dev, int dev_offs, ota_image *pimg)
{
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
	int pagelen;
	bool baderaseblock = false;
	long long imglen = 0;
	long long offs=0;
	long long blockstart = -1;
	unsigned char *filebuf = NULL;
	size_t filebuf_max = 0;
	size_t filebuf_len = 0;
	long long mtdoffset = dev_offs;
	int fd = -1;
	bool failed = true;
	int ret=-1;
	unsigned char *writebuf = NULL;
	int idx;

	if (dev == NULL || pimg == NULL)
		return -1;
#ifdef ENABLE_FLASH_ERASE_128KB_BLOCK
	offs = dev_offs;
#endif
	ret = nand_flash_erase(dev, offs);
	if (ret < 0)
		return -1;

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

	if (mtdoffset & (mtd.min_io_size - 1)) {
		ERROR("The start address is not page-aligned !\n"
			"The pagesize of this NAND Flash is 0x%x.\n",
			mtd.min_io_size);
		libmtd_close(mtd_desc);
		return -1;
	}

	pagelen = mtd.min_io_size;
	idx = pimg->zone_idx;
	imglen = pimg->head.zone[idx].image_size;
	fake_offs = 0;

	if ((imglen / pagelen) * mtd.min_io_size > mtd.size) {
		ERROR("Image %s does not fit into zone", dev);
		libmtd_close(mtd_desc);
		return -EIO;
	}

	filebuf_max = mtd.eb_size / mtd.min_io_size * pagelen;
	filebuf = calloc(1, filebuf_max);
	erase_buffer(filebuf, filebuf_max);

	if ((fd = open(dev, O_RDWR)) < 0) {
        printf("%s: %s: %s", __func__, dev, strerror(errno));
        free(filebuf);
		libmtd_close(mtd_desc);
		return -ENODEV;
	}

	/*
	 * Get data from input and write to the device while there is
	 * still input to read and we are still within the device
	 * bounds. Note that in the case of standard input, the input
	 * length is simply a quasi-boolean flag whose values are page
	 * length or zero.
	 */
	while ((imglen > 0 || writebuf < filebuf + filebuf_len)
		&& mtdoffset < mtd.size) {
		/*
		 * New eraseblock, check for bad block(s)
		 * Stay in the loop to be sure that, if mtdoffset changes because
		 * of a bad block, the next block that will be written to
		 * is also checked. Thus, we avoid errors if the block(s) after the
		 * skipped block(s) is also bad
		 */
		while (blockstart != (mtdoffset & (~mtd.eb_size + 1))) {
			blockstart = mtdoffset & (~mtd.eb_size + 1);
			offs = blockstart;

			/*
			 * if writebuf == filebuf, we are rewinding so we must
			 * not reset the buffer but just replay it
			 */
			if (writebuf != filebuf) {
				erase_buffer(filebuf, filebuf_len);
				filebuf_len = 0;
				writebuf = filebuf;
			}

			baderaseblock = false;

			do {
				ret = mtd_is_bad(&mtd, fd, offs / mtd.eb_size);
				if (ret < 0) {
					ERROR("%s: MTD get bad block failed", dev);
					goto closeall;
				} else if (ret == 1) {
					baderaseblock = true;
				}

				if (baderaseblock) {
					mtdoffset = blockstart + mtd.eb_size;

					if (mtdoffset > mtd.size) {
						ERROR("too many bad blocks, quit");
						goto closeall;
					}
				}

				offs +=  mtd.eb_size;
			} while (offs < blockstart + mtd.eb_size);
		}

		/* Read more data from the input if there isn't enough in the buffer */
		if (writebuf + mtd.min_io_size > filebuf + filebuf_len) {
			size_t readlen = mtd.min_io_size;
			size_t alreadyread = (filebuf + filebuf_len) - writebuf;
			size_t tinycnt = alreadyread;
			ssize_t cnt = 0;

			while (tinycnt < readlen) {
				cnt = fake_read(pimg, (char *)(writebuf + tinycnt),
						readlen - tinycnt);
				if (cnt == 0) { /* EOF */
					break;
				} else if (cnt < 0) {
					ERROR("File I/O error on input");
					goto closeall;
				}
				tinycnt += cnt;
			}

			/* No padding needed - we are done */
			if (tinycnt == 0) {
				imglen = 0;
				break;
			}

			/* Padding */
			if (tinycnt < readlen) {
				erase_buffer(writebuf + tinycnt, readlen - tinycnt);
			}

			filebuf_len += readlen - alreadyread;

			imglen -= tinycnt - alreadyread;
		}

		ret = 0;
		if (!buffer_check_pattern(writebuf, mtd.min_io_size, 0xff)) {
			/* Write out data */
			ret = mtd_write(mtd_desc, &mtd, fd, mtdoffset / mtd.eb_size,
					mtdoffset % mtd.eb_size,
					writebuf,
					mtd.min_io_size,
					NULL,
					0,
					MTD_OPS_PLACE_OOB);
		}
		if (ret) {
			long long i;
			if (errno != EIO) {
				ERROR("%s: MTD write failure", dev);
				goto closeall;
			}

			/* Must rewind to blockstart if we can */
			writebuf = filebuf;

			for (i = blockstart; i < blockstart + mtd.eb_size;
					i += mtd.eb_size) {
				if (mtd_erase(mtd_desc, &mtd, fd, i / mtd.eb_size)) {
					int errno_tmp = errno;
					TRACE("%s: MTD Erase failure", dev);
					if (errno_tmp != EIO)
						goto closeall;
				}
			}

			TRACE("Marking block at %08llx bad",
					mtdoffset & (~mtd.eb_size + 1));
			if (mtd_mark_bad(&mtd, fd, mtdoffset / mtd.eb_size)) {
				ERROR("%s: MTD Mark bad block failure", dev);
				goto closeall;
			}
			mtdoffset = blockstart + mtd.eb_size;

			continue;
		}

		update_writein_progress(pimg, pimg->head.zone[idx].image_size - imglen);

		mtdoffset += mtd.min_io_size;
		writebuf += pagelen;
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
