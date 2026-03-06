#ifndef __TUYA_IPC_AI_STATION_H_
#define __TUYA_IPC_AI_STATION_H_

#include "tuya_ai_client.h"
#include "tuya_ipc_media.h"

typedef enum {
    E_AI_CMD_CHAT_START   = 1,
    E_AI_CMD_CHAT_FIN     = 2,
    E_AI_CMD_SERVER_VAD   = 3,
    E_AI_CMD_MOVE_AROUND  = 4,
    E_AI_CMD_MOVE_LEFT    = 5,
    E_AI_CMD_MOVE_RIGHT   = 6,
    E_AI_CMD_MOVE_UP      = 7,
    E_AI_CMD_MOVE_DOWN    = 8,
    E_AI_CMD_ASR_TEXT     = 9,    //text of automatic speech recognition
    E_AI_CMD_NLG_TEXT     = 0x0a, //text of natural language generation
    E_AI_CMD_EMOTION      = 0x0b, //emotion recognition
    E_AI_CMD_SESSION_CREATE = 0x0c, //the session has been successfully created, and you can now proceed with the conversation
    E_AI_CMD_SESSION_CLOSE  = 0x0d, //the session has been closed, and the conversation has ended.
}TUYA_AI_CMD_TYPE_E;

typedef struct {
    TUYA_CODEC_ID_E codec_type;
    UINT_T sample_rate; // unit: Hz
    TUYA_AUDIO_CHANNEL_E channels;
    USHORT_T bit_depth;
} TUYA_AI_AUDIO_ATTR_T;

/**
 * @brief         callback of ai cmd
 * @param[in]     cmd. specifically refer to TUYA_AI_CMD_TYPE_E
 * @param[in]     args. additional data for cmd
 * @return        VOID
 */
typedef VOID (*TUYA_IPC_AI_CMD_CB)(TUYA_AI_CMD_TYPE_E cmd, CONST PVOID_T args);

/**
 * @brief         callback of audio play
 * @param[in]     audio_data. if the AI voice ends, audio_data will be equal to NULL
 * @param[in]     len. len of audio data. if the AI voice ends, len will be equal to 0
 * @return        VOID
 */
typedef VOID (*TUYA_IPC_AI_AUDIO_PLAY_CB)(CONST CHAR_T *audio_data, INT_T len);


/**
 * @brief init ai station

 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_init();

/**
 * @brief init ai chat
 * 
 * @param[in] event_cb: ai event cb
 * @param[in] audio_cb: audio play cb
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_chat_init(TUYA_IPC_AI_CMD_CB event_cb, TUYA_IPC_AI_AUDIO_PLAY_CB audio_cb);

/**
 * @brief uninit ai chat
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_chat_uninit();

/**
 * @brief start a conversation
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_start_act();

/**
 * @brief stop a conversation
 * 
 * @param VOID
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_stop_act();

/**
 * @brief break a conversation
 *
 * @param VOID
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_ai_station_break_act();

/**
 * @brief Set the audio format of the cloud replies to MP3. This interface must be called before tuya_ipc_ai_station_init
 * 
 * @param[in] enable_mp3: TRUE indicates MP3 format. FALSE indicates the default cloud format.
 * 
 * @return void
 */
VOID_T tuya_ipc_ai_station_set_audio_format_to_mp3(BOOL_T enable_mp3);

#endif
