#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#include "tkl_storage.h"
#include "tal_system.h"
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_media_stream_event.h"
#include "ty_sdk_common.h"

/************************
Description: Using the TUYA SD card storage and playback function, 
the developers need to implement the relevant interface.
Note：Interface names cannot be changed, and these interfaces are declared and used in SDK.
This demo file gives the management implementation of SD card operation for typical embedded devices in Linux system.
Developers can modify it according to their practical application.

VOID tuya_ipc_sd_format(VOID);
VOID tuya_ipc_sd_remount(VOID);
SD_STATUS_E tuya_ipc_sd_get_status(VOID);
VOID tuya_ipc_sd_get_capacity(UINT_T *p_total, UINT_T *p_used, UINT_T *p_free);
CHAR_T *tuya_ipc_get_sd_mount_path(VOID);

************************/

/*SD CARD info*/
#define MAX_MMC_NAME_LEN        16
#define MAX_MOUNTS_INFO_LEN     1024
#define LINUX_SD_DEV_FILE       "/dev/mmcblk0"
#define LINUX_MOUNT_INFO_FILE   "/proc/mounts"
#define FORMAT_CMD              "mkfs.vfat"

STATIC CHAR_T s_mounts_info[MAX_MOUNTS_INFO_LEN];
STATIC CHAR_T s_mmcblk_name[MAX_MMC_NAME_LEN] = {0};
STATIC CHAR_T s_sd_mount_path[SS_BASE_PATH_LEN] = {0};


//Implementation Interface of Formatting Operation
VOID tuya_ipc_sd_format(VOID)
{
    CHAR_T format_cmd[256] = {0};
    char buffer[512] = {0};
    PR_DEBUG("sd format begin\n");
#if defined(OPERATING_SYSTEM) && (OPERATING_SYSTEM == 100)//linux
    snprintf(format_cmd,256,"umount %s;%s %s;mkdir -p /mnt/sdcard;mount -t auto %s /mnt/sdcard;",s_mmcblk_name,FORMAT_CMD,s_mmcblk_name,s_mmcblk_name);
    PR_DEBUG("execute: %s\n",format_cmd);
    FILE *pp = popen(format_cmd, "r");
    if(NULL != pp)
    {
        fgets(buffer,sizeof(buffer),pp);
        PR_INFO("%s\n",buffer);
        pclose(pp);
    }
    else
    {
        PR_DEBUG("format_sd_card failed\n");
    }
#else // non-linux
    OPERATE_RET ret = 0;

    ret = tkl_storage_umount("/mnt/sdcard", 0);
    if (0 != ret) {
        PR_ERR("tkl_storage_umount failed\r\n", ret);
        // return;
    }
    if (s_mmcblk_name[0] == '0') {
        PR_INFO("user never call tuya_ipc_sd_get_status but format sdcard, rarely happen\r\n", ret);
        if (SD_STATUS_NOT_EXIST == tuya_ipc_sd_get_status()) {
            PR_DEBUG("sd not exist\r\n");
            return;
    }
}

    ret = tkl_storage_mkfs("elm", s_mmcblk_name, "mkfs.vfat", NULL, NULL);
    if (ret != 0) {
        PR_ERR("tkl_storage_mkfs failed\r\n");
        return;
}

    ret = tkl_storage_mount(s_mmcblk_name, "/mnt/sdcard", "elm", 0, 0);
    if (0 != ret) {
        PR_ERR("tkl_storage_mount %s on /mnt/sdcard fs elm failed %d\r\n",s_mmcblk_name, ret);
        return;
    }
#endif
    PR_DEBUG("sd format end\n");
}

//Implementation Interface for Remounting
VOID tuya_ipc_sd_remount(VOID)
{
    CHAR_T format_cmd[128] = {0};
    char buffer[512] = {0};
    SD_STATUS_E status = SD_STATUS_UNKNOWN;

    status = tuya_ipc_sd_get_status();
    if(SD_STATUS_NORMAL == status)
    {
        PR_DEBUG("sd don't need to remount!\n");
        return;
    }
    PR_DEBUG("remount_sd_card ..... \n");
#if defined(OPERATING_SYSTEM) && (OPERATING_SYSTEM == 100)//linux
    snprintf(format_cmd,128,"umount %s;sleep 1;mount -t auto %s /mnt/sdcard;",s_mmcblk_name,s_mmcblk_name);
    FILE *pp = popen(format_cmd, "r");
    if(NULL != pp)
    {
        fgets(buffer,sizeof(buffer),pp);
        PR_INFO("%s\n",buffer);
        pclose(pp);
    }
    else
    {
        PR_INFO("remount_sd_card failed\n");
    }
#else
    OPERATE_RET ret = 0;
    ret = tkl_storage_umount("/mnt/sdcard", 0);
    if (0 != ret) {
        PR_ERR("tkl_storage_mkf failed\r\n", ret);
        return ;
}
    ret = tkl_storage_mount(s_mmcblk_name, "/mnt/sdcard", "elm", 0, 0);
    if (0 != ret) {
        PR_ERR("tkl_storage_mount %s failed %d\r\n", s_mmcblk_name, ret);
    }
#endif
}

