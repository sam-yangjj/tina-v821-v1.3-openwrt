/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/audio_subsys.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-24 10:55:31
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-11-08 12:00:14
 */
#ifndef __AUDIO_SUBSYS_H__
#define __AUDIO_SUBSYS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/audio_subsys_types.h"
#include "common/audio_subsys_module.h"

#include "utils/audio_subsys_utils.h"

#include "aec/audio_subsys_aec.h"
#include "agc/audio_subsys_agc.h"
#include "filter/audio_subsys_filter.h"
#include "ns/audio_subsys_ns.h"
#include "resample/audio_subsys_resample.h"
#include "vad/audio_subsys_vad.h"
#include "doabf/audio_subsys_doabf.h"
#include "doabf/audio_subsys_odaslive.h"

#include "speexdsp/audio_subsys_speexdsp_wrap.h"
#include "webrtc_apm/audio_subsys_webrtc_apm_wrap.h"

#include "manager/audio_subsys_manager.h"

#include "combine3a/audio_subsys_combine3a.h"

#include "kws/audio_subsys_kws.h"
#include "rnn_vad/audio_subsys_rnn_vad.h"
#ifdef __cplusplus
}
#endif

#endif /* !__AUDIO_SUBSYS_H__ */
