 /**
 * @file tkl_storage_api.c
 * @brief Common process - storage: sdCard U-Disk HDD SSD etc.
 * @version 0.1
 * @date 2021-08-31
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <tkl_storage.h>

#define UEVENT_MSG_LEN  1024
 
#define INTERNAL_DIR            ("/tmp/")
#define TKL_FDISK_PARM_FILE    ("tkl_fdisk_doing.txt")

static int excute_shell_cmd(const char* cmd, char* buffer, int* len)
{
        FILE *fp = NULL;
        int i = 0;

        if (!cmd || !buffer || !len)
                return -1;

        fp = popen(cmd, "r");
        if (fp == NULL)
                return -1;

        while (fgets(buffer, *len, fp) != NULL) {
                // should do something
        }

        for (i = 0; i < *len; i++) {
                if (buffer[i] == '\n' || buffer[i] == '\r') {
                        buffer[i] = '\0';
                        break;
                }
        }

        buffer[*len - 1] = '\0';
        *len = strlen(buffer);

        pclose(fp);
        fp = NULL;

        return 0;
}

/**
 * @brief get internal r/w dir
 * 
 * @param path absolute path returned on success
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_get_internal_dir(CHAR_T path[MAX_PATH_LEN])
{
	strncpy(path, INTERNAL_DIR, 64);
	return 0;
}


/**
 * @brief report filesystem disk space usage
 * 
 * @param mount_path mount point
 * @param df fs state loaded on success
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_df(CHAR_T *mount_point, TKL_STORAGE_DF_T *df)
{
	extern int df_main(CHAR_T *mount_dir, TKL_STORAGE_DF_T *df);
	return df_main(mount_point, df);
}

/**
 * @brief get one device list
 * 
 * @param list device list loaded on success
 * @return OPERATE_RET return list number on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_get_device_info(CHAR_T *dev_name, TKL_STORAGE_DEVICE_INFO_T *list)
{
    extern int list_devs_in_one_device(CHAR_T *dev_name, TKL_STORAGE_DEVICE_INFO_T *list);
	return list_devs_in_one_device(dev_name, list);
}


/**
 * @brief get block device list
 * 
 * @param list device list loaded on success
 * @return OPERATE_RET return list number on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_get_device_list(TKL_STORAGE_DEVICE_INFO_T *list, int num)
{
	extern int list_devs_in_proc_partititons(TKL_STORAGE_DEVICE_INFO_T *list, int num);
	return list_devs_in_proc_partititons(list, num);
}

/**
 * @brief get partition info
 * 
 * @param dev_name device node wanted to list
 * @param list partition list loaded on success
 * @return OPERATE_RET return list number on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_get_part_list(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, int num)
{
	extern int list_device_partitions(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, int num);
	return list_device_partitions(dev_name, list, num);
}

/**
 * @brief make partition. refer to parted/fdisk
 * 
 * @param part_name partition label, optional
 * @param part_type partition type, eg. primary logical extended
 * @param fs_type filesystem type, eg. ext4 xfs, optional
 * @param isadd part is add or delet, 1:add, 2:delete
 * @param part_index means part number, eg. primary[1-4] logical [5 or over]
 * @param start partition start position, KB
 * @param end partition end position, KB
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_make_part(CHAR_T *part_name, CHAR_T *part_type,
    CHAR_T *fs_type, BOOL_T isadd, UINT_T part_index, LONG_T start, LONG_T end)
{
	int ret;
	char parmbuf[128] = {0};
	char cmd[256] = {0};
	char buffer[128] = {0};
	int len = sizeof(buffer);

	if (part_name == NULL || part_type == NULL || fs_type == NULL || isadd == 0 || start == 0 || end == 0)
		return OPRT_INVALID_PARM;

	/* 拼装命令 */
	memset(parmbuf, 0, sizeof(parmbuf));
	if(PART_ADD == isadd)
	{
		snprintf(parmbuf, sizeof(parmbuf), "n\n%s\n", part_type);
		
		// for logical part do not need index OR 4th primary part
		if (part_index) {
			snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "%u\n", part_index);
		}

		if (start == -1) {
			snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "\n");
		} else {
			snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "%ld\n", start * 2);
		}

		if (end == -1) {
			snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "\n");
		} else {
			snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "%ld\n", end * 2);
		}

		snprintf(parmbuf + strlen(parmbuf), sizeof(parmbuf) - strlen(parmbuf), "w\nq\n");
	} else if (PART_DEL == isadd) {
		if (part_index) {
			snprintf(parmbuf, sizeof(parmbuf), "d\n%u\nw\nq\n", part_index);
		} else {
			snprintf(parmbuf, sizeof(parmbuf), "d\nw\nq\n");
		}
	} else {
		printf("invalid isadd = %d\n",isadd);
		return OPRT_COM_ERROR;
	}

	/* 卸载分区 */
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s*", part_name);
	tkl_storage_umount(cmd, 0);

	/* 执行命令 */
	// echo -e "d\n2\nw\n" | fdisk /dev/mmcblk0
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "echo -e \"%s\" | fdisk %s", parmbuf, part_name);
	ret = excute_shell_cmd(cmd, buffer, &len);
	if (ret < 0) {
		printf("fdisk err:%d, len %d, buf %s\n", ret, len, buffer);
		return OPRT_COM_ERROR;
	}

	sleep(1);
	return OPRT_OK;
}
 

 /**
 * @brief partiton format, build filesystem
 * 
 * @param fs_type filesystem type, eg. ext4 fat32
 * @param fs_dev device node, eg. /dev/sda1
 * @param parm mkfs parame, eg. mkfs.vfat -F 32 -n "name" /dev/sda1 
 * @param cb progress callback, optional
 * @param user_ctx caller's private context, optional
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
 OPERATE_RET tkl_storage_mkfs(CHAR_T *fs_type, CHAR_T *fs_dev, CHAR_T *parm,
	TKL_STORAGE_PROGRESS_CB cb, VOID *user_ctx)
{
	int ret;
	char cmd[256] = {0};
	char buffer[128] = {0};
	int len = sizeof(buffer);

	if (fs_type == NULL || fs_dev == NULL)
		return OPRT_INVALID_PARM;

	if(parm) {
		snprintf(cmd, sizeof(cmd), "mkfs.%s %s %s", fs_type, parm, fs_dev);
	} else {
		snprintf(cmd, sizeof(cmd), "mkfs.%s %s", fs_type, fs_dev);
	}
	
	ret = excute_shell_cmd(cmd, buffer, &len);
	if (ret < 0)
		return OPRT_COM_ERROR;

	printf("len %d \n buf %s\n", len, buffer);

	if (cb)
		cb(100, user_ctx);

	return OPRT_OK;
}


/**
 * @brief mount a filesystem partition. refer to mount system call
 * 
 * @param source device node
 * @param target dir wanted mount to
 * @param fs_type filesystem type
 * @param flags filesystem r/w/a flags, optional
 * @param data filesystem-specific params, optional
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_mount(CHAR_T *source, CHAR_T *target,
    CHAR_T *fs_type, ULONG_T flags, VOID *data)
{
	return mount(source, target, fs_type, flags, data);
}


/**
 * @brief umount a filesystem partition. refer to umount system call
 * 
 * @param target dir fs mounted
 * @param flags reserved flags
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_umount(CHAR_T *target, INT_T flags)
{
	return umount2(target, flags);
}


static int open_uevent_socket()
{
    struct sockaddr_nl addr;
    int sz = 10 * 1024; /* we need a big buffer for udev event */
    int fd;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    /* set to 0 to let kernel take care the ID */
    addr.nl_pid = 0;
    addr.nl_groups = 0xffffffff;

    fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (fd < 0) {
        printf("%s: open socket failed errno:%d\n", __func__, -errno);
        return -errno;
    }

    setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("%s: bind socket failed errno:%d\n", __func__, -errno);
        close(fd);
        return -errno;
    }

    return fd;
}

