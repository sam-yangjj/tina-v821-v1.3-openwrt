/**
 * @file tuya_tmm_control.h
 * @author baijue.huang@tuya.com
 * @brief tuya tmm control
 * @version 0.1
 * @date 2022-8-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __TUYA_TMM_CONTROL_H__
#define __TUYA_TMM_CONTROL_H__

/***********************************************************************
 ** INCLUDE                                                           **
 **********************************************************************/
#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ** CONSTANT ( MACRO AND ENUM )                                       **
 **********************************************************************/

/***********************************************************************
 ** STRUCT                                                            **
 **********************************************************************/

typedef enum
{
    TUYA_TMM_CONTROL_EVT_RINGING,                    //对方已响铃,但未接听
    TUYA_TMM_CONTROL_EVT_INCOMING,                   //有来电
    TUYA_TMM_CONTROL_EVT_ACCEPTED,                   //对方接通
    TUYA_TMM_CONTROL_EVT_UNANSWERED,                 //对方未接通
    TUYA_TMM_CONTROL_EVT_REJECT,                     //对方拒接
    TUYA_TMM_CONTROL_EVT_HANGUP,                     //对方挂断
    TUYA_TMM_CONTROL_EVT_BUSY,                       //对方正忙
    TUYA_TMM_CONTROL_EVT_CANCEL,                     //未接通，取消通话
    TUYA_TMM_CONTROL_EVT_STOP,                       //通话中断
    TUYA_TMM_CONTROL_EVT_ACCEPTED_BY_OTHERS,         //被其他端接听
    TUYA_TMM_CONTROL_EVT_ERROR,                      //错误
} TUYA_TMM_CONTROL_EVT_E;

typedef enum
{
    TUYA_TMM_CONTROL_STREAM_TYPE_AUDIO,              //音频通话
    TUYA_TMM_CONTROL_STREAM_TYPE_VIDEO,              //视频通话
    TUYA_TMM_CONTROL_STREAM_TYPE_AV,                 //音视频
} TUYA_TMM_CONTROL_STREAM_TYPE_E;

typedef enum
{
    TUYA_TMM_CONTROL_TARGET_TYPE_APP,
    TUYA_TMM_CONTROL_TARGET_TYPE_DEV
}TUYA_TMM_CONTROL_TARGET_TYPE_E;
typedef struct
{
    TUYA_TMM_CONTROL_EVT_E          event;                 //呼叫事件
	TUYA_TMM_CONTROL_STREAM_TYPE_E  stream_type;           //通话流类型, call事件时关注
    TUYA_TMM_CONTROL_TARGET_TYPE_E  target_type;           //对方类型，app或者设备
    CHAR_T                          target_id[64];         //对方设备id
    CHAR_T                          target_localkey[64];   //对方设备的local key
    CHAR_T                          target_name[64];       //对方设备的名称
} TUYA_TMM_CONTROL_INFO_S;

typedef OPERATE_RET (*TUYA_TMM_CONTROL_EVT_CB)(TUYA_TMM_CONTROL_INFO_S* pinfo, VOID* priv_data);

/***********************************************************************
 ** VARIABLE                                                          **
 **********************************************************************/

/***********************************************************************
 ** FUNCTON                                                           **
 **********************************************************************/

/*****************************************************************************
 * @brief      tuya tmm control 初始化.
 *
 * @param      cb: 事件回调函数
 * @param      priv_data: 用户数据，通过callback返回
 * @param      call_timeout_s: 设定呼叫超时时间,单位秒， <= 0 使用默认30s
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_init(TUYA_TMM_CONTROL_EVT_CB cb, VOID* priv_data, INT_T call_timeout_s);

/*****************************************************************************
 * @brief      tuya tmm control 注销.
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_deinit(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 主叫
 *
 * @param      target_device: 目标设备
 * @param      category: 本设备所属品类，可以为""
 * @param      stream_type: 通话流类型
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_call(CHAR_T *target_device, CHAR_T *category, TUYA_TMM_CONTROL_STREAM_TYPE_E stream_type);

/*****************************************************************************
 * @brief      tuya tmm control 接通
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_answer(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 来电拒接
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_reject(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 取消呼叫
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_cancel(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 挂断
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_hangup(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 异常停止
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_stop(VOID);

/*****************************************************************************
 * @brief      tuya tmm control 设备忙
 *
 * @return
 *     - 0， success
 *     - 非0 error
*****************************************************************************/
OPERATE_RET tuya_tmm_control_busy(VOID);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_TMM_CONTROL_H__ */
