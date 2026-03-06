/*
 * @Description:
 * @FilePath: /tal_audio_subsys/include/vad/audio_subsys_vad.h
 * @Version: v0.0.0
 * @Company:
 * @Author:
 * @Date: 2024-07-19
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-08-02 12:08:56
 */
#ifndef __AUDIO_SUBSYS_VAD_H__
#define __AUDIO_SUBSYS_VAD_H__

#include "audio_subsys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sample_rate;    /**< 支持8000, 16000, 32000, 48000 */
    int mode;           /**< 建议1，Aggressiveness mode (0, 1, 2, or 3)
                         *  0: Quality mode, 1: Low bitrate mode, 2. Aggressive mode, 3: Very aggressive mode
                         */
} vad_config_t;

#define VAD_CONFIG_DEFAULT() {      \
    .sample_rate = 16000,           \
    .mode = 1,                      \
}

int audio_subsys_vad_create(void **self, void *cfg);
int audio_subsys_vad_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_vad_process(void *self, const void *in, int inlen, void *out, int outlen);
int audio_subsys_vad_destroy(void *self);

extern audio_subsys_module_t audio_subsys_vad;

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SUBSYS_VAD_H__ */
