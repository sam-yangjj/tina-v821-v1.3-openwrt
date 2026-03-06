/*
* Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : isp_version.h
* Description :
* History :
* Author  : zhaowei <zhaowei@allwinnertech.com>
* Date    : 2018/02/08
*
*/

#ifndef _ISP_VERSION_H_
#define _ISP_VERSION_H_

#include "include/isp_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISP_VERSION 601
#define ISP_SMALL_VERSION 100
#define REPO_TAG "isp-500-520-v2.00"
#define REPO_BRANCH "libisp-dev"
#define REPO_COMMIT "c3bfa4e23fb037123e65a04eaaa39f7876cfe1a6"
#define REPO_DATE "Mon Apr 10 10:37:15 2023 +0800"
#define RELEASE_AUTHOR "<zhenghanjie@allwinnertech.com>"

static inline void isp_version_info(void)
{
	ISP_PRINT(">>>>>>>>>>>>>>>>>>>> ISP VERSION INFO <<<<<<<<<<<<<<<<<<<\n"
		"IPCORE: ISP%d\n"
		"branch: %s\n"
		"commit: %s\n"
		"date  : %s\n"
		"author: %s\n"
		"--------------------------------------------------------\n\n",
		ISP_VERSION, REPO_BRANCH, REPO_COMMIT, REPO_DATE, RELEASE_AUTHOR);
}

#ifdef __cplusplus
}
#endif

#endif

