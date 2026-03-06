/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/kws/audio_subsys_kws_factory2.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2025-02-19 16:14:37
 * @LastEditors: clare.phang
 * @LastEditTime: 2025-02-19 18:08:45
 */
#ifndef __AUDIO_SUBSYS_KWS_FACTORY2_H__
#define __AUDIO_SUBSYS_KWS_FACTORY2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"
#define KWS_WINDOW_SIZE 30
#define KWS_WORDS 2

typedef struct kws_factory2_config_t {
    /* multi kws factory config filed */
    int     interpreter_type;   ///< 0: ort, 1: ncnn, 2: mnn
    int     chunk_size;         ///< dstcn+mnn: 300ms, mdtc+ncnn: 2000ms, fsmn ctc+mnn: 300ms. TODO: what? optimize！
    int     feats_size;         ///< dstcn+mnn: 30, fsmn ctc+mnn: 10, mdtc+ncnn: 200. TODO: what? optimize！

    int     sample_rate;
    int     interval;
    int     model_type;         ///< 0: dstcn maxpooling, 1: fsmn ctc, 2: mdtc maxpooling
    int     num_bins;           ///< dstcn: 40, fsmn ctc: 80, mdtc: 80.
    int     batch_size;         ///< dstcn+mnn: 10, fsmn ctc+mnn: 10, mdtc+ncnn: 10.
    float   threshold;
    char   *key_word;
    char   *model_path;
    char   *param_path;         ///< only for ncnn!
    char   *token_path;
} kws_factory2_config_t;

#define KWS_FACTORY_CONFIG_DEFAULT_ORT_DSTCN_MAXPOOLING_NIHAO_WENWEN() {\
    .interpreter_type = 0,                                              \
    .chunk_size = 300 * 16,                                             \
    .feats_size = 30,                                                   \
                                                                        \
    .sample_rate = 16000,                                               \
    .interval = 100,                                                    \
    .model_type = 0,                                                    \
    .num_bins = 40,                                                     \
    .batch_size = 10,                                                   \
    .threshold = 0.8,                                                   \
    .key_word = (char *)"你好问问",                                      \
    .model_path = (char *)"./model/dstcn-maxpooling-wenwen.ort",        \
    .token_path = (char *)"./model/dstcn-maxpooling-wenwen.token.txt",  \
}

#define KWS_FACTORY_CONFIG_DEFAULT_MNN_DSTCN_MAXPOOLING_NIHAO_WENWEN() {\
    .interpreter_type = 0,                                              \
    .chunk_size = 300 * 16,                                             \
    .feats_size = 30,                                                   \
                                                                        \
    .sample_rate = 16000,                                               \
    .interval = 100,                                                    \
    .model_type = 0,                                                    \
    .num_bins = 40,                                                     \
    .batch_size = 10,                                                   \
    .threshold = 0.8,                                                   \
    .key_word = (char *)"你好问问",                                      \
    .model_path = (char *)"./model/dstcn-maxpooling-wenwen.mnn",        \
    .token_path = (char *)"./model/dstcn-maxpooling-wenwen.token.txt",  \
}

#define KWS_FACTORY_CONFIG_DEFAULT_MNN_FSMN_CTC_NIHAO_WENWEN() {        \
    .interpreter_type = 0,                                              \
    .chunk_size = 300 * 16,                                             \
    .feats_size = 10,                                                   \
                                                                        \
    .sample_rate = 16000,                                               \
    .interval = 100,                                                    \
    .model_type = 0,                                                    \
    .num_bins = 80,                                                     \
    .batch_size = 10,                                                   \
    .threshold = 0.1,                                                   \
    .key_word = (char *)"你好问问",                                      \
    .model_path = (char *)"./model/fsmn-ctc-wenwen.mnn",                \
    .token_path = (char *)"./model/fsmn-ctc-wenwen.tokens.txt",         \
}

#define KWS_FACTORY_CONFIG_DEFAULT_NCNN_MDTC_NIHAO_WENWEN() {           \
    .interpreter_type = 0,                                              \
    .chunk_size = 2000 * 16,                                            \
    .feats_size = 200,                                                  \
                                                                        \
    .sample_rate = 16000,                                               \
    .interval = 100,                                                    \
    .model_type = 0,                                                    \
    .num_bins = 80,                                                     \
    .batch_size = 10,                                                   \
    .threshold = 0.8,                                                   \
    .key_word = (char *)"你好问问",                                      \
    .model_path = (char *)"./model/mdtc-wenwen-ncnn.model.bin",         \
    .param_path = (char *)"./model/mdtc-wenwen-ncnn.param.bin",         \
    .token_path = (char *)"./model/mdtc-wenwen-ncnn.token.txt",         \
}

extern audio_subsys_module_t audio_subsys_kws_factory2;

int audio_subsys_kws_factory2_create(void **self, void *cfg);
int audio_subsys_kws_factory2_destroy(void *self);
int audio_subsys_kws_factory2_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_kws_factory2_ioctl(void *self, int cmd, void *arg);
int audio_subsys_kws_factory2_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** ！__AUDIO_SUBSYS_KWS_FACTORY2_H__ */
