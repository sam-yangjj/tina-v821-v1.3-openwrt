#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <bcmnvram.h>

#include <linux/bio.h>
#include <linux/fs.h>

#include <linux/mtd/mtd.h>
#include <linux/major.h>
#include <linux/blkdev.h>

static unsigned int current_write_counter = 0;
static sector_t current_sector = 0;
static sector_t start_sector = (NVRAM_FLASH_OFFSET >> 9);

extern unsigned short verify_zbuf_chksum(char *zbuf_with_header);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,8,0)
static int bdev_blktrans_flush(void)
{
	struct block_device *bdev;

	bdev = blkdev_get_by_dev(MKDEV(NVRAM_FLASH_MAJOR, NVRAM_FLASH_MINOR),
			 FMODE_READ|FMODE_WRITE, NULL);
	if (IS_ERR(bdev)) {
		pr_err("get blkdev err\n");
		return -EINVAL;
	}

	if (bdev->bd_disk->fops->ioctl)
		return bdev->bd_disk->fops->ioctl(bdev, 0, BLKFLSBUF, 0);

	return -ENOTTY;
}

static inline unsigned int bio_allowed_max_sectors(struct request_queue *q)
{
	return round_down(UINT_MAX, queue_logical_block_size(q)) >> 9;
}

static int __blkdev_issue_discard_tuyaos(unsigned int rw, sector_t start, sector_t nr_sects,
			char *buf, size_t len, struct bio **biop)
{
	struct bio *bio = *biop;
	sector_t bs_mask;
	struct block_device *bdev;
	struct request_queue *q;
	struct page *page;
	unsigned int sz;
	int ret;

	unsigned int req_sects = nr_sects;
	unsigned int req_sects_tmp = req_sects;

	struct mtd_info *mtd;
	loff_t ofs = start<<9;
	unsigned int blk_id=0;

	bdev = blkdev_get_by_dev(MKDEV(NVRAM_FLASH_MAJOR, NVRAM_FLASH_MINOR),
			 FMODE_READ|FMODE_WRITE, NULL);
	if (IS_ERR(bdev)) {
		pr_err("get blkdev err\n");
		return -EINVAL;
	}
#ifdef CONFIG_MTD
    if(NVRAM_FLASH_MAJOR == MTD_BLOCK_MAJOR)
    {
        mtd = get_mtd_device(NULL, NVRAM_FLASH_MINOR);
        if(!mtd)
        {
            pr_err("no mtd device minor=%d\n", NVRAM_FLASH_MINOR);
            return -ENODEV;
        }

        if(((mtd->type == MTD_MLCNANDFLASH) || (mtd->type == MTD_NANDFLASH)))
        {
            // defualt start is align to erase_size
	    blk_id = ofs & mtd->erasesize_mask;
            if(blk_id)
            {
                pr_err("start addr is not align on erase size, start=0x%llx mtd->erasesize=0x%x\n", ofs, mtd->erasesize);
                return -EINVAL;
            }

            ret = mtd_block_isbad(mtd, ofs);
            if(ret)
            {
                pr_err("block is badblock, addr=0x%llx\n", ofs);
                put_mtd_device(mtd);
                return -EDEADLK;
            }
        }
        put_mtd_device(mtd);
    }
#endif
	q = bdev_get_queue(bdev);
	if (!q)
		return -ENXIO;

	bs_mask = (bdev_logical_block_size(bdev) >> 9) - 1;
	if ((start | nr_sects) & bs_mask)
		return -EINVAL;

	while (nr_sects) {

		sector_t end_sect;

		if (!req_sects)
			goto fail;

		req_sects = min(req_sects, bio_allowed_max_sectors(q));

		req_sects = min((sector_t)req_sects, (sector_t)BIO_MAX_PAGES);
		bio = bio_alloc(GFP_KERNEL, req_sects);
		if (!bio) {
			ret = -ENOMEM;
			goto fail;
		}

		req_sects_tmp = req_sects;
		end_sect = start + req_sects;
		while (req_sects != 0) {
			sz = min((sector_t) PAGE_SIZE >> 9 , (sector_t)req_sects);
			if (sz == 0)
			{
				printk("%s %d \n", __func__, __LINE__);
				break;
			}
			page = virt_to_page(buf);
			ret = bio_add_page(bio, page, sz << 9, 0);
			req_sects -= ret >> 9;
			buf += PAGE_SIZE;
			if (ret < (sz << 9))
			{
				printk("%s %d ret=%d sz=%u\n", __func__, __LINE__, ret, sz);
				break;
			}
		}

		bio->bi_iter.bi_sector = start;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
		bio_set_dev(bio, bdev);
		bio->bi_opf = rw;
#else
                bio->bi_bdev = bdev;
		bio_set_op_attrs(bio, rw, 0);
#endif
		bio->bi_iter.bi_size = req_sects_tmp << 9;
		nr_sects -= req_sects_tmp;
		start = end_sect;

		/*
		 * We can loop for a long time in here, if someone does
		 * full device discards (like mkfs). Be nice and allow
		 * us to schedule out to avoid softlocking if preempt
		 * is disabled.
		 */
		cond_resched();
	}

	*biop = bio;
	return 0;

fail:
	if (bio) {
		submit_bio_wait(bio);
		bio_put(bio);
	}
	*biop = NULL;
	return -EOPNOTSUPP;
}

