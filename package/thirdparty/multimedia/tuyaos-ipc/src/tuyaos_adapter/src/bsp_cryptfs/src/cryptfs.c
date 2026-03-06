#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/dm-ioctl.h>

#include "loop.h"

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif

#define DM_CRYPT_BUF_SIZE 4096
#define DM_CRYPT_SECTOR_SIZE 512
#define DM_CRYPT_SECTORS_PER_BUFSIZE (DM_CRYPT_BUF_SIZE / DM_CRYPT_SECTOR_SIZE)
#define TABLE_LOAD_RETRIES 10
#define MAX_PATH_LEN 256
#define DM_CRYPT_CIPHER "aes-cbc-essiv:sha256"

static unsigned long get_blkdev_size(const char *blkdev)
{
	unsigned long nr_sec;
	int fd;

	fd = open(blkdev, O_RDONLY|O_CLOEXEC);
	if (fd < 0)
		return 0;

	if ((ioctl(fd, BLKGETSIZE, &nr_sec)) == -1) {
		nr_sec = 0;
	}

	close(fd);

	return nr_sec;
}

static void dm_ioctl_init(struct dm_ioctl *io, size_t size, const char *name, unsigned flags)
{
	memset(io, 0, size);
	io->data_size = size;
	io->data_start = sizeof(struct dm_ioctl);
	io->version[0] = 4;
	io->version[1] = 0;
	io->version[2] = 0;
	io->flags = flags;
	if (name)
		strncpy(io->name, name, sizeof(io->name));
}

static int dm_crypt_load_table(int fd, const char *blkdev, const char *name,
			       const char *key)
{
	char buffer[DM_CRYPT_BUF_SIZE];
	struct dm_ioctl *io;
	struct dm_target_spec *tgt;
	char *crypt_params;
	size_t buff_offset;
	int i;

	io = (struct dm_ioctl *) buffer;
	tgt = (struct dm_target_spec *) (buffer + sizeof(struct dm_ioctl));
	crypt_params = buffer + sizeof(struct dm_ioctl) + sizeof(struct dm_target_spec);

	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	io->target_count = 1;
	tgt->status = 0;
	tgt->sector_start = 0;
	tgt->length = get_blkdev_size(blkdev);
	strncpy(tgt->target_type, "crypt", DM_MAX_TYPE_NAME);
	buff_offset = crypt_params - buffer;
	snprintf(crypt_params, sizeof(buffer) - buff_offset, "%s %s 0 %s 0",
		 DM_CRYPT_CIPHER, key, blkdev);
	crypt_params += strlen(crypt_params) + 1;
	crypt_params = (char *) (((unsigned long)crypt_params + 7) & ~8); /* Align to an 8 byte boundary */
	tgt->next = crypt_params - buffer;

	for (i = 0; i < TABLE_LOAD_RETRIES; i++) {
		if (! ioctl(fd, DM_TABLE_LOAD, io)) {
			break;
		}
		usleep(500000);
	}

	if (i == TABLE_LOAD_RETRIES)
		return -1;

	return 0;
}

static int dm_crypt_create_dev(const char *blkdev, const char *name,
			       const char *key)
{
	char buffer[DM_CRYPT_BUF_SIZE];
	struct dm_ioctl *io;
	int minor;
	int fd;
	int err;

	fd = open("/dev/mapper/control", O_RDWR|O_CLOEXEC);
	if (fd < 0)
		return -EBADF;

	io = (struct dm_ioctl *) buffer;
	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	err = ioctl(fd, DM_DEV_CREATE, io);
	if (err < 0) {
		printf("Create devmapper device failed: %s\n", strerror(errno));
		goto err_create;
	}

	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	ioctl(fd, DM_DEV_STATUS, io);
	if (err < 0) {
		printf("Get device status failed:%d\n", err);
		goto err_status;
	}
	minor = (io->dev & 0xff) | ((io->dev >> 12) & 0xfff00);

	err = dm_crypt_load_table(fd, blkdev, name, key);
	if (err < 0)
		goto err_status;

	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	err = ioctl(fd, DM_DEV_SUSPEND, io);
	if (err < 0) {
		printf("Resume device failed:%d\n", err);
		goto err_status;
	}

	close(fd);
	return minor;

err_status:
	io = (struct dm_ioctl *) buffer;
	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	ioctl(fd, DM_DEV_REMOVE, io);
err_create:
	close(fd);
	return err;
}