STREAM_STORAGE_WRITE_MODE_E tuya_ipc_sd_get_mode_config(VOID)
{
    BOOL_T sd_on = IPC_APP_get_sd_record_onoff();
    if(sd_on)
    {
        UINT_T sd_mode = IPC_APP_get_sd_record_mode();
        if(0 == sd_mode)
        {
            return SS_WRITE_MODE_EVENT;
        }
        else
        {
            return SS_WRITE_MODE_ALL;
        }
    }
    else
    {
        return SS_WRITE_MODE_NONE;
    }
}

//Implementation Interface for Obtaining SD Card Status
SD_STATUS_E tuya_ipc_sd_get_status(VOID)
{

    // UserTodo
    FILE *fp = fopen(LINUX_SD_DEV_FILE, "rb");
    if(!fp)
    {
        return SD_STATUS_NOT_EXIST;
    }
    fclose(fp);

    strcpy(s_mmcblk_name,LINUX_SD_DEV_FILE);
    if(0 == access(LINUX_SD_DEV_FILE"p1",F_OK)) //Default node name information
    {
        strcat(s_mmcblk_name,"p1");
    }
    
    fp = fopen(LINUX_MOUNT_INFO_FILE, "rb");
    if(fp)
    {
        memset(s_mounts_info,0,sizeof(s_mounts_info));
        fread(s_mounts_info,1,MAX_MOUNTS_INFO_LEN,fp);
        fclose(fp);
        CHAR_T *mmcblk_name_start = strstr(s_mounts_info,"/dev/mmcblk"); //Confirm the final node name in Mount information
        CHAR_T *mmcblk_name_end = strstr(s_mounts_info," /mnt/sdcard");
        if(mmcblk_name_start && mmcblk_name_end)
        {
            int mmcblk_name_len = mmcblk_name_end-mmcblk_name_start;
            if(mmcblk_name_len >= MAX_MMC_NAME_LEN)
            {
                 return SD_STATUS_ABNORMAL;
            }
            strncpy(s_mmcblk_name, mmcblk_name_start, mmcblk_name_len);
            s_mmcblk_name[mmcblk_name_len] = '\0';
        }
        //There are device nodes but no mount information. Generally, the card format is incorrect and report abnormal.
        else 
        {
            return SD_STATUS_ABNORMAL;
        } 
        //If the mount information of the SD card is not at the end and there is a ro mount behind it, there will be a problem.
        if(NULL != strstr(mmcblk_name_start,"ro,")) 
        {
            return SD_STATUS_ABNORMAL;
        }
        if(NULL == strstr(mmcblk_name_start,"vfat"))
        {
            return SD_STATUS_ABNORMAL;
        }
        if (access(s_mmcblk_name,0))
        {
            return SD_STATUS_ABNORMAL;
        }

        return SD_STATUS_NORMAL;
    }
    else
    {
        return SD_STATUS_UNKNOWN;
    }
}

// get SD card capacity acquisition interface by mount path (/mnt/sdcard e.g.), unit: KB
VOID tuya_ipc_sd_get_capacity_by_mount_path(CONST CHAR_T* mount_path, UINT_T *p_total, UINT_T *p_used, UINT_T *p_free)
{
    // *p_total = 1024*1024;
    // *p_used = 1024*20;
    // *p_free =  *p_total  - *p_used;
    // return ;
#if defined(OPERATING_SYSTEM) && (OPERATING_SYSTEM == 100)//linux
    struct statfs sd_fs;
    if (statfs(mount_path, &sd_fs) != 0) {  
        PR_ERR("statfs failed %s!/n", mount_path);
        return;
    }

    *p_total = (UINT_T)(((UINT64_T)sd_fs.f_blocks * (UINT64_T)sd_fs.f_bsize) >> 10);
    *p_used = (UINT_T)((((UINT64_T)sd_fs.f_blocks - (UINT64_T)sd_fs.f_bfree) * (UINT64_T)sd_fs.f_bsize) >> 10);
    *p_free = (UINT_T)(((UINT64_T)sd_fs.f_bavail * (UINT64_T)sd_fs.f_bsize) >> 10);
#else //non-linux
    OPERATE_RET ret = 0;
    TKL_STORAGE_DF_T sd_df = {0};
    ret = tkl_storage_df(mount_path, &sd_df);
    if (0 != ret) {
        PR_ERR("tkl_storage_df failed %s!/n", mount_path);
        return;
    }

    *p_total = (UINT_T)sd_df.size;
    *p_used = (UINT_T)sd_df.used;
    *p_free = (UINT_T)sd_df.avail;
#endif
    // PR_DEBUG("sd capacity: total: %d KB, used %d KB, free %d KB\n",*p_total,*p_used,*p_free);
    return;
}

//get the path of mounting sdcard
CHAR_T *tuya_ipc_get_sd_mount_path(VOID)
{
    PR_DEBUG("s_sd_mount_path = %s", s_sd_mount_path);
    return s_sd_mount_path;
}

INT_T tuya_ipc_sd_status_upload(INT_T status)
{
	IPC_APP_report_sd_status_changed(status);
	return 0;
}

int tuya_ipc_ss_media_cb(int cameraId, IPC_MEDIA_INFO_T* param)
{
    if (cameraId < 0 || cameraId > 1) {
        return -1;
    }

    DEVICE_MEDIA_INFO_T media_info = {0};
    tuya_ipc_media_adapter_get_media_info(0, cameraId, &media_info);
    memcpy(param, &media_info.av_encode_info, sizeof(IPC_MEDIA_INFO_T));

    return 0;
}