int get_bus_type(char *name, char *type)
{
	FILE *procpt;
	char line[8], devname[MAX_PATH_LEN];
	int ret;
	char *nameend;

	if (!name || !type)
		return -1;

	if (strstr(name, "mmcblk")) {
		strncpy(type, BUSTP_SDIO, 8);
		return 0;
	}

	nameend = strrchr(name, '/');
	if(nameend) {
		nameend++;
	} else {
		nameend=name;
	}

	snprintf(devname, MAX_PATH_LEN, "/sys/block/%s/removable", nameend);

	procpt = fopen(devname, "r");
	if (procpt == NULL)
	{
        printf("%s %d fopen %s fail!\n", __func__, __LINE__, devname);
		return -1;
	}

	//ret = fgets(line, sizeof(line), procpt);
	ret = fread(line, sizeof(line), 1, procpt);
	if (ret < 0)
	{
        printf("%s %d read %s fail! or END .ret=%d\n", __func__, __LINE__, devname, ret);
        fclose(procpt);
		return -1;
	}

	if (atoi(line) == 1) {
		strncpy(type, BUSTP_USB, 8);
	} else {
		strncpy(type, BUSTP_SATA, 8);
	}

    fclose(procpt);
	return 0;
}

static int parse_uevent_msg(char* msg, TKL_STORAGE_EVENT_MSG *pevt)
{
	char dev_type[64] = {0};
	char dev_name[64] = {0};

    while (*msg) {
        if (!strncmp(msg, "ACTION=", sizeof("ACTION=") - 1)) {
            msg += sizeof("ACTION=") - 1;
            if (!strncmp(msg, "add", sizeof("add") - 1)) {
                pevt->type = EVT_HOTPLUG;
                pevt->msg.i = DEVICE_HOTPLUG_ADD;
            } else if (!strncmp(msg, "remove", sizeof("remove") - 1)) {
                pevt->type = EVT_HOTPLUG;
                pevt->msg.i = DEVICE_HOTPLUG_DEL;
	    	} else {
                printf("%s: invalid usb storage event:%s\n", __func__, msg);
                return -EINVAL;
            }
        } else if (!strncmp(msg, "DEVNAME=", sizeof("DEVNAME=") - 1)) {
            msg += sizeof("DEVNAME=") - 1;
            strncpy(dev_name, msg, strlen(msg) + 1);
        } else if (!strncmp(msg, "DEVTYPE=", sizeof("DEVTYPE=") - 1)) {
            msg += sizeof("DEVTYPE=") - 1;
            strncpy(dev_type, msg, strlen(msg) + 1);
        }
        /* we have already add two '\0' at the end of msg */
        while (*msg++);
    }

	//printf("msg: %d %s %s \n", pevt->msg.i, pevt->dev_name, dev_type);

	if (strcmp(dev_type, "disk") == 0) {
		get_bus_type(dev_name, pevt->bus_type);
		snprintf(pevt->dev_name, MAX_PATH_LEN, "/dev/%s", dev_name);
		return 1;
	}

    return 0;
}

