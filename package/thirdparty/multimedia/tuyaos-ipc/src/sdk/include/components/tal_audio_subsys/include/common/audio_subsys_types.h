/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/common/audio_subsys_types.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-08-01 10:02:26
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-08-01 11:03:25
 */
#ifndef __AUDIO_SUBSYS_TYPES_H__
#define __AUDIO_SUBSYS_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_format {
    int sample_rate;        /**< Audio PCM SampleRate number */
    int bits_per_sample;    /**< Audio PCM BitsPreSample number, default: 16 */
    int channels;           /**< Audio PCM Channels number  */
    int is_interleaved;     /**< Audio PCM InterleavedMode bool value, 1: LRLR or 0: LLRR, default: 1 */
} audio_format_t;

#define AUDIO_FORMAT_DEFAULT() {        \
    .sample_rate = 16000,               \
    .bits_per_sample = 16,              \
    .channels = 1,                      \
    .is_interleaved = 1,                \
}

typedef struct audio_buffer {
    audio_format_t  format; /**< Audio Buffer Format */
    void           *data;   /**< Audio Buffer Data Print */
    int             size;   /**< Audio Buffer Size by Bytes Number */
} audio_buffer_t;

#define AUDIO_BUFFER_DEFAULT() {        \
    .format = AUDIO_FORMAT_DEFAULT(),   \
    .data = NULL,                       \
    .size = 0,                          \
}

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_TYPES_H__ */
