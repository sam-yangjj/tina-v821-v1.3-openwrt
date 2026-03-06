
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include <tkl_storage.h>
//#include "mountlist.h"

/* Linked list of mounted file systems.  */
static struct mount_entry *mount_list;

void event_cb(TKL_STORAGE_EVENT_MSG *pevt, VOID *user_ctx)
{
	TKL_STORAGE_DEVICE_INFO_T list[MAX_DEVICE_NUM]={0};
	int ret,i;

	printf("recv_msg: %s %d %d %s.\n", pevt->bus_type, pevt->type, pevt->msg.i, pevt->dev_name);

	if(pevt->msg.i == 0)
		return;

	// ADD
	ret = tkl_storage_get_device_info(pevt->dev_name, list);
	printf("########### tkl_storage_get_device_info ret=%d.\n", ret);
	for (i = 0; i < 1; i++)
	{
        printf("solt:%d ", list[i].slot);
		printf("major:%d minor:%d ", list[i].major, list[i].minor);
        printf("bus_type:%s ", list[i].bus_type);
        printf("model:%s ", list[i].model);
        printf("dev_name:%s ", list[i].dev_name);
        printf("capacity:%llu ", list[i].capacity);
        printf("sect_size:%u ", list[i].sect_size);
        printf("part_table_type:%s ", list[i].part_table_type);
        printf("dev_id:%s \n", list[i].dev_id);
    }
#if 1
    //tkl_storage_get_part_list
	extern int list_device_partitions(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, int num);
	TKL_STORAGE_PART_INFO_T list1[MAX_PART_NUM]={0};
	ret = list_device_partitions(pevt->dev_name, list1, MAX_PART_NUM);
    printf("%s %d==============end of list_device_partitions===========\n", __func__, __LINE__);
	printf("ret %d\n", ret);
	if (ret <= 0)
		return;
	for (i = 0; i < ret; i++)
	{
		printf("dev_name:%s %lluKB %lluKB %lluKB part_id:%u \n", list1[i].dev_name, list1[i].start, list1[i].end, list1[i].size, list1[i].part_id);
	}

#endif
}

VOID event_pro(INT_T progress, VOID *user_ctx)
{
	printf("event_pro: %d \n", progress);
}

int main (int argc, char **argv)
{
	int ret;
	char path[64];
	TKL_STORAGE_DF_T info;
    int i;
	int partnum = 0;

	ret = tkl_storage_get_internal_dir(path);
	if (ret == 0)
		printf("1. path: %s\n", path);

	//extern int df_main(CHAR_T *mount_dir, TKL_STORAGE_DF_T *df);

	TKL_STORAGE_DF_T df;
	memset(&df, 0, sizeof(df));
	ret = tkl_storage_df(argv[2], &df);
	printf("ccc: %s %s %lld %lld %lld %s \n", df.dev_name, df.fs_type, df.size, df.used, df.avail, df.mounted_on);

#if 1
    //tkl_storage_get_device_list    
	TKL_STORAGE_DEVICE_INFO_T list[MAX_DEVICE_NUM]={0};
	extern int list_devs_in_proc_partititons(TKL_STORAGE_DEVICE_INFO_T *list, int num);
	ret = list_devs_in_proc_partititons(list, MAX_DEVICE_NUM);
    printf("%s %d==============end of list_devs_in_proc_partititons===========\n", __func__, __LINE__);
	for (i = 0; i < ret; i++)
	{
        printf("solt:%d ", list[i].slot);
		printf("major:%d minor:%d ", list[i].major, list[i].minor);
        printf("bus_type:%s ", list[i].bus_type);
        printf("model:%s ", list[i].model);
        printf("dev_name:%s ", list[i].dev_name);
        printf("capacity:%llu ", list[i].capacity);
        printf("sect_size:%u ", list[i].sect_size);
        printf("part_table_type:%s ", list[i].part_table_type);        
        printf("dev_id:%s \n", list[i].dev_id);
    }

    //tkl_storage_get_part_list
	extern int list_device_partitions(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, int num);
	TKL_STORAGE_PART_INFO_T list1[MAX_PART_NUM]={0};
	partnum = list_device_partitions(argv[1], list1, MAX_PART_NUM);
    printf("%s %d==============end of list_device_partitions===========\n", __func__, __LINE__);    
	printf("part num: %d\n", partnum);
	for (i = 0; i < partnum; i++) {
		printf("dev_name:%s %lluKB %lluKB %lluKB part_id:%u \n", list1[i].dev_name, list1[i].start, list1[i].end, list1[i].size, list1[i].part_id);
	}

	/* 删除所有分区 */
	printf("del all part\n");
	for (i = 0; i < partnum; i++) {
		ret = tkl_storage_make_part(argv[1], "p", "linux", PART_DEL, list1[i].part_id, 100, 1000);
		printf("del part %d ret %d\n", list1[i].part_id, ret);
	}

	/* 查询分区 */
	memset(list1, 0, sizeof(list1));
	partnum = list_device_partitions(argv[1], list1, MAX_PART_NUM);
    printf("%s %d==============end of list_device_partitions===========\n", __func__, __LINE__);    
	printf("part num: %d\n", partnum);
	for (i = 0; i < partnum; i++) {
		printf("dev_name:%s %lluKB %lluKB %lluKB part_id:%u \n", list1[i].dev_name, list1[i].start, list1[i].end, list1[i].size, list1[i].part_id);
	}

	/* 创建最大分区 */
	ret = tkl_storage_make_part(argv[1], "p", "linux", PART_ADD, 1, -1, -1);
	printf("make one part:%d\n", ret);
	
	/* 查询分区 */
	memset(list1, 0, sizeof(list1));
	partnum = list_device_partitions(argv[1], list1, MAX_PART_NUM);
    printf("%s %d==============end of list_device_partitions===========\n", __func__, __LINE__);    
	printf("part num: %d\n", partnum);
	for (i = 0; i < partnum; i++) {
		printf("dev_name:%s %lluKB %lluKB %lluKB part_id:%u \n", list1[i].dev_name, list1[i].start, list1[i].end, list1[i].size, list1[i].part_id);
	}

	/* 格式化分区 */
	printf("mkfs start\n");
	tkl_storage_mkfs("vfat", "/dev/mmcblk0p1", NULL, event_pro, NULL);
	printf("mkfs success\n");

	printf("mount start\n");
    tkl_storage_mount("/dev/mmcblk0p1", "/mnt/mmcblk0p1", "vfat", 0, 0);
	printf("mount success\n");
#endif

    
    //tkl_storage_mkfs("vfat", "/dev/sdb1", "-F 32 -n \"mkfs.boot\" ", event_pro, NULL);

//    tkl_storage_fsck("/dev/sdb1", "vfat", NULL, 0, event_pro, NULL);
    
	extern OPERATE_RET tkl_storage_loop_event(TKL_STORAGE_EVENT_CB event_cb, VOID *user_ctx);

	ret = tkl_storage_loop_event(event_cb, NULL);


	return 0;
}
