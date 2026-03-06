/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/agc/audio_subsys_agc.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-26 17:43:02
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-07-25 17:52:59
 */
#ifndef __AUDIO_SUBSYS_AGC_H__
#define __AUDIO_SUBSYS_AGC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef struct agc_config {
    /** Sample rate in Hz, default is 16000. */
    int sample_rate;           ///< default 16000

    /** Target level in dBFS, default is -3 dBOv (3). */
    int target_level_db_fs;     ///< default 3 (-3 dBOv)

    /** Compression gain in dB, default is 9 dB. */
    int compression_gain_db;    ///< default 9 dB

    /** Limiter enable flag, default is kAgcTrue (on). */
    int limiter_enable;         ///< default kAgcTrue (on)

    /** AGC mode, default is kAgcModeAdaptiveDigital (2).
     *
     *  Possible values are:
     *      - kAgcModeUnchanged (0)
     *      - kAgcModeAdaptiveAnalog (1)
     *      - kAgcModeAdaptiveDigital (2)
     *      - kAgcModeFixedDigital (3)
     */
    int mode;                   ///< default kAgcModeAdaptiveDigital (2)

    /** Input microphone level, default is 0. */
    int in_mic_level;           ///< default 128
} agc_config_t;

#define AGC_CONFIG_DEFAULT() { \
    .sample_rate = 16000, \
    \
    .target_level_db_fs = 3, \
    .compression_gain_db = 9, \
    .limiter_enable = 1, \
    \
    .mode = 2, \
    \
    .in_mic_level = 128,  \
}

extern audio_subsys_module_t audio_subsys_agc;

int audio_subsys_agc_create(void **self, void *cfg);
int audio_subsys_agc_destroy(void *self);
int audio_subsys_agc_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_agc_ioctl(void *self, int cmd, void *arg);
int audio_subsys_agc_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_AGC_H__ */