#ifndef __AUDIO_SUBSYS_AEC_H__
#define __AUDIO_SUBSYS_AEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AEC_CNG_MODE_FALSE 0
#define AEC_CNG_MODE_TRUE  1

#ifdef ARC_LINUX

#include <stdint.h>

typedef struct {
    int sample_rate;        /**< audio sample rate in Hz */
    int mic_channels;       /**< microphone channels, FIXME: NOW only support mono! */
    int ref_channels;       /**< reference channels, FIXME: NOW only support mono! */
    int snd_card_buf_delay; /**< sound card get audio buffer delay in ms */
    int cng_mode;           /**< AEC_CNG_MODE_FALSE, AEC_CNG_MODE_TRUE (default) */
    int echo_mode;          /**< 0, 1, 2, 3 (default), 4 */
} aecm_config_t;

int audio_subsys_aec_create(void **self, void *cfg);
int audio_subsys_aec_destroy(void *self);
int audio_subsys_aec_process(void *self, int16_t *mic, int16_t *ref, int16_t *aec);

#else

#include "audio_subsys.h"

typedef struct {
    int sample_rate;        /**< audio sample rate in Hz */
    int mic_channels;       /**< microphone channels, FIXME: NOW only support mono! */
    int ref_channels;       /**< reference channels, FIXME: NOW only support mono! */
    int snd_card_buf_delay; /**< sound card get audio buffer delay in ms */
    int cng_mode;           /**< AEC_CNG_MODE_FALSE, AEC_CNG_MODE_TRUE (default) */
    int echo_mode;          /**< 0, 1, 2, 3 (default), 4 */
} aec_config_t;

#define AEC_CONFIG_DEFAULT() {          \
    .sample_rate = 16000,               \
    .mic_channels = 1,                  \
    .ref_channels = 1,                  \
    .snd_card_buf_delay = 40,           \
    .cng_mode = AEC_CNG_MODE_TRUE,      \
    .echo_mode = 3,                     \
}

extern audio_subsys_module_t audio_subsys_aec;

#endif


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SUBSYS_AEC_H__ */
