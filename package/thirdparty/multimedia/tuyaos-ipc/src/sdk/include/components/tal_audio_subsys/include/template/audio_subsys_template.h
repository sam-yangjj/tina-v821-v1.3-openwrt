/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/template/audio_subsys_template.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-24 15:48:15
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-06-26 17:50:13
 */
#ifndef __AUDIO_SUBSYS_TEMPLATE_H__
#define __AUDIO_SUBSYS_TEMPLATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef struct template_config {
    int sample_rate;
    int channels;
    int bits_per_sample;
} template_config_t;

typedef enum template_cmd {
    TEMPLATE_CMD_SET_CONFIG = 0,
    TEMPLATE_CMD_GET_CONFIG,
    TEMPLATE_CMD_START,
    TEMPLATE_CMD_STOP,
    TEMPLATE_CMD_PAUSE,
    TEMPLATE_CMD_RESUME,
    TEMPLATE_CMD_RESET,
} template_cmd_t;

int audio_subsys_template_create(void **self, void *cfg);
int audio_subsys_template_destroy(void *self);
int audio_subsys_template_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_template_ioctl(void *self, int cmd, void *arg);
int audio_subsys_template_process(void *self, const void *in, int inlen, void *out, int outlen);

extern audio_subsys_module_t audio_subsys_template_module;

#ifdef __cplusplus
}
#endif

#endif /* !__AUDIO_SUBSYS_TEMPLATE_H__ */