static int read_and_parse_uevent_msg(int uevent_fd, struct timeval* timeout, TKL_STORAGE_EVENT_MSG *pevt)
{
    int parse_ret = 0;
    int select_ret = 0;
    ssize_t n;
    char* data;
    fd_set fds;
    int fd_size;
    struct iovec iov;
    struct sockaddr_nl snl;

    iov.iov_base = malloc(UEVENT_MSG_LEN + 2);
    iov.iov_len = UEVENT_MSG_LEN + 2;
    struct msghdr msg = {&snl, sizeof(snl), &iov, 1, NULL, 0, 0};
    data = (char*)msg.msg_iov->iov_base;

    do {
        FD_ZERO(&fds);
        FD_SET(uevent_fd, &fds);
        fd_size = uevent_fd + 1;

        select_ret = select(fd_size, &fds, NULL, NULL, timeout);

        if (select_ret == 0) {
            free(iov.iov_base);
            return -ETIMEDOUT;
        }

        if (select_ret < 0) {
            printf("%s: select failed errno:%d\n", __func__, -errno);
            free(iov.iov_base);
            return -errno;
        }

		if (FD_ISSET(uevent_fd, &fds)) {
            n = recvmsg(uevent_fd, &msg, 0);
            if (!n) {
                printf("%s End of file\n", __func__);
                continue;
            }
            if (n < 0) {
                printf("%s: recvmsg failed errno:%d\n", __func__, -errno);
                free(iov.iov_base);
                return -errno;
            }

            if ((snl.nl_groups != 1) || (snl.nl_pid != 0)) {
                /* ignoring non-kernel netlink multicast message */
                continue;
            }

            if (n >= UEVENT_MSG_LEN) {
                continue;
            }

            /* these '\0' for msg parse in hw_parse_XXX_event */
            data[n] = '\0';
            data[n + 1] = '\0';
            n = n + 2;

			parse_ret = parse_uevent_msg(data, pevt);
        }
    /* parse_ret=1 match. parse_ret=0 event not match, so continue.*/
    } while (parse_ret == 0);

    free(iov.iov_base);
    return 0;
}

