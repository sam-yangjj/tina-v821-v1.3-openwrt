 /**
 * @file tkl_video_enc.c
 * @brief video encode
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 * Set video parameters and get video bitstream.
 */

#include "tkl_video_enc.h"
#include "sample_smartIPC_demo_ty.h"

extern int startJpegPicture(VencStreamContext *stream);
extern int stopJpegPicture(VencStreamContext *stream);
extern SampleSmartIPCDemoContext *gpSampleSmartIPCDemoContext;

static char SetOsdExitFlag;
pthread_t mOsdRunThreadId;

/**
* @brief video encode init
* 
* @param[in] vi_chn: vi channel number
* @param[in] pconfig: venc config
* @param[in] count: count of pconfig
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_init(INT32_T vi_chn, TKL_VENC_CONFIG_T *pconfig, INT32_T count)
{
    return OPRT_OK;
}

// 参考 takePictureThread()写法
int getJpegStream(TKL_VENC_FRAME_T *pframe)
{
    int ret = 0;
    if (gpSampleSmartIPCDemoContext == NULL) {
        aloge("gpSampleSmartIPCDemoContext is null");
        return -1;
    }
    SampleSmartIPCDemoContext *pContext = gpSampleSmartIPCDemoContext;
    VencStreamContext *pStreamContext = &pContext->mMain2ndStream;

    VIDEO_FRAME_INFO_S video_frame;
    memset(&video_frame, 0, sizeof(video_frame));

    unsigned char *stream_buf = NULL;
    unsigned int buf_size = 0;

    if (pContext->mConfigPara.mMain2ndEnable == 0 || pStreamContext->mTakePicture == 0) {
        aloge("config file not enable");
        return -1;
    }

    ViVirChnAttrS mVirviChnAttr;
    memset(&mVirviChnAttr, 0, sizeof(ViVirChnAttrS));
    mVirviChnAttr.mCacheFrameNum = 1;
    AW_MPI_VI_CreateVirChn(pStreamContext->mVipp, pStreamContext->mTakePictureViChn, &mVirviChnAttr);
    AW_MPI_VI_EnableVirChn(pStreamContext->mVipp, pStreamContext->mTakePictureViChn);

    if (SUCCESS != startJpegPicture(pStreamContext))
    {
        aloge("fatal error! VeChn%d startJpegPicture fail!", pStreamContext->mTakePictureVeChn);
        ret = -1;
        goto err1;
    }

    // struct timeval tv;
    // struct tm *tm_info;
    // char time_str[64];

    // gettimeofday(&tv, NULL);
    // tm_info = localtime(&tv.tv_sec);
    // strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    // CreateStreamOSD(&pContext->mStreamOsdConfig, 8, "0", 24, 16, 3);
    // UpdateStreamOSDRgb(&pContext->mStreamOsdConfig, 8, time_str, 24, 16, 3);

retry:
    ret = AW_MPI_VI_GetFrame(pStreamContext->mVipp, pStreamContext->mTakePictureViChn, &video_frame, 2000);
    if (ret != 0)
    {
        goto retry;
    }
    if (pStreamContext->mTakePictureOnlyCaptureYuv)
    {
        //暂无需求
        aloge("TODO");
    }
    else
    {
        ret = AW_MPI_VENC_SendFrameSync(pStreamContext->mTakePictureVeChn, &video_frame, 10 * 1000);
        if (ret != 0)
        {
            aloge("fatal error! VeChn%d send frame sync failed! ret[0x%x]", pStreamContext->mTakePictureVeChn, ret);
            AW_MPI_VI_ReleaseFrame(pStreamContext->mVipp, pStreamContext->mTakePictureViChn, &video_frame);
            ret = -1;
            goto err2;
        }
        alogd("VeChn%d send frame sync done", pStreamContext->mTakePictureVeChn);
        AW_MPI_VI_ReleaseFrame(pStreamContext->mVipp, pStreamContext->mTakePictureViChn, &video_frame);

        VENC_STREAM_S venc_stream;
        VENC_PACK_S pack;
        memset(&venc_stream, 0, sizeof(venc_stream));
        memset(&pack, 0, sizeof(pack));
        venc_stream.mPackCount = 1;
        venc_stream.mpPack = &pack;

        ret = AW_MPI_VENC_GetStream(pStreamContext->mTakePictureVeChn, &venc_stream, 1000);
        if (ret != SUCCESS)
        {
            aloge("fatal error! VeChn%d why get stream fail?", pStreamContext->mTakePictureVeChn);
            ret = -1;
        }
        else
        {
            int len = 0;
            stream_buf = pframe->pbuf;
            buf_size = pframe->buf_size;

            if(venc_stream.mpPack[0].mpAddr0 != NULL && venc_stream.mpPack[0].mLen0 > 0)
            {
                if (buf_size >= (len + venc_stream.mpPack[0].mLen0))
                {
                    memcpy(stream_buf + len, venc_stream.mpPack->mpAddr0, venc_stream.mpPack->mLen0);
                    len += venc_stream.mpPack->mLen0;
                }
            }
            if(venc_stream.mpPack[0].mpAddr1 != NULL && venc_stream.mpPack[0].mLen1 > 0)
            {
                if (buf_size >= (len + venc_stream.mpPack->mLen1))
                {
                    memcpy(stream_buf + len, venc_stream.mpPack->mpAddr1, venc_stream.mpPack->mLen1);
                    len += venc_stream.mpPack->mLen1;
                }
            }
            if(venc_stream.mpPack[0].mpAddr2 != NULL && venc_stream.mpPack[0].mLen2 > 0)
            {
                if (buf_size >= (len + venc_stream.mpPack->mLen2))
                {
                    memcpy(stream_buf + len, venc_stream.mpPack->mpAddr2, venc_stream.mpPack->mLen2);
                    len += venc_stream.mpPack->mLen2;
                }
            }

            if (len > 0)
            {
                pframe->codectype = TKL_CODEC_VIDEO_MJPEG;
                pframe->used_size = len;
            }

            // char strFilePath[MAX_FILE_PATH_SIZE];
            // if (pStreamContext->mTakePictureFileCurCnt >= pStreamContext->mTakePictureFileCnt)
            //     pStreamContext->mTakePictureFileCurCnt = 0;
            // snprintf(strFilePath, MAX_FILE_PATH_SIZE, "%s_%d_TU.jpg", pStreamContext->mTakePictureFile, pStreamContext->mTakePictureFileCurCnt);
            // FILE *fpPic = fopen(strFilePath, "wb");
            // if(fpPic != NULL)
            // {
            //     if(venc_stream.mpPack[0].mpAddr0 != NULL && venc_stream.mpPack[0].mLen0 > 0)
            //         fwrite(venc_stream.mpPack[0].mpAddr0, 1, venc_stream.mpPack[0].mLen0, fpPic);
            //     if(venc_stream.mpPack[0].mpAddr1 != NULL && venc_stream.mpPack[0].mLen1 > 0)
            //         fwrite(venc_stream.mpPack[0].mpAddr1, 1, venc_stream.mpPack[0].mLen1, fpPic);
            //     if(venc_stream.mpPack[0].mpAddr2 != NULL && venc_stream.mpPack[0].mLen2 > 0)
            //         fwrite(venc_stream.mpPack[0].mpAddr2, 1, venc_stream.mpPack[0].mLen2, fpPic);
            //     pStreamContext->mTakePictureFileCurCnt++;
            //     fclose(fpPic);
            //     alogd("VeChn%d store jpeg in file[%s]", pStreamContext->mTakePictureVeChn, strFilePath);
            // }
            // else
            // {
            //     aloge("fatal error! VeChn%d open file[%s] fail! errno(%d)", pStreamContext->mTakePictureVeChn, strFilePath, errno);
            // }
        }

        ret = AW_MPI_VENC_ReleaseStream(pStreamContext->mTakePictureVeChn, &venc_stream);
        if (ret != SUCCESS)
        {
            aloge("fatal error! VeChn%d why release stream fail(0x%x)?", pStreamContext->mTakePictureVeChn, ret);
            ret = -1;
        }
    }

err2:
    // DestroyStreamOSD(&pContext->mStreamOsdConfig, 8, 3);
    stopJpegPicture(pStreamContext);
err1:
    AW_MPI_VI_DisableVirChn(pStreamContext->mVipp, pStreamContext->mTakePictureViChn);
    AW_MPI_VI_DestroyVirChn(pStreamContext->mVipp, pStreamContext->mTakePictureViChn);

    return ret;
}

/**
* @brief video encode get frame
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[out] pframe:  output frame
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_get_frame(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_FRAME_T *pframe)
{
    alogd("enter");
    OPERATE_RET ret = 0;

    if (venc_chn == 8) { //抓JPEG图
        ret = getJpegStream(pframe);
        if (ret != 0) {
            alogd("get Jpeg fail");
            return OPRT_COM_ERROR;
        }
    }

    return OPRT_OK;
}

/**
* @brief video first snap
* 
* @param[in] vi_chn: vi channel number
* @param[out] pframe: output frame
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_get_first_snap(TKL_VI_CHN_E vi_chn, TKL_VENC_FRAME_T *pframe)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

static void *setOsdThread(void *arg)
{
    SampleSmartIPCDemoContext *pThreadData = (SampleSmartIPCDemoContext *)arg;
    StreamOsdConfig *pContext = &pThreadData->mStreamOsdConfig;

    FONT_SIZE_TYPE font_size = FONT_SIZE_32;

    int ret = load_font_file(font_size);
    if (ret < 0)
    {
        aloge("load_font_file %d fail! ret:%d\n", ret, font_size);
        return NULL;
    }

    CreateStreamOSD(pContext, 0, "0", 48, 32, 0);
#ifndef IPC_DUAL
    CreateStreamOSD(pContext, 2, "0", 24, 16, 1);
#endif
    struct timeval tv;
    struct tm *tm_info;
    char time_str[64];
    SetOsdExitFlag = 0;
    while (!SetOsdExitFlag)
    {
        gettimeofday(&tv, NULL);
        tm_info = localtime(&tv.tv_sec);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        UpdateStreamOSDRgb(pContext, 0, time_str, 48, 32, 0);
#ifndef IPC_DUAL
        UpdateStreamOSDRgb(pContext, 2, time_str, 24, 16, 1);
#endif
        usleep(1000 * 1000);
    }
    DestroyStreamOSD(pContext, 0, 0);
#ifndef IPC_DUAL
    DestroyStreamOSD(pContext, 2, 1);
#endif
    ret = unload_font_file(font_size);
    if (ret < 0)
    {
        aloge("unload_font_file %d fail! ret:%d\n", ret, font_size);
    }
}

/**
* @brief video encode set osd
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[out] posd:  osd config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_set_osd(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_OSD_T *posd)
{
    alogd("enter, enable:%d", posd->enable);
    if (gpSampleSmartIPCDemoContext == NULL) {
        aloge("gpSampleSmartIPCDemoContext is null");
        return OPRT_COM_ERROR;
    }
    SampleSmartIPCDemoContext *pContext = gpSampleSmartIPCDemoContext;
    int result = 0;

    if (posd->enable == 1) {
        alogd("Create Osd pthread");
        result = pthread_create(&mOsdRunThreadId, NULL, setOsdThread, (SampleSmartIPCDemoContext *)pContext);
        if (result != 0) {
            aloge("fatal error! pthread create fail[%d]", result);
            return OPRT_COM_ERROR;
        }
    } else {
        void *pRetVal = NULL;
        SetOsdExitFlag = 1;
        pthread_join(mOsdRunThreadId, &pRetVal);
        alogd("destroy Osd pthread");
    }
    return OPRT_OK;
}

/**
* @brief video encode set mask
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[out] pmask: mask config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_set_mask(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_MASK_T *pmask)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video encode stream buff pool set
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[in] parg:  buff pool config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_set_video_stream_buffer(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_STREAM_BUFF_T *parg)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video encode  start
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_start(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video encode  stop
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_stop( TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video encode uninit
* 
* @param[in] vi_chn: vi channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_uninit(TKL_VI_CHN_E vi_chn)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video venc set time cb
* 
* @param[in] cb: venc timer cb
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_set_time_cb(TKL_VENC_TIME_CB cb)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}


/**
 * @brief video encode request IDR frame
 *
 * @param[in] vi_chn: vi channel number
 * @param[in] venc_chn: venc channel number
 * @param[in] idr_type: request idr type
 *
 * @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
 */
OPERATE_RET tkl_venc_set_IDR(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_IDR_E idr_type)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video settings format
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[in] pformat: format config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_set_format(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_FORMAT_T *pformat)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief video get format
* 
* @param[in] vi_chn: vi channel number
* @param[in] venc_chn: venc channel number
* @param[out] pformat: format config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_venc_get_format(TKL_VI_CHN_E vi_chn, TKL_VENC_CHN_E venc_chn, TKL_VENC_FORMAT_T *pformat)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}
