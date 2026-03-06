/*********************************************************************************
  *Copyright(C),2015-2020, 
  *TUYA 
  *FileName: tuya_ipc_dp_.c
  *
  * File Description：
  * 1. API implementation of DP point
  *
  * This file code is the basic code, users don't care it
  * Please do not modify any contents of this file at will. 
  * Please contact the Product Manager if you need to modify it.
  *
**********************************************************************************/
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_config.h"
#include "ty_dp_define.h"
#include "ty_sdk_common.h"
#include "utilities/uni_log.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
 
#define IPC_CPU_UPGRADE_FILE    "/tmp/rootfs.squashfs" //File with path to download file during OTA
#define IPC_MCU_UPGRADE_FILE    "/tmp/rtos.bin"

/* OTA */
//Callback after downloading OTA files
OPERATE_RET __IPC_APP_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    FILE *p_upgrade_fd = (FILE *)pri_data;
    fclose(p_upgrade_fd);

    PR_DEBUG("Upgrade Finish");
    PR_DEBUG("download_result:%d fw_url:%s", download_result, fw->fw_url);

    if(download_result == 0)
    {
        /* The developer needs to implement the operation of OTA upgrade,
        when the OTA file has been downloaded successfully to the specified path. [ p_mgr_info->upgrade_file_path ]*/
        const char *cmd = NULL;
        if (fw->tp == 0){
            cmd = "/etc/fire.sh &";
        }
        else if (fw->tp == 10){
            cmd = "/etc/fire_mcu.sh &";
        }
        if (cmd != NULL){
            PR_DEBUG("run shell cmd :%s \r\n", cmd);
            system(cmd);
        }
    }
    //TODO
    //reboot system
    return OPRT_OK;
}

//To collect OTA files in fragments and write them to local files
OPERATE_RET __IPC_APP_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,IN CONST UINT_T offset,
                             IN CONST BYTE_T *data,IN CONST UINT_T len,OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data,ch:%d", fw->tp);
    PR_DEBUG("total_len:%d  fw_url:%s", total_len, fw->fw_url);
    PR_DEBUG("Offset:%d Len:%d", offset, len);

    //report UPGRADE process, NOT only download percent, consider flash-write time
    //APP will report overtime fail, if uprgade process is not updated within 60 seconds
    int download_percent = (offset * 100) / (total_len+1);
    int report_percent = download_percent/2; // as an example, download 100% = 50%  upgrade work finished
    if (fw->tp == 0){
        tuya_ipc_upgrade_progress_report(report_percent);
    }
    else if (fw->tp == 10){
        tuya_ipc_upgrade_progress_report_by_type(report_percent, 10);
    }
    //APP will report "uprage success" after reboot and new FW version is reported inside SDK automaticlly
     FILE *p_upgrade_fd = (FILE *)pri_data;
    fwrite(data, 1, len, p_upgrade_fd);

    return OPRT_OK;
}

INT_T TUYA_IPC_Upgrade_Inform_cb(IN CONST FW_UG_S *fw)
{
    PR_INFO("Rev Upgrade Info");
    PR_INFO("fw->tp:%d", fw->tp);
    PR_INFO("fw->fw_url:%s", fw->fw_url);
    PR_INFO("fw->sw_ver:%s", fw->sw_ver);
    PR_INFO("fw->file_size:%u", fw->file_size);
    
    FILE *p_upgrade_fd = NULL;
    if (fw->tp == 0){
        p_upgrade_fd = fopen(IPC_CPU_UPGRADE_FILE, "w+b");
        tuya_ipc_upgrade_sdk(fw, __IPC_APP_get_file_data_cb, __IPC_APP_upgrade_notify_cb, p_upgrade_fd);
    }
    else if (fw->tp == 10){
        p_upgrade_fd = fopen(IPC_MCU_UPGRADE_FILE, "w+b");
    }
    else{
        return -1;
    }
    tuya_ipc_upgrade_sdk(fw, __IPC_APP_get_file_data_cb, __IPC_APP_upgrade_notify_cb, p_upgrade_fd);
    return 0;
}
