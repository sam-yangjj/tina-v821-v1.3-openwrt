#ifndef _TY_SDK_CALL_H
#define _TY_SDK_CALL_H

#include "tuya_tmm_control.h"
#include "tal_workq_service.h"
#include "tuya_ipc_media_adapter.h"
#include "tuya_ipc_download_file.h"
#include "tuya_tmm_link.h"


#ifdef __cplusplus
extern "C" {
#endif

/**双向视频呼叫功能初始化*/
OPERATE_RET TUYA_IPC_call_init();

/**主动呼叫app*/
OPERATE_RET TUYA_IPC_call_app();

/**挂断与app的通话*/
OPERATE_RET TUYA_IPC_hangup();

/**
 * 背景图下载的demo实现
*/
OPERATE_RET TUYA_IPC_download_background_demo();

#ifdef __cplusplus
}
#endif

#endif