#define MAX_MOUNTS_INFO_LEN     256
#define LINUX_SD_DEV_FILE       "/dev/mmcblk0"
#define LINUX_MOUNT_INFO_FILE   "/proc/mounts"

/* 查看mount分区中是否有只读状态，状态变为只读返回1，否则返回0 */
static int tkl_storage_get_mountrdinfo(TKL_STORAGE_EVENT_MSG *pevt)
{
	FILE *fp = NULL;
	INT8_T readonly_status = 0;
	CHAR_T s_mounts_info[MAX_MOUNTS_INFO_LEN];
	CHAR_T *blk_mount_name_start = NULL;
	CHAR_T *blk_mount_name = NULL;
	CHAR_T blk_mount_name_split[] = " ";

	/* 检测blk0设备是否存在 */
	if(access(LINUX_SD_DEV_FILE,F_OK)) {
		// printf("not find /dev/mmcblk0\n");
		return 0;
	}

	fp = fopen(LINUX_MOUNT_INFO_FILE, "rb");
	if(fp == NULL) {
		// printf("open /dev/mmcblk0 err\n");
		return 0;  
	}

	while (true) {
		memset(s_mounts_info,0,sizeof(s_mounts_info));
		if(fgets(s_mounts_info,MAX_MOUNTS_INFO_LEN,fp) == NULL) {
			break;
		}

		/* 遍历，以最后一行为准 */
		blk_mount_name_start = strstr(s_mounts_info, LINUX_SD_DEV_FILE);
		if ((NULL != blk_mount_name_start) && (NULL != strstr(blk_mount_name_start,"ro,"))) {
			memset(pevt->dev_name, 0, sizeof(pevt->dev_name));
			blk_mount_name = strtok(blk_mount_name_start, (const char *)blk_mount_name_split);
			if (blk_mount_name) {
				strncpy(pevt->dev_name, blk_mount_name, strlen(blk_mount_name));
			} else {
				strncpy(pevt->dev_name, LINUX_SD_DEV_FILE, strlen(LINUX_SD_DEV_FILE));
			}
			readonly_status = 1;
		} else {
			readonly_status = 0;
		}
	}

	fclose(fp);

	if (readonly_status) {
		pevt->type = EVT_FS_ERR;
		pevt->msg.i = 0;
	}

	return readonly_status;
}

/**
 * @brief loop storage event, eg. hotplug
 * will not return except an error occured.
 * 
 * @param event_cb callback handle storage event
 * @param user_ctx user private data, optional
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_loop_event(TKL_STORAGE_EVENT_CB event_cb, VOID *user_ctx)
{
	int ret;
	int uevent_fd;
	struct timeval timeout;
	TKL_STORAGE_EVENT_MSG evt;

	if (event_cb == NULL)
		return -1;

	ret = open_uevent_socket();
	if (ret < 0)
	{
		printf("%s %d open_uevent_socket fail!\n", __func__, __LINE__);
		return -1;
	}

	uevent_fd = ret;

	while (1) {
		memset(&evt, 0, sizeof(TKL_STORAGE_EVENT_MSG));
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		ret = read_and_parse_uevent_msg(uevent_fd, &timeout, &evt);
		if (ret < 0) {
			if  (ret == -ETIMEDOUT) {
				/* 查看mount状态是否变化 */
				if (tkl_storage_get_mountrdinfo(&evt)) {
					event_cb(&evt, user_ctx);
				}
			} else {
				printf("%s %d read_and_parse_uevent_msg ret fail\n", __func__, __LINE__);
				break;
			}
		} else {
			event_cb(&evt, user_ctx);
		}
	}

	close(uevent_fd);

	return ret;
}


