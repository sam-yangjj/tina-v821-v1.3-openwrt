#ifndef __TUYA_IPC_AOV_BUFFER_H__
#define __TUYA_IPC_AOV_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_ipc_media.h"

/**
 * @enum AOV_BUFFER_OPEN_TYPE_E
 *
 * @brief the aov buffer open mode
 */
typedef enum
{
    E_AOV_BUF_READ,
    E_AOV_BUF_WRITE,
} AOV_BUF_OPEN_TYPE_E;

#pragma pack(push, 4)
/**
 * @struct AOV_BUFFER_NODE_T
 *
 * @brief  aov buffer node info
 */
typedef struct
{
    UINT_T             magic_number;    ///<magic number
    MEDIA_FRAME_TYPE_E type;            ///<media frame type
    UCHAR_T           *raw_data;        ///<address of the media data
    UINT_T             size;            ///<data size
    UINT64_T           pts;             ///<time stamp in us
    UINT64_T           timestamp;       ///<time stamp in ms
    UINT_T             seq_no;          ///<sequence number
    BOOL_T             is_last_frame;   ///<is last frame for aov file
} AOV_BUFFER_NODE_T;
#pragma pack(pop)

/**
 * @struct AOV_BUFFER_INIT_PARAM_T
 *
 * @brief aov buffer init parameters, should be set as actual
 */
typedef struct
{
    UINT_T buffer_size;                 ///<buffer size for aov buffer
    BOOL_T correct_last_frame_enable;   ///<AOV last frame time calibration strategy control
} AOV_BUFFER_INIT_PARAM_T;

/**
 * @brief user handle which returned by open
 */
typedef void* AOV_BUFFER_USER_HANDLE_T;

/**
 * @brief initialize one aov buffer for one channel(one device)
 *
 * @param[in] device    device number
 * @param[in] channel   channel number in device
 * @param[in] stream    stream number in channel
 * @param[in] pparam    initialize parameter
 *
 * @return error code
 * - OPRT_OK  init success
 * - Others   init failed
 */
OPERATE_RET tuya_ipc_aov_buffer_init(INT_T device, INT_T channel, IPC_STREAM_E stream, AOV_BUFFER_INIT_PARAM_T *pparam);

/**
 * @brief open a new session for read/write
 *
 * @param[in] device    device number
 * @param[in] channel   channel number in device
 * @param[in] stream    stream number in channel
 * @param[in] open_type open type
 *
 * @return    user handle
 */
AOV_BUFFER_USER_HANDLE_T tuya_ipc_aov_buffer_open(INT_T device, INT_T channel, IPC_STREAM_E stream, AOV_BUF_OPEN_TYPE_E open_type);

/**
 * @brief close session
 *
 * @warning open/close should be called in pair like file
 * @param[in] handle   user handle returned by open
 *
 * @return error code
 * - OPRT_OK  success
 * - Others   failed
 */
OPERATE_RET tuya_ipc_aov_buffer_close(AOV_BUFFER_USER_HANDLE_T handle);

/**
 * @brief append new frame into aov buffer with custom timestamp
 *
 * @param[in] handle    user handle returned by open
 * @param[in] data_addr the address of the data to append
 * @param[in] data_size size of the data
 * @param[in] data_type media type
 * @param[in] pts       time stamp in us
 * @param[in] timestamp time stamp in ms
 * @param[in] is_last_frame TRUE for last frame in aov file
 *
 * @return error code
 * - OPRT_OK  success
 * - Others   failed
 */
OPERATE_RET tuya_ipc_aov_buffer_append_data_with_timestamp(AOV_BUFFER_USER_HANDLE_T handle, UCHAR_T *data_addr,
                        UINT_T data_size, MEDIA_FRAME_TYPE_E data_type, UINT64_T pts, UINT64_T timestamp, BOOL_T is_last_frame);

/**
 * @brief get frame from aov buffer
 *
 * @param[in] handle    user handle returned by open
 * @param[in] is_retry  whether retry to get the last frame
 *
 * @return    the aov buffer node or NULL if there is no newer frame in aov buffer
 */
AOV_BUFFER_NODE_T* tuya_ipc_aov_buffer_get_frame(AOV_BUFFER_USER_HANDLE_T handle, BOOL_T is_retry);

/**
 * @brief get max frame size of the specified stream
 *
 * @param[in] device    device number
 * @param[in] channel   channel number in device
 * @param[in] stream    stream number in channel
 *
 * @return max frame size
 */
INT_T tuya_ipc_aov_buffer_get_max_frame_size(INT_T device, INT_T channel, IPC_STREAM_E stream);

/**
 * @brief set max frame size
 *
 * @param[in] device    device number
 * @param[in] channel   channel number in device
 * @param[in] stream    stream number in channel
 * @param[in] max_frame_size the max frame size that can be put into aov buffer
 *
 * @return    error code
 * - OPRT_OK  success
 * - Others   failed
 */
OPERATE_RET tuya_ipc_aov_buffer_set_max_frame_size(INT_T device, INT_T channel, IPC_STREAM_E stream, UINT_T max_frame_size);

/**
 * @brief get frame num by given size
 *
 * @param[in] handle      user handle returned by open
 * @param[in] limit_size  limited total frame size
 * @param[in] security_level security level
 * @param[out] p_encrypted_size total frame size after encrypted
 *
 * @return    frame number
 */
UINT_T tuya_ipc_aov_buffer_get_frame_num_by_size(AOV_BUFFER_USER_HANDLE_T handle, UINT_T limit_size, INT_T security_level, UINT_T *p_encrypted_size);

/**
 * @brief clean aov buffer, synchronous api
 *
 * @param[in] device    device number
 * @param[in] channel   channel number in device
 * @param[in] stream    stream number in channel
 *
 * @return    error code
 * - OPRT_OK  success
 * - Others   failed
 */
VOID tuya_ipc_aov_buffer_clean_buffer_sync(INT_T device, INT_T channel, IPC_STREAM_E stream);

#ifdef __cplusplus
}
#endif

#endif
