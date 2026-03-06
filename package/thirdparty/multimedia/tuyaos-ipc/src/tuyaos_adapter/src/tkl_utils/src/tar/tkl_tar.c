/**
 * @file hal_tar.c
 * @brief tar hal interface 
 * @version 0.1
 * @date 2019-08-15
 *
 * @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_log.h"
#include "tkl_cmd.h"
#include "tkl_tar.h"

#ifndef TAG 
#define TAG "tkl_tar"
#endif

#define TAR_PACK_HEAD    "/bin/busybox tar czf"
#define TAR_UNPACK_HEAD  "/bin/busybox tar xf"
#define MAX_CMD_BUF 1024

OPERATE_RET tkl_tar_unpack(const char *pack_file,const char *unpack_path)
{
    int iret = OPRT_OK;
    char cmd_buf[MAX_CMD_BUF] = {0};

    if(pack_file == NULL) {
        LOGE(TAG,"invalid param\n");
        return OPRT_INVALID_PARM;	
    }

    memset(cmd_buf,0,sizeof(cmd_buf));

    if(unpack_path && strlen(unpack_path)) {
        snprintf(cmd_buf,MAX_CMD_BUF,"%s %s -C %s",TAR_UNPACK_HEAD,pack_file,unpack_path);
    }else {
        snprintf(cmd_buf,MAX_CMD_BUF,"%s %s",TAR_UNPACK_HEAD,pack_file);
    }

    iret = tkl_cmd_run(cmd_buf,NULL,NULL);

    return iret;
}

OPERATE_RET tkl_tar_pack(const char *pack_file,const char * files[], const size_t file_counts)
{
    int i;
    int n = 0;
    int iret = OPRT_OK;
    char cmd_buf[MAX_CMD_BUF] = {0};

    if(pack_file == NULL ||  files == NULL || file_counts == 0) {
        LOGE(TAG,"invalid param");
        return OPRT_INVALID_PARM;	
    }

    n += snprintf(cmd_buf+n,MAX_CMD_BUF-n,"%s %s",TAR_PACK_HEAD,pack_file); 

    for(i=0;i<file_counts;i++) {
        n += snprintf(cmd_buf+n,MAX_CMD_BUF-n," %s",files[i]);
    }
      
    iret = tkl_cmd_run(cmd_buf,NULL,NULL);

    return iret;
}

