/*
 * @Description:
 * @FilePath: /tal_audio_subsys/include/rnn_vad/audio_subsys_rnn_vad.h
 * @Version: v0.0.0
 * @Company:
 * @Author:
 * @Date: 2025-06-18
 * @LastEditors: wrynn
 * @LastEditTime: 2025-06-18 12:08:56
 */

#ifndef __AUDIO_SUBSYS_RNN_VAD_H__
#define __AUDIO_SUBSYS_RNN_VAD_H__

#include "audio_subsys.h"

#ifdef __cplusplus
extern "C" {
#endif

int audio_subsys_rnn_vad_create(void **self, void *cfg);
int audio_subsys_rnn_vad_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_rnn_vad_process(void *self, const void *in, int inlen, void *out, int outlen);
int audio_subsys_rnn_vad_destroy(void *obj);
extern audio_subsys_module_t audio_subsys_rnn_vad;
#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SUBSYS_RNN_VAD_H__ */

