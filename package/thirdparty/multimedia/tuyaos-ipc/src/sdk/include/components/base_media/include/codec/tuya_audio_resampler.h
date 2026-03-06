//
//  tuya_audio_resampler.h
//  tuya_audio_resampler
//
//  Created by jie-fei on 2020/8/10.
//  Copyright © 2020 jie-fei. All rights reserved.
//

#ifndef __TUYA_AUDIO_RESAMPLER_H__
#define __TUYA_AUDIO_RESAMPLER_H__

#include <stdio.h>
#include <stdbool.h>
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    INT_T channel;
    SIZE_T frame_size;      // samplerate / 100
    SIZE_T sample_rate_in;  // default 8000 or 16000
    SIZE_T sample_rate_out; // default 16000 or 8000
    VOID  *resampler_inst;  // webrtc resampler instance
    BOOL_T enabled;         // enable/disable resampler
} TY_AUDIO_RESAMPLER_HANDLE_T;

typedef struct {
    INT_T channel;
    SIZE_T frame_size;
    SIZE_T sample_rate_in;
    SIZE_T sample_rate_out;
    BOOL_T enabled;
} TY_AUDIO_RESAMPLER_PARAM_T;

/**
 * @ brief resample algorithm init
 * @ handle    全局结构体
 * @ param    初始化参数
 *
 * @ return
 *        0     succeed
 *        -1     failed
 */
INT_T ty_audio_resampler_init(TY_AUDIO_RESAMPLER_HANDLE_T *phdl, TY_AUDIO_RESAMPLER_PARAM_T *pparam);
/**
 * @ brief resample process
 * @ handle    全局结构体
 * @ in_buf     input
 * @ out_buf   output
 *
 * @ return
 *        0     succeed
 *        -1     failed
 */
INT_T ty_audio_resampler_process(TY_AUDIO_RESAMPLER_HANDLE_T *phdl, const SHORT_T *in_buf, SHORT_T *out_buf);

/**
 * @brief resample algorithm uninit
 * @handle    全局结构体
 *
 * @return
 *        0     succeed
 *        -1     failed
 */
INT_T ty_audio_resampler_uninit(TY_AUDIO_RESAMPLER_HANDLE_T *phdl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TUYA_AUDIO_RESAMPLER_H__ */
