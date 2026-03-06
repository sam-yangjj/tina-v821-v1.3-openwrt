 /**
 * @file tkl_audio.c
 * @brief Common process - adapter the audio api
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "tkl_audio.h"

#include "plat_log.h"
#include "media_common_aio.h"
#include "mpi_sys.h"
#include "mpi_ai.h"

typedef struct DemoAudioContext
{
    int nAudioChnNum;
    int nAudioBitWidth;
    int nAudioSamplesPerFrame;
    int nAudioSampleRate;
    int nAudioBitRate; //bps

    AUDIO_DEV mAIDevId;
    AI_CHN mAIChnId;
    AIO_ATTR_S mAioAttr;
//---------------------
    int ao_dev;
    AO_CHN ao_chn;
    int playback_volume;
}DemoAudioContext;

DemoAudioContext *gpDemoAudioContext = NULL;

float fDBThreshold = 37;
static pthread_t mAiRunThreadId;
static int capture_task_running = 0;

extern void tuya_audio_stream_cb(TKL_AUDIO_FRAME_INFO_T *frame, int channel);
static TKL_AUDIO_FRAME_INFO_T s_audio_main_frame;
#define TUYA_AUDIO_MAIN_CHANNEL 9


static void *audio_capture_task(void *arg)
{
    ERRORTYPE ret;
    DemoAudioContext *context = (DemoAudioContext *)arg;
    prctl(PR_SET_NAME, "audiocap", 0, 0, 0);
    AUDIO_FRAME_S frame;

    capture_task_running = 1;
    while (capture_task_running) {
        memset(&frame, 0, sizeof(AUDIO_FRAME_S));
        ret = AW_MPI_AI_GetFrame(context->mAIDevId, context->mAIChnId, &frame, NULL, 500);
        if (ret != SUCCESS) {
            usleep(10*1000);
            continue;
        }

        s_audio_main_frame.pbuf = frame.mpAddr;
        s_audio_main_frame.used_size = frame.mLen;
        s_audio_main_frame.type = TKL_AUDIO_FRAME;
        s_audio_main_frame.pts = frame.mTimeStamp / 1000;
        tuya_audio_stream_cb(&s_audio_main_frame, TUYA_AUDIO_MAIN_CHANNEL);
        AW_MPI_AI_ReleaseFrame(context->mAIDevId, context->mAIChnId, &frame, NULL);
    }

    return (void *)NULL;
}

/**
* @brief ai init
* 
* @param[in] pconfig: audio config
* @param[in] count: count of pconfig
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_init(TKL_AUDIO_CONFIG_T *pconfig, INT32_T count)
{
    alogd("enter");
    int ret;
    DemoAudioContext *pContext = calloc(1, sizeof(DemoAudioContext));
    if(NULL == pContext)
    {
        aloge("fatal error! malloc fail");
    }
    gpDemoAudioContext = pContext;
    //config init param here
    pContext->nAudioChnNum = 1;
    pContext->nAudioBitWidth = 16;
    pContext->nAudioSampleRate = 8000;
    pContext->nAudioBitRate = 16000;
    pContext->playback_volume = 90;

    pContext->mAIDevId = 0;
    pContext->mAIChnId = 0;
    pContext->mAioAttr.enSamplerate = map_SampleRate_to_AUDIO_SAMPLE_RATE_E(pContext->nAudioSampleRate);
    pContext->mAioAttr.enBitwidth = map_BitWidth_to_AUDIO_BIT_WIDTH_E(pContext->nAudioBitWidth);
    pContext->mAioAttr.mChnCnt = pContext->nAudioChnNum;
    pContext->mAioAttr.mMicNum = 1;
    pContext->mAioAttr.ai_aec_en = 0;
    pContext->mAioAttr.mbBypassAec = 0;
    pContext->mAioAttr.ai_ans_en = 0;
    pContext->mAioAttr.ai_agc_en = 0;
    AW_MPI_AI_SetPubAttr(pContext->mAIDevId, &pContext->mAioAttr);

    AI_CHN_ATTR_S ai_chn_attr;
    memset(&ai_chn_attr, 0, sizeof(ai_chn_attr));
    ai_chn_attr.nFrameSize = 128;
    ret = AW_MPI_AI_CreateChn(pContext->mAIDevId, pContext->mAIChnId, &ai_chn_attr);
    if (ret != SUCCESS)
    {
        aloge("fatal error! create ai chn fail");
    }
    AW_MPI_AI_EnableChn(pContext->mAIDevId, pContext->mAIChnId);
    pthread_create(&mAiRunThreadId, NULL, audio_capture_task, (void *)pContext);

    return OPRT_OK;
}
/**
* @brief ai start
* 
* @param[in] card: card number
* @param[in] chn: channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_start(INT32_T card, TKL_AI_CHN_E chn)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief ai set mic volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_set_vol(INT32_T card, TKL_AI_CHN_E chn, INT32_T vol)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief ai get frame
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[out] pframe: audio frame, pframe->pbuf allocated by upper layer application
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_get_frame(INT32_T card, TKL_AI_CHN_E chn, TKL_AUDIO_FRAME_INFO_T *pframe)
{
    return OPRT_OK;
}

/**
* @brief ai stop
* 
* @param[in] card: card number
* @param[in] chn: channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_stop(INT32_T card, TKL_AI_CHN_E chn)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief ai uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_uninit(VOID)
{
    alogd("enter");
    if(capture_task_running == 0)
        return OPRT_OK;
    capture_task_running = 0;
    void *pRetVal = NULL;
    pthread_join(mAiRunThreadId, &pRetVal);
    alogd("destroy capture pthread");
    return OPRT_OK;
}

/**
* @brief ao init
* 
* @param[in] pconfig: audio config
* @param[in] count: config count
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
// static char tuya_file[64] = "/mnt/extsd/tuya_cloud.wav";
// static FILE *fp = NULL;
OPERATE_RET tkl_ao_init(TKL_AUDIO_CONFIG_T *pconfig, INT32_T count, VOID **handle)
{
    alogd("enter");

    if (gpDemoAudioContext == NULL) {
        aloge("gpDemoAudioContext is null");
        return OPRT_COM_ERROR;
    }
    DemoAudioContext *context = gpDemoAudioContext;

    context->ao_dev = 0;
    context->ao_chn = 0;
    AW_MPI_AO_CreateChn(context->ao_dev, context->ao_chn);

    AW_MPI_AO_StartChn(context->ao_dev, context->ao_chn);
    AW_MPI_AO_SetDevVolume(context->ao_dev, context->playback_volume);

    return OPRT_OK;
}

/**
* @brief ao start
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[out] handle: handle of start
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_start(INT32_T card, TKL_AO_CHN_E chn, VOID *handle)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief ao set volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_set_vol(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, INT32_T vol)
{
    alogd("enter");

    if (gpDemoAudioContext == NULL) {
        aloge("gpDemoAudioContext is null");
        return OPRT_COM_ERROR;
    }
    DemoAudioContext *context = gpDemoAudioContext;

    if (vol < 0 || vol > 100) {
        aloge("vol is illegal");
        return;
    }
    context->playback_volume = vol;

    alogd("Set Ao Volume:%d\n", vol);
    AW_MPI_AO_SetDevVolume(context->ao_dev, vol);
    return OPRT_OK;
}

/**
* @brief ao get volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_get_vol(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, INT32_T *vol)
{
    alogd("enter");
    if (gpDemoAudioContext == NULL) {
        aloge("gpDemoAudioContext is null");
        return OPRT_COM_ERROR;
    }
    DemoAudioContext *context = gpDemoAudioContext;

    AW_MPI_AO_GetDevVolume(context->ao_dev, vol);
    alogd("Get Ao Volume:%d\n", *vol);

    return OPRT_OK;
}

/**
* @brief ao output frame
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] handle: handle of start
* @param[in] pframe: output frame
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_put_frame(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, TKL_AUDIO_FRAME_INFO_T *pframe)
{
    if (gpDemoAudioContext == NULL) {
        aloge("gpDemoAudioContext is null");
        return OPRT_COM_ERROR;
    }
    DemoAudioContext *context = gpDemoAudioContext;

    AUDIO_FRAME_S frame;
    memset(&frame, 0, sizeof(frame));

    frame.mBitwidth = map_AUDIO_BIT_WIDTH_E_to_BitWidth(pframe->datebits);
    frame.mSamplerate = map_AUDIO_SAMPLE_RATE_E_to_SampleRate(pframe->sample);
    frame.mSoundmode = (pframe->channel == 1) ? AUDIO_SOUND_MODE_MONO : AUDIO_SOUND_MODE_STEREO;
    frame.mLen = pframe->used_size;
    frame.mpAddr = pframe->pbuf;
    AW_MPI_AO_SendFrameSync(context->ao_dev, context->ao_chn, &frame);

    return OPRT_OK;
}

/**
* @brief ao stop
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] handle: handle of start
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_stop(INT32_T card, TKL_AO_CHN_E chn, VOID *handle)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief ao uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_uninit(VOID *handle)
{
    alogd("enter");
    if (gpDemoAudioContext == NULL) {
        aloge("gpDemoAudioContext is null");
        return OPRT_COM_ERROR;
    }
    DemoAudioContext *context = gpDemoAudioContext;

    AW_MPI_AO_StopChn(context->ao_dev, context->ao_chn);
    AW_MPI_AO_DestroyChn(context->ao_dev, context->ao_chn);
    return OPRT_OK;
}

/**
* @brief audio input detect start
* 
* @param[in] card: card number
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_start(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief audio input detect stop
* 
* @param[in] card: card number
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_stop(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief audio detect get result
* 
* @param[in] card: card number
* @param[in] type: detect type
* @param[out] presult: audio detect result
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_get_result(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type, TKL_AUDIO_DETECT_RESULT_T *presult)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}
