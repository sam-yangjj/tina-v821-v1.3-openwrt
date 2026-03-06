#ifndef __AUDIO_SUBSYS_NS_H__
#define __AUDIO_SUBSYS_NS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NSLevel_Low,
    NSLevel_Moderate,
    NSLevel_High,
    NSLevel_VeryHigh
} NSLevel;

typedef struct {
    int sample_rate;
    NSLevel level;
} ns_config_t;

#ifdef ARC_LINUX

int audio_subsys_ns_create(void **self, void *cfg);
int audio_subsys_ns_destroy(void *self);
int audio_subsys_ns_process(void *self, const void *in, int inlen, void *out, int outlen);

#else 

#include "audio_subsys.h"
#define NS_CONFIG_DEFAULT() {   \
    .sample_rate = 16000,       \
    .level = NSLevel_Moderate,  \
}

extern audio_subsys_module_t audio_subsys_ns;

#endif

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SUBSYS_NS_H__ */
