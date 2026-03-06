
/**
* @file tkl_reboot.h
* @brief reboot tkl interface 
* @version 0.1
* @date 2020-11-09
*
* @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
*
*/

#ifndef __TKL_REBOOT_H__
#define __TKL_REBOOT_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief reboot system
*
* @param[in] en_sync: 1,sync 0,null
*
* @return VOID_T
*/
VOID_T tkl_reboot(int en_sync);

#ifdef __cplusplus
}
#endif

#endif
