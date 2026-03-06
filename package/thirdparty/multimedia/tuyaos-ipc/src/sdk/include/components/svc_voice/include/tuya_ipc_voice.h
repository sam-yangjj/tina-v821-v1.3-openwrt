/*********************************************************************************
  *Copyright(C),2020, 涂鸦科技 www.tuya.comm
  *FileName:    tuya_ipc_voice.h
**********************************************************************************/

#ifndef __TUYA_IPC_VOICE_H__
#define __TUYA_IPC_VOICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_ipc_media.h"

#define VOICE_NAME_LEN                64
#define VOICE_FILENAME_LEN            128

typedef struct
{
    BOOL_T valid;
    BOOL_T isPreset;
    CHAR_T name[VOICE_NAME_LEN+1];
    CHAR_T filename[VOICE_FILENAME_LEN+1];
    INT_T  id;
    UINT_T size;
    UINT_T duration;
} VOICE_LIST_S;

/**
 * \brief callback to process downloading voice data
 * \param[in] data is voice data
 * \param[in] len is voice data length
 * \param[in] pri_data is pointer set in tuya_ipc_voice_download_start api
 * \return OPERATE_RET
 */
typedef OPERATE_RET (*GET_VOICE_DATA_CB)(IN BYTE_T *data, IN UINT_T len, IN PVOID_T pri_data);

/**
 * \brief callback when download finished
 * \param[in] download_result > 0 :  download success
 *            download_result == 0 : download is stopped by tuya_ipc_voice_download_stop api
 *            dowaload_result < 0 :  download fail
 * \param[in] pri_data is pointer set in tuya_ipc_voice_download_start api
 * \return VOID
 */
typedef VOID (*VOICE_NOTIFY_CB)(IN INT_T download_result, IN PVOID_T pri_data);

/**
 * \fn OPERATE_RET tuya_ipc_voice_init  
 * \brief initialize voice
 * \param[in] code is voice code
 * \param[in] max_voice_num is max voice number in voice list
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_voice_init(IN CHAR_T *code, IN UINT_T max_voice_num);

/**
 * \fn OPERATE_RET tuya_ipc_voice_download_start
 * \brief start downloading voice
 * \param[in] code is voice code
 * \param[in] id is voice id for each voice file
 * \param[in] get_voice_cb is voice file downloading callback function
 * \param[in] voice_notify_cb is voice file download finished callback function
 * \param[in] priv_data is pointer passed to get_voice_cb and voice_notify_cb callback function
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_voice_download_start(IN CHAR_T *code, IN INT_T id, IN GET_VOICE_DATA_CB get_voice_cb, IN VOICE_NOTIFY_CB voice_notify_cb, IN PVOID_T pri_data);

/**
 * \fn VOID tuya_ipc_download_stop  
 * \brief stop downloading voice
 * \return VOID
 */
VOID tuya_ipc_voice_download_stop(VOID);

/**
 * \fn OPERATE_RET tuya_ipc_get_voice_list  
 * \brief get voice list
 * \param[inout] voice_list is the voice list, voice list is array which has max_voice_num elements
 * \param[in] upgrade, update the latest voice list immediately if upgrade is TRUE
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_get_voice_list(INOUT VOICE_LIST_S *voice_list, BOOL_T upgrade);

/**
 * \fn VOID tuya_ipc_voice_uninit  
 * \brief uninitialize voice
 * \return VOID
 */
VOID tuya_ipc_voice_uninit(VOID);

/**
 * \fn OPERATE_RET tuya_ipc_get_voice_code  
 * \brief get voice code
 * \param[inout] voice_code, is the voice code
 * \param[inout] code_len, voice code string len
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_get_voice_code(INOUT CHAR_T *voice_code, INOUT INT_T* code_len);

#ifdef __cplusplus
}
#endif

#endif
