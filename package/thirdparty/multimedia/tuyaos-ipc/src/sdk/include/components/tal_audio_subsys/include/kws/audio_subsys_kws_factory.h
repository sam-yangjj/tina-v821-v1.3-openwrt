#ifndef __AUDIO_SUBSYS_KWS_FACTORY_H__
#define __AUDIO_SUBSYS_KWS_FACTORY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef enum ModelType
{
    TYPE_DSTCN_MNN = 1, /*chunk = 300ms*/
    TYPE_MDTC_NCNN,     /*chunk = 2000ms*/
    TYPE_FSMN_CTC_MNN   /*chunk = 300ms*/
}ModelType;

typedef struct kws_factory_config
{
    int num_bins;
    int batch_size;
    int sample_rate;
    char *key_word;
    char *mnn_model_path;
    char *ncnn_model_path;
    char *ncnn_param_path;
    char *token_path;
    ModelType type;
    int iCHUNK;
}kws_factory_config_t;

#define KWS_FACTORY_CONFIG_DEFAULT_DSTCN_MNN() { \
    .num_bins = 40, \
    .batch_size = 1, \
    .sample_rate = 16000, \
    .mnn_model_path = (char *)"dstcn.mnn", \
    .type = TYPE_DSTCN_MNN, \
    .iCHUNK = 300*16, \
}

#define KWS_FACTORY_CONFIG_DEFAULT_FSMN_CTC_MNN() { \
    .num_bins = 40, \
    .batch_size = 1, \
    .sample_rate = 16000, \
    .mnn_model_path = (char *)"keyword_spot_fsmn_ctc_wenwen.mnn", \
    .token_path = (char *)"token.txt", \
    .key_word = (char *)"空气很好", \
    .type = TYPE_FSMN_CTC_MNN, \
    .iCHUNK = 300*16, \
}

#define KWS_FACTORY_CONFIG_DEFAULT_MDTC_NCNN() { \
    .num_bins = 80, \
    .batch_size = 1, \
    .sample_rate = 16000, \
    .ncnn_model_path = (char *)"mdtc.ncnn.bin", \
    .ncnn_param_path = (char *)"mdtc.ncnn.param", \
    .type = TYPE_MDTC_NCNN, \
    .iCHUNK = 2000*16, \
}

extern audio_subsys_module_t audio_subsys_kws_factory;

int audio_subsys_kws_factory_create(void **self, void *cfg);
int audio_subsys_kws_factory_destroy(void *self);
int audio_subsys_kws_factory_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_kws_factory_ioctl(void *self, int cmd, void *arg);
int audio_subsys_kws_factory_process(void *self, const void *in, int inlen, void *out, int outlen);


#ifdef __cplusplus
}
#endif

#endif