int nvram_bio_rw(unsigned int rw, sector_t start, sector_t nr_sects,
			char *buf, size_t len)
{
	struct bio *bio = NULL;
	struct blk_plug plug;
	int ret;

	blk_start_plug(&plug);
	ret = __blkdev_issue_discard_tuyaos(rw, start, nr_sects, buf, len,
			&bio);
	if (!ret && bio) {
		ret = submit_bio_wait(bio);
		if (ret == -EOPNOTSUPP)
		{
			printk("%s %d submit_bio_wait fail!\n", __func__, __LINE__);
			ret = 0;
		}
		bio_put(bio);
	}
	bdev_blktrans_flush();
	blk_finish_plug(&plug);
	return ret;
}
#else

struct bio_batch {
	atomic_t		done;
	int			error;
	struct completion	*wait;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
static void bio_batch_end_io(struct bio *bio)
{
	struct bio_batch *bb = bio->bi_private;

	if (bio->bi_error && bio->bi_error != -EOPNOTSUPP)
		bb->error = bio->bi_error;
	if (atomic_dec_and_test(&bb->done))
		complete(bb->wait);
	bio_put(bio);
}
#else
static void bio_batch_end_io(struct bio *bio, int err)
{
	struct bio_batch *bb = bio->bi_private;

        if (err && (err != -EOPNOTSUPP))
		bb->error = err;
        if (atomic_dec_and_test(&bb->done))
                complete(bb->wait);
        bio_put(bio);
}
#endif

static int nvram_bio_rw(unsigned int rw, sector_t start, sector_t nr_sects,
			char *buf, size_t len)
{
	DECLARE_COMPLETION_ONSTACK(wait);
	struct block_device *bdev;
	struct bio_batch bb;
	struct bio *bio;
	struct page *page;
	unsigned int sz;
	int ret = 0;

	struct mtd_info *mtd;
	loff_t ofs = start<<9;
	unsigned int blk_id=0;

	bdev = blkdev_get_by_dev(MKDEV(NVRAM_FLASH_MAJOR, NVRAM_FLASH_MINOR),
				 FMODE_READ|FMODE_WRITE, NULL);
	if (IS_ERR(bdev)) {
		pr_err("get blkdev err\n");
		return ret;
	}

#ifdef CONFIG_MTD

    if(NVRAM_FLASH_MAJOR == MTD_BLOCK_MAJOR)
    {
        mtd = get_mtd_device(NULL, NVRAM_FLASH_MINOR);
        if(!mtd)
        {
            pr_err("no mtd device minor=%d\n", NVRAM_FLASH_MINOR);
            return -ENODEV;
        }

        if(((mtd->type == MTD_MLCNANDFLASH) || (mtd->type == MTD_NANDFLASH)))
        {
            // defualt start is align to erase_size
	    blk_id = ofs & mtd->erasesize_mask;
            if(blk_id)
            {
                pr_err("start addr is not align on erase size, start=0x%llx mtd->erasesize=0x%x\n", ofs, mtd->erasesize);
                return -EINVAL;
            }

            ret = mtd_block_isbad(mtd, ofs);
            if(ret)
            {
                pr_err("block is badblock, addr=0x%llx\n", ofs);
                put_mtd_device(mtd);
                return -EDEADLK;
            }
        }
        put_mtd_device(mtd);
    }
#endif

	atomic_set(&bb.done, 1);
	bb.error = 0;
	bb.wait = &wait;

	while (nr_sects != 0) {
		bio = bio_alloc(GFP_KERNEL, min(nr_sects, (sector_t)BIO_MAX_PAGES));
		if (!bio) {
			ret = -ENOMEM;
			goto out;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
		bio->bi_iter.bi_sector = start;
#else
		bio->bi_sector = start;
#endif
		bio->bi_end_io = bio_batch_end_io;
		bio->bi_private = &bb;
		bio->bi_bdev = bdev;

		while (nr_sects != 0) {
			sz = min((sector_t) PAGE_SIZE >> 9 , nr_sects);
			if (sz == 0)
				break;

			page = virt_to_page(buf);
			ret = bio_add_page(bio, page, sz << 9, 0);
			nr_sects -= ret >> 9;
			buf += PAGE_SIZE;
			if (ret < (sz << 9))
				break;
		}
	}

	ret = 0;

	atomic_inc(&bb.done);
	submit_bio(rw, bio);

	/* Wait for bios in-flight */
	if (!atomic_dec_and_test(&bb.done))
		wait_for_completion_io(&wait);

	if (bb.error)
		ret = bb.error;

#ifdef CONFIG_MTD
       if (bdev->bd_disk->fops->ioctl)
           return bdev->bd_disk->fops->ioctl(bdev, 0, BLKFLSBUF, 0);
#endif
out:
	blkdev_put(bdev, FMODE_READ|FMODE_WRITE);
	return ret;
}
#endif

static int inline nvram_bio_write(sector_t start, sector_t nr_sects, char *buf, size_t len)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	return nvram_bio_rw(REQ_OP_WRITE | REQ_NOWAIT, start, nr_sects, buf, len);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4,8,0)
	return nvram_bio_rw(REQ_OP_WRITE, start, nr_sects, buf, len);
#else
	return nvram_bio_rw(WRITE_FLUSH, start, nr_sects, buf, len);
#endif
}

