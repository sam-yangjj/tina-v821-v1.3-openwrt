/*
* @file tkl_file.h
* @brief file tkl interface 
* @version 0.1
* @date 2020-11-09
*
* @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
*
*/

#ifndef __TKL_FILE_H__
#define __TKL_FILE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief file read   
* @param[in] path: file path
* @param[out] buf: file read buffer size
* @param[in] size: file read buffer size
* @return -1 fail, >=0 read bytes 
*/
int tkl_file_read(char *path, void *buf, int size);

/**
* @brief file write   
* @param[in] path: file path
* @param[in] buf: file read buffer size
* @param[in] size: file read buffer size
* @return -1 fail, >= 0 write bytes
*/
int tkl_file_write(char *path, void *buf, int size);


/**
* @brief file is exixt   
* @param[in] path: file path
* @return true(1),exists;false(0),not exists 
*/
int tkl_file_exists(char *path);

/**
* @brief get file size  
* @param[in] path: file path
* @param[out] size: file  size
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_file_size(char *path, int *size);


#ifdef __cplusplus
}
#endif

#endif
