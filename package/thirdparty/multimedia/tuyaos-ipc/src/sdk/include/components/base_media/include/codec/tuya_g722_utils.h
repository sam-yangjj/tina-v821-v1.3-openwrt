
#ifndef __TUYA_AUDIO_G722_UTILS_H__
#define __TUYA_AUDIO_G722_UTILS_H__

#include <stdint.h>
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    /* 仅支持 48000kbps, 56000kbps, 64000kbps */
    INT_T bits_per_sample;
} TY_AUDIO_G722_DEC_PARAM_T;

typedef struct {
    UINT8_T *psrc;
    INT_T src_len;
    UINT8_T *pdest;
    INT_T dest_len;
} TY_AUDIO_G722_DEC_DATA_T;

typedef void* PG722_HANDLE; 

/**
 * @ brief init
 * @ phdl   句柄(出参)
 * @ pparam    初始化参数
 * @ return
 *        0     succeed
 *        -1     failed
 */
INT_T ty_audio_g722_decode_init(PG722_HANDLE *phdl, TY_AUDIO_G722_DEC_PARAM_T *pparam);
/**
 * @ brief 解码数据
 * @ handle    句柄
 * @ pdata     数据参数
 *
 * @ return 解码后的长度
 *        
 */
INT_T ty_audio_g722_decode_process(PG722_HANDLE phdl, TY_AUDIO_G722_DEC_DATA_T *pdata);

/**
 * @brief uninit
 * @handle    句柄
 *
 * @return
 *        0     succeed
 *        -1     failed
 */
INT_T ty_audio_g722_decode_uninit(PG722_HANDLE phdl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TUYA_AUDIO_G722_UTILS_H__ */
