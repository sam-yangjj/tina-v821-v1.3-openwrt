/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/combine3a/audio_subsys_combine3a_general.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-10-23 16:23:23
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-10-24 14:08:35
 */
#ifndef __AUDIO_SUBSYS_COMBINE3A_GENERAL_H__
#define __AUDIO_SUBSYS_COMBINE3A_GENERAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

/**
 * XXX: (webrtc-apm) raw(1mic+1ref) -> aec -> ns/filter -> agc -> vad -> out(mono)
 *
 * FIXME: 因为基于webrtc-apm及分离裁剪的模块，默认支持时10ms每帧；基于speexdsp的模块，默认再10ms也是最优。
 *       所以，这里默认的raw_frame_duration为100ms，proc_frame_duration为10ms，保证原始数据的处理数据过渡。
 *       combine3a存在的价值时是将预期的处理流封装为整体按照module的使用方式使用，简化业务集成代码。
 */

typedef struct combine3a_general_config {
    int raw_frame_duration;     ///< raw frame duration (ms), must be a multiple of processed frame duration
    int proc_frame_duration;    ///< processed frame duration (ms)
    void *aec_cfg;
    void *ns_cfg;
    void *filter_cfg;
    void *agc_cfg;
    void *vad_cfg;
} combine3a_general_config_t;

#define COMBINE3A_GENERAL_CONFIG_DEFAULT() {    \
    .raw_frame_duration = 100,                  \
    .proc_frame_duration = 10,                  \
    .aec_cfg = NULL,                            \
    .ns_cfg = NULL,                             \
    .filter_cfg = NULL,                         \
    .agc_cfg = NULL,                            \
    .vad_cfg = NULL,                            \
}

extern audio_subsys_module_t audio_subsys_combine3a_general;

int audio_subsys_combine3a_general_create(void **self, void *cfg);
int audio_subsys_combine3a_general_destroy(void *self);
int audio_subsys_combine3a_general_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_combine3a_general_ioctl(void *self, int cmd, void *arg);
int audio_subsys_combine3a_general_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_COMBINE3A_GENERAL_H__ */
