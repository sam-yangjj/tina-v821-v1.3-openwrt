/**
* @file tkl_tar.h
* @brief tar tkl interface 
* @version 0.1
* @date 2020-11-09
*
* @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
*
*/

#ifndef __TKL_PASSWD_H__
#define __TKL_PASSWD_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*encryption algorithm*/
typedef enum {
    E_TKL_PWD_ALGO_DES = 0, /*des*/
    E_TKL_PWD_ALGO_MD5,     /*md5 */
    E_TKL_PWD_ALGO_SHA256,  /*sha256 */
    E_TKL_PWD_ALGO_SHA512   /*sha512*/
} TKL_PWD_ALGO_E;


/**
* @brief passwd update for exsixt user, root user can change all user's passwd,common user can change own passwd 
*
* @param[in] algo:  encryption algorithm,see Also TKL_PWD_ALGO_E
* @param[in] user:  user name 
* @param[in] passed: new passwd
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

OPERATE_RET tkl_passwd_update(TKL_PWD_ALGO_E algo,const char *user, const char *passwd);

#ifdef __cplusplus
}
#endif

#endif