/**
 * @brief check and repair a filesystem
 * 
 * @param blk_dev device node
 * @param fs_type filesystem type
 * @param parm fsck parame, eg. fsck.vfat -f /dev/sda1
 * @param flags reserved flags
 * @param cb progress callback, optional
 * @param user_ctx caller's private context, optional
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_fsck(CHAR_T *blk_dev, CHAR_T *fs_type, CHAR_T *parm, INT_T flags,
    TKL_STORAGE_PROGRESS_CB cb, VOID *user_ctx)
{
	int ret;
	char cmd[256] = {0};
	char buffer[128] = {0};
	int len = sizeof(buffer);

	if (fs_type == NULL || blk_dev == NULL)
		return OPRT_INVALID_PARM;

	if (parm) {
		snprintf(cmd, sizeof(cmd), "fsck.%s %s %s", fs_type, parm, blk_dev);
	} else {
		snprintf(cmd, sizeof(cmd), "fsck.%s %s", fs_type, blk_dev);
	}

	ret = excute_shell_cmd(cmd, buffer, &len);
	if (ret < 0)
		return OPRT_COM_ERROR;

	printf("len %d \n buf %s\n", len, buffer);

	if (cb)
		cb(100, user_ctx);

	return OPRT_OK;
}

#if 0
/* simple encapsulate of open/read/write/lseek/fcntl/close */

/**
 * @brief open file
 * 
 * @param pathname file to open
 * @param flags open flags, refer to posix defines
 * @param mode access prevelage, optional. mode_t
 * @return On success, return fd, on error, -1 returned.
 */
INT_T tkl_storage_open(CHAR_T *pathname, INT_T flags, UINT_T mode)
{
	return open(pathname, flags, mode);
}


/**
 * @brief read file
 * 
 * @param fd file desp
 * @param buf data to load in
 * @param count read count size_t
 * @return On success, the number of bytes read is returned,
 * On error, -1 is returned, and errno is set appropriately.
 * ssize_t
 */
LONG_T tkl_storage_read(INT_T fd, VOID *buf, UINT_T count)
{
	return read(fd, buf, count);
}


/**
 * @brief write file
 * 
 * @param fd file desp
 * @param buf data to load in
 * @param count read count size_t
 * @return On success, the number of bytes write is returned,
 * On error, -1 is returned, and errno is set appropriately.
 * ssize_t
 */
LONG_T tkl_storage_write(INT_T fd, const VOID *buf, UINT_T count)
{
	return write(fd, buf, count);
}


/**
 * @brief close file
 * 
 * @param fd file desp
 * @return returns zero on success.  On error, -1 is returned, and errno is set appropriately.
 */
INT_T tkl_storage_close(INT_T fd)
{
	return close(fd);
}


/**
 * @brief seek file
 * 
 * @param fd file desp
 * @param offset offset wanted to seek. off_t
 * @param whence flags eg. SEEK_SET SEEK_CUR SEEK_END etc.
 * @return returns the resulting offset location as measured in bytes from the beginning of the file.
 * On error, the value (off_t) -1 is returned and errno is set to indicate the error
 */
LONG_T tkl_storage_lseek(INT_T fd, LONG_T offset, INT_T whence)
{
	return lseek(fd, offset, whence);
}
#endif

/**
 * @brief scsi generic ioctl
 * 
 * @param fd file desp
 * @param request request
 * @param args args
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_ioctl(CHAR_T *dev_name, ULONG_T request, VOID *args)
{
    int fd=0;
    int ret=0;
    
    fd = open(dev_name, O_RDONLY);
    if(fd<0)
    {
        printf("open %s fail\n", dev_name);
        return OPRT_COM_ERROR;
    }

	ret = ioctl(fd, request, args);

    close(fd);

    return ret;
}


/* medium-specific interfaces */

/**
 * @brief check bad blocks TODO: 增加更多参数
 * 
 * @param dev_name device node wanted to check
 * @param cb callback to report bad block index
 * @param user_ctx user private context, optional
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET tkl_storage_check_badblocks(CHAR_T *dev_name, TKL_STORAGE_BADBLK_CB cb, VOID *user_ctx)
{
	int ret;
	char cmd[256] = {0};
	char buffer[128] = {0};
	int len = sizeof(buffer);

	if (dev_name == NULL)
		return OPRT_INVALID_PARM;

	snprintf(cmd, sizeof(cmd), "badblocks %s", dev_name);
	
	ret = excute_shell_cmd(cmd, buffer, &len);
	if (ret < 0)
			return OPRT_COM_ERROR;

	printf("len %d \n buf %s\n", len, buffer);

	if (cb)
		cb(100, user_ctx);

	return OPRT_OK;
}