static int dm_crypt_remove_dev(const char *name)
{
	char buffer[DM_CRYPT_BUF_SIZE];
	struct dm_ioctl *io;
	int fd;
	int err;

	fd = open("/dev/mapper/control", O_RDWR|O_CLOEXEC);
	if (fd < 0)
		return -EBADF;

	io = (struct dm_ioctl *) buffer;
	dm_ioctl_init(io, DM_CRYPT_BUF_SIZE, name, 0);
	err = ioctl(fd, DM_DEV_REMOVE, io);

	close(fd);

	return err;
}

int cryptfs_create(const char *blkdev, const char *key)
{
	char name[MAX_PATH_LEN];
	char cryptdev[MAX_PATH_LEN];
	char buf[DM_CRYPT_BUF_SIZE];
	char *loopdev;
	int fd_loopdev, fd_cryptdev;
	unsigned long fs_size, numblocks, remainder;
	int minor, i, err = 0;

	loopdev = loopdev_create(blkdev);
	if (!loopdev) {
		printf("Create loop device failed\n");
		err = -ENODEV;
		goto err_loop;
	}

	fs_size = get_blkdev_size(loopdev);

	snprintf(name, MAX_PATH_LEN, "crypt-%ld", random());
	minor = dm_crypt_create_dev(loopdev, name, key);
	if (minor < 0) {
		printf("dm-crypt create failed\n");
		err = -ENODEV;
		goto err_crypt;
	}

	snprintf(cryptdev, MAX_PATH_LEN, "/dev/dm-%d", minor);

	if ((fd_loopdev = open(loopdev, O_RDWR|O_CLOEXEC)) < 0) {
		printf("Open loop device: %s failed\n", loopdev);
		err = -EBADF;
		goto err_loopfd;
	}

	if ((fd_cryptdev = open(cryptdev, O_RDWR|O_CLOEXEC)) < 0) {
		printf("Open crypt device: %s failed\n", cryptdev);
		err = -EBADF;
		goto err_cryptfd;
	}

	numblocks = fs_size / DM_CRYPT_SECTORS_PER_BUFSIZE;
	remainder = fs_size % DM_CRYPT_SECTORS_PER_BUFSIZE;

	for (i = 0; i < numblocks; i++) {
		if (read(fd_loopdev, buf, DM_CRYPT_BUF_SIZE) != DM_CRYPT_BUF_SIZE) {
			err = -EIO;
			goto err_io;
		}
		if (write(fd_cryptdev, buf, DM_CRYPT_BUF_SIZE) != DM_CRYPT_BUF_SIZE) {
			err = -EIO;
			goto err_io;
		}
	}

	for (i = 0; i < remainder; i++) {
		if (read(fd_loopdev, buf, DM_CRYPT_SECTOR_SIZE) != DM_CRYPT_SECTOR_SIZE) {
			err = -EIO;
			goto err_io;
		}
		if (write(fd_cryptdev, buf, DM_CRYPT_SECTOR_SIZE) != DM_CRYPT_SECTOR_SIZE) {
			err = -EIO;
			goto err_io;
		}
	}

err_io:
	close(fd_cryptdev);
err_cryptfd:
	close(fd_loopdev);
err_loopfd:
	dm_crypt_remove_dev(name);
err_crypt:
	loopdev_destroy(loopdev);
err_loop:
	return err;
}

int cryptfs_enable(const char *blkdev, const char *name, const char *key)
{
	return dm_crypt_create_dev(blkdev, name, key);
}

int cryptfs_disable(const char *name)
{
	return dm_crypt_remove_dev(name);
}
