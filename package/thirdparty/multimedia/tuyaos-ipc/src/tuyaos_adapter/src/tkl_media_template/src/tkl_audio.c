 /**
 * @file tkl_audio.c
 * @brief Common process - adapter the audio api
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_audio.h"
#include <mpi_ao.h>
#include "media_common_aio.h"
#include "plat_log.h"

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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
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
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief ai uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_uninit(VOID)
{
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
OPERATE_RET tkl_ao_init(TKL_AUDIO_CONFIG_T *pconfig, INT32_T count, VOID **handle)
{
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}


void linear_VolumeAdjust(int targetVolume)
{
    int hardwareVolume;
    int softwareGain;

    if (targetVolume < 20) {
        hardwareVolume = targetVolume * 3;
        softwareGain = 0;
    } else if (targetVolume < 60) {
        hardwareVolume = 60;
        softwareGain = (targetVolume - 20) * 30 / (60 - 20);
    } else {
        hardwareVolume = targetVolume;
        softwareGain = 30;
    }

    AW_MPI_AO_SetDevVolume(0, hardwareVolume);
    AW_MPI_AO_SetSoftVolume(0, softwareGain);
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

    if (vol < 0 || vol > 100) {
        aloge("vol is illegal");
        return OPRT_COM_ERROR;
    }
    alogd("Set Ao Volume:%d\n", vol);
    // AW_MPI_AO_SetDevVolume(0, vol);
    linear_VolumeAdjust(vol);
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
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
    AUDIO_FRAME_S frame;
    memset(&frame, 0, sizeof(frame));

    frame.mBitwidth = map_AUDIO_BIT_WIDTH_E_to_BitWidth(pframe->datebits);
    frame.mSamplerate = map_AUDIO_SAMPLE_RATE_E_to_SampleRate(pframe->sample);
    frame.mSoundmode = (pframe->channel == 1) ? AUDIO_SOUND_MODE_MONO : AUDIO_SOUND_MODE_STEREO;
    frame.mLen = pframe->used_size;
    frame.mpAddr = pframe->pbuf;
    AW_MPI_AO_SendFrameSync(0, 0, &frame);

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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief ao uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_uninit(VOID *handle)
{
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}
