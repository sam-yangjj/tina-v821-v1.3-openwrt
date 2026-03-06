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

#include "bsp_sha256.h"
/* for img write ahead check */
static sha2_context sha2_ctx_2;
static unsigned char img_hmac_2[32];

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

static int copy_read(int *out, char *buf, unsigned int len)
{
	int ret;
	int fd = (out != NULL) ? *out : -1;

	while (len) {
		errno = 0;
		ret = read(fd, (void *)buf, len);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			ERROR("cannot read %d bytes: %s", len, strerror(errno));
			return -1;
		}

		if (ret == 0) {
			ERROR("cannot read %d bytes: %s", len, strerror(errno));
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
	int ret=-1;
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

int install_nor_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer)
{
	int ret=0;
	int len;
	int left = 0;
    char *bkdev, *realdev;
    int erase_sz;

	unsigned int eb, bkeb, eb_start, ebbk_start, eb_cnt;
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
    int fd, bkfd;
	int noskipbad = 0;
	int noskipbad_bk = 0;

    bkdev =  pimg->head.zone[bk_idx].device[0];
    realdev =  pimg->head.zone[idx].device[0];
    
	if (bkdev == NULL  || realdev == NULL || pimg == NULL)
		return -1;

    TRACE("installing %s to %s offset=%d length=%d...", bkdev, realdev, offset, length);

    erase_sz = get_device_geometry(realdev);

	mtd_desc = libmtd_open();
	if (mtd_desc == NULL) {
		ERROR("can't initialize libmtd");
		return -1;
	}

	if ((fd = open(realdev, O_RDWR)) < 0) {
		ERROR("%s: %s: %s", __func__, realdev, strerror(errno));
		libmtd_close(mtd_desc);
		return -ENODEV;
	}

    if ((bkfd = open(bkdev, O_RDWR)) < 0) {
            ERROR("%s: %s: %s", __func__, bkdev, strerror(errno));
    		libmtd_close(mtd_desc);
            close(fd);
            return -ENODEV;
    }

	if (mtd_get_dev_info(mtd_desc, realdev, &mtd) < 0) {
		ERROR("mtd_get_dev_info failed");
        ret = -ENODEV;
		goto erase_out;
	}

    len = erase_sz;//WRITE_SIZE
	//idx = pimg->zone_idx;
	left = length;

	eb_start = offset / erase_sz;
	eb_cnt = ((length+erase_sz-1) / erase_sz);

    eb = eb_start;
    ebbk_start = pimg->head.zone[bk_idx].device_offset / erase_sz;
    bkeb = ebbk_start;


	ret = nor_flash_erase(realdev, offset, length);
	if (ret < 0)
		goto erase_out;

    memset(img_hmac_2, 0, 32);
    bsp_sha2_starts(&sha2_ctx_2, 0);

	for (; (eb < eb_start + eb_cnt) && (bkeb < ebbk_start + eb_cnt); )
    {
		/* Always skip bad sectors */
		if (!noskipbad) {
			ret = mtd_is_bad(&mtd, fd, eb);
			if (ret > 0) {
                eb++;
				TRACE("skip bad block");
				continue;
			} else if (ret < 0) {
				if (errno == EOPNOTSUPP) {
					noskipbad = 1;
				} else {
					ERROR("%s: MTD get bad block failed", realdev);
					ret = -EFAULT;
					goto erase_out;
				}
			}
		}
		/* Unlock memory if required */
		if (mtd_is_locked(&mtd, fd, eb) > 0) {
			if (mtd_unlock(&mtd, fd, eb) != 0) {
				if (errno != EOPNOTSUPP) {
					TRACE("%s: MTD unlock failure", realdev);
                    eb++;
					continue;
				}
			}
		}

		/* Always skip bad sectors */
		if (!noskipbad_bk) {
			ret = mtd_is_bad(&mtd, bkfd, bkeb);
			if (ret > 0) {
				TRACE("skip bad block");
                bkeb++;
				continue;
			} else if (ret < 0) {
				if (errno == EOPNOTSUPP) {
					noskipbad_bk = 1;
				} else {
					ERROR("%s: MTD get bad block failed", bkdev);
					ret = -EFAULT;
					goto erase_out;
				}
			}
		}
		/* Unlock memory if required */
		if (mtd_is_locked(&mtd, bkfd, bkeb) > 0) {
			if (mtd_unlock(&mtd, bkfd, bkeb) != 0) {
				if (errno != EOPNOTSUPP) {
					TRACE("%s: MTD unlock failure", bkdev);
                    bkeb++;
					continue;
				}
			}
		}        

    	if (bkeb > 0) {
    		if (lseek(bkfd, bkeb*erase_sz, SEEK_SET) < 0) {
    			ERROR("offset argument: seek failed");
				ret = -EFAULT;
				goto erase_out;
    		}
    	}

		if (left < erase_sz)
			len = left;

        //mtd_read
        ret = copy_read(&bkfd, buffer, len);
		if (ret < 0) {
			ret = -ENOSPC;
			goto erase_out;
		}

#if 0
        TRACE("%s %d erase eb=%d ", __func__, __LINE__, eb);
		if (mtd_erase(mtd_desc, &mtd, fd, eb) != 0) {
			ERROR("%s: MTD Erase failure", realdev);
			ret = -EFAULT;
			goto erase_out;
		}
#endif
    	if (eb > 0) {
    		if (lseek(fd, eb*erase_sz, SEEK_SET) < 0) {
    			ERROR("offset argument: seek failed");
				ret = -EFAULT;
				goto erase_out;
    		}
    	}

        ret = copy_write(&fd, buffer, len);
		if (ret < 0) {
			ret = -ENOSPC;
			goto erase_out;
		}

		bsp_sha2_update(&sha2_ctx_2, (unsigned char *)buffer, len);

        eb++;
        bkeb++;
        left -= (len);
		update_writein_progress(pimg, eb);
#if defined(ENABLE_FLASH_ERASE_WRITE_SLEEP) && (ENABLE_FLASH_ERASE_WRITE_SLEEP==1)
		usleep(SLEEP_INTERVAL);
#endif

	}
    
	bsp_sha2_finish(&sha2_ctx_2, img_hmac_2);

	if (memcmp(pimg->head.zone[idx].sha256, img_hmac_2, 32) != 0) {
    	ERROR("bad hmac, abort upgrade! ");
		ret = -ENOSPC;
	}
    else
    {
        ret=0;
        TRACE("%s %d eb=%d bkeb=%d eb_cnt=%d ", __func__, __LINE__, eb, bkeb, eb_cnt);
    }

erase_out:
	close(fd);
	close(bkfd);
	libmtd_close(mtd_desc);
    return ret;
}