static int inline nvram_bio_read(sector_t start, sector_t nr_sects, char *buf, size_t len)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,8,0)
	return nvram_bio_rw(REQ_OP_READ, start, nr_sects, buf, len);
#else
	return nvram_bio_rw(READ, start, nr_sects, buf, len);
#endif
}

int _nvram_check(struct nvram_header *header)
{
	if (header->magic == 0xFFFFFFFF || header->magic == 0)
		return 1; // NVRAM MTD is clear

	if (header->magic != NVRAM_MAGIC)
		return -1; // NVRAM is garbage

	if (header->len < sizeof(struct nvram_header))
		return -2; // NVRAM size underflow

	if (header->len > NVRAM_SPACE)
		return -3; // NVRAM size overflow

	if (verify_zbuf_chksum((char*)header))
		return -4; // NVRAM content is corrupted

	return 0;
}

static int nvram_blkdev_get_total_size(sector_t *nr_sects)
{
    struct block_device *bdev;

    bdev = blkdev_get_by_dev(MKDEV(NVRAM_FLASH_MAJOR, NVRAM_FLASH_MINOR),
                             FMODE_READ|FMODE_WRITE, NULL);
    if (IS_ERR(bdev)) {
        pr_err("get blkdev err\n");
        return -ENODEV;
    }

    *nr_sects = (bdev->bd_part->nr_sects); // unit 512byte
    return 0;
}

int nvram_flash_read_zbuf(char *zbuf, u_int32_t len)
{
	int ret;
	struct nvram_header *header = (struct nvram_header*)zbuf;
	sector_t off;
	sector_t unit_sectors = NVRAM_SPACE>>9;
    sector_t nr_sects;

	current_write_counter = 0;
    ret = nvram_blkdev_get_total_size(&nr_sects);
    if(ret)
    {
        pr_err("%s %d get blkdev total size fail ret=%d\n", __func__, __LINE__, ret);
    }

	for (off = 0; (off+unit_sectors) <= nr_sects; off += unit_sectors) {
		ret = nvram_bio_read(start_sector+off, unit_sectors, zbuf, len);
		if (ret < 0)
			continue;

		ret = _nvram_check(header);
		if (ret)
			continue;

		if (current_write_counter <= header->write_counter) {
			current_write_counter = header->write_counter;
			current_sector = start_sector + off;
		}
	}

	ret = nvram_bio_read(current_sector, unit_sectors, zbuf, len);
	if (ret < 0) {
		return -1;
	}

	pr_debug("%s: offs: 0x%lx, len: %d\n", __func__, current_sector, len);

	return 0;
}

int nvram_flash_write_zbuf(char *zbuf, u_int32_t len)
{
	int ret;
	struct nvram_header *header = (struct nvram_header*)zbuf;
	sector_t unit_sectors;
    int retry=0;
    sector_t nr_sects;

    ret = nvram_blkdev_get_total_size(&nr_sects);
    if(ret)
    {
        pr_err("%s %d get blkdev total size fail ret=%d\n", __func__, __LINE__, ret);
    }

	current_write_counter++;
	header->write_counter = current_write_counter;

	unit_sectors = NVRAM_SPACE>>9;
    retry = nr_sects/unit_sectors;

    while(retry)
    {
        retry--;
        current_sector += unit_sectors;
        if (current_sector + unit_sectors > start_sector + nr_sects)
            current_sector = start_sector;

        ret = nvram_bio_write(current_sector, unit_sectors, zbuf, len);
        if(ret == -EDEADLK)
            continue;
        else if (ret < 0) {
            printk("nvram flash nvram_bio_write fail ret=%d\n", ret);
            return -1;
        }
        else if(ret == 0) {
            break;
        }
    }

    if(!retry)
        printk("nvram flash only remaining one block!\n");

	pr_debug("%s: offs: 0x%lx, len: %d, header->len %d\n", __func__, current_sector, len, header->len);

	return 0;
}

EXPORT_SYMBOL(nvram_flash_write_zbuf);
EXPORT_SYMBOL(nvram_flash_read_zbuf);
