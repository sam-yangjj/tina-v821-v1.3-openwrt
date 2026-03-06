/**
* @file hal_tar.h
* @brief tar hal interface 
* @version 0.1
* @date 2020-11-09
*
* @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
*
*/

#ifndef __TKL_TAR_H__
#define __TKL_TAR_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief unpack tar packet to path
*
* @param[in] pack_file: tar packet filename
* @param[in] unpack_path: unpack tar path
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_tar_unpack(const char *pack_file,const char *unpack_path);

/**
* @brief pack tar packet to path
*
* @param[in] pack_file: tar packet filename
* @param[in] files: pack files 
* @param[in] file_counts: pack file counts
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

OPERATE_RET tkl_tar_pack(const char *pack_file,const char * files[], const size_t file_counts);

#ifdef __cplusplus
}
#endif

#endif
