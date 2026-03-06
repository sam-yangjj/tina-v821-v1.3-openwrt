#include <stdio.h>
#include "tkl_video_in.h"
#include "tuya_error_code.h"
#include "tkl_video_enc.h"
#include "plat_log.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/resource.h>

#define LOG_TAG "demo_video"

#include "AW_VideoInput_API.h"
#include "aw_util.h"

#include <ion_mem_alloc.h>
#include "rt_aiservice_pdet.h"
#include "rt_de_hw_scale.h"

#include "rt_motion_pdet.h"
#include "rt_motion_detect_common.h"

#include "tkl_gpio.h"

#define JPEG_WIDTH   (320)
#define JPEG_HEIGHT  (180)

#define PDET_INPUT_W (320)
#define PDET_INPUT_H (192)
#define PDET_INPUT_C (3)

#define ENABLE_OSD_FUNCTION   (0)
#define ENABLE_CATCH_JPEG     (1)

#define ENABLE_GET_BIN_IMAGE_DATA (0)
#define ENABLE_GET_MV_INFO_DATA   (0)
#define ENABLE_GET_LV_SET_IR      (0)

#define TESET_ROI_FUNC 	  0

#define ENABLE_SET_HFLIP (0)
#define ENABLE_SET_VFLIP (0)
#define ENABLE_SET_PLF   (0)

#define ENABLE_SET_I_FRAME (0)
#define ENABLE_RESET_ENCODER_TYPE (0)
#define ENABLE_RESET_SIZE (0)

#define ENABLE_SET_VBR_PARAM (0)
#define ENABLE_GET_SUM_MAD (0)

#define ENABLE_GET_MOTION_SEARCH_RESULT 1
//#define MOTION_SEARCH_DYNAMIC_ADJUST_UPDATE_INTV

#define ENABLE_DYNAMIC_SET_QP_AND_BITRATE_AND_FPS (0)
#define ENABLE_SET_SUPER_FRAME_PARAM (0)

#define DROP_STREAM_DATA (0)

#define TEST_CROP_FUNC 0

#define TEST_OUTSIDE_YUV 0

#define TEST_INSERT_P_SKIP 0
#define TEST_INSERT_NULL_FRAME 0

#define TEST_DROP_FRAME (0)

#define OUT_PUT_FILE_PREFIX "/tmp/stream"

#define OUT_PUT_FILE_2 "/mnt/extsd/data.yuv"
#define OUT_PUT_FILE_3 "/mnt/extsd/stream0.h265"
#define OUT_PUT_FILE_JPEG "/tmp/data.jpg"
#define OUT_PUT_FILE_BIN_IMAGE "/mnt/extsd/bin_image.data"
#define OUT_PUT_FILE_MV_INFO   "/mnt/extsd/mv_info.data"

#define IN_OSD_ARGB_FILE "/mnt/extsd/01_argb_464x32_time.dat"
#define IN_OSD_ARGB_FILE_1 "/mnt/extsd/03_argb_864x160_time.argb"

#define ENABLE_ISP_REG_GET	(0)
#define ISP_REG_SAVE_FILE 	"/tmp/isp_reg_save.h"

#define ISP_RGB_BIN_PATH	"/etc/ISP/RGB"
#define ISP_IR_BIN_PATH 	"/etc/ISP/IR"

//#define ALIGN_XXB(y, x) (((x) + ((y)-1)) & ~((y)-1))


#define ENABLE_TEST_SET_CAMERA_MOVE_STATUS  (0)
#define ENABLE_TEST_SET_CAMERA_ADAPTIVE_MOVING_AND_STATIC (0)
// #define PDET_SAVE_DEBUG

// #define ENABLE_PDET     (1)
// #define SAVE_STREAM_TF

static int max_bitstream_count = SAVE_BITSTREAM_COUNT;
static int64_t time_start = 0;
static int64_t time_end = 0;
static int64_t time_aw_start = 0;

static int video_finish_flag = 0;
static int channelId_0 = 0;
static int channelId_1 = 4;
static int channelId_2 = 4;
static int motionAlarm_sensitivity = 30;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char* argb_addr;
    unsigned int size;
}BitMapInfoS;

#define MAX_BITMAP_NUM (8)
BitMapInfoS bit_map_info[MAX_BITMAP_NUM];

static demo_video_param mparam;

#define TUYA_VIDEO_MAIN_CHANNEL 0
#define TUYA_VIDEO_SUB_CHANNEL 1
extern void tuya_video_stream_cb(TKL_VENC_FRAME_T *frame, int channel);
static unsigned int media_main_frame_size = 0;
static unsigned int media_sub_frame_size = 0;
static TKL_VENC_FRAME_T s_video_main_frame;
static TKL_VENC_FRAME_T s_video_main2nd_frame;
static char PersonDetectExitFlag;
static char save_day_night_mod = -1;
static unsigned char first_init_flag = 0;
extern float fDBThreshold;

MotionPdetParam *pMotionPdetParam = NULL;
FILE *out_file_0 = NULL;

static int runPdet(MotionPdetParam *pMotionPdetParam)
{
    int channel = channelId_2;
    SamplePdetInfo *pPdetInfo = pMotionPdetParam->pPdetInfo;
    SampleDewbInfo *pDewbInfo = pMotionPdetParam->pDewbInfo;
    MotionDetectRectInfo *pMotionLastData = &pMotionPdetParam->mMotionLastData;

    int ret = 0;
    VideoYuvFrame mYuvFrame;
    // unsigned int pdet_run_intv = PDET_RUN_INTV;
    // static int pdet_cnt = 0;

    alogd("run Pdet begin!");
#ifdef PDET_SAVE_DEBUG
    FILE *fp_pd = fopen("/mnt/sdcard/per.yuv", "wb");
#endif
    while(!PersonDetectExitFlag)
    {
        if (AWVideoInput_Check_Wait_Start(channel)) {
            usleep(200*1000);
            alogd("wait start");
            continue;
        }

        memset(&mYuvFrame, 0, sizeof(VideoYuvFrame));
        ret = AWVideoInput_GetYuvFrame(channel, &mYuvFrame);
        if (ret != 0) {
            aloge("get frame fail ret[%d]", ret);
            continue;
        }

        // if(0 == cnt % pdet_run_intv)
        {
            /* DE use VIN dma_buf */
            pDewbInfo->mInputInfo.mIonFd = mYuvFrame.fd;

            ret = motion_pdet_prepare(pMotionPdetParam, mYuvFrame.virAddr[0]);
            if(ret < 0)
            {
                aloge("fatal error! motion_pdet_prepare fail!");
                AWVideoInput_ReleaseYuvFrame(channel, &mYuvFrame);
                continue;
            }
#ifdef PDET_SAVE_DEBUG
            if (fp_pd) 
            {
                if (mYuvFrame.virAddr[0])
                    fwrite(mYuvFrame.virAddr[0], 1, mYuvFrame.widht*mYuvFrame.height, fp_pd);
                if (mYuvFrame.virAddr[1])
                    fwrite(mYuvFrame.virAddr[1], 1, mYuvFrame.widht*mYuvFrame.height / 2, fp_pd);
            }
#endif
            AWVideoInput_ReleaseYuvFrame(channel, &mYuvFrame);

            motion_pdet_run(pMotionPdetParam, pMotionLastData);

            // if (pdet_cnt == 0) {
            // 	kernel_fwrite("dmesg: motion_pdet_run first time! \n");
            // 	time_pdet_end = get_cur_time_us();
            // 	aw_logw("time of first motion_pdet_run: %lld.%lldms",  time_pdet_end/1000, time_pdet_end%1000);
            // 	pdet_cnt++;
            // }
        }
        // else
        // {
        // 	AWVideoInput_ReleaseYuvFrame(channel, &mYuvFrame);
        // }
        usleep(1000*1000); //adjust interval

    }
#ifdef PDET_SAVE_DEBUG
    if(fp_pd != NULL) {
        fclose(fp_pd);
        fp_pd = NULL;
    }
#endif

    return 0;
}

int getJpegStream(TKL_VENC_FRAME_T *pframe)
{
    alogd("enter");
    int ret = 0;

    static catch_jpeg_config jpg_config;
    memset(&jpg_config, 0, sizeof(catch_jpeg_config));
    jpg_config.channel_id = channelId_1;
    jpg_config.width = 640;
    jpg_config.height = 360;
    jpg_config.qp = 80;

    int cur_len = pframe->buf_size;
    AWVideoInput_CatchJpegConfig(&jpg_config);
    ret = AWVideoInput_CatchJpeg(pframe->pbuf, &cur_len, jpg_config.channel_id);
    alogd("cur_len: %d", cur_len);
    pframe->used_size = cur_len;

    if(ret != 0) {
        aloge("channel %d catch jpeg failed\n", jpg_config.channel_id);
        return -1;
    }
    return 0;
}

void video_stream_cb(const AWVideoInput_StreamInfo* stream_info)
{
    int keyframe = stream_info->keyframe_flag;
    int buf_size = s_video_main_frame.buf_size;

    if (stream_info->size0 == 0 || stream_info->data0 == NULL) {
        alogd("stream data error: data = %p, len = %d",stream_info->data0, stream_info->size0);
        return ;
    }
    if (video_finish_flag == 1) {
        return ;
    }

    int len = 0;

    memset(s_video_main_frame.pbuf, 0, s_video_main_frame.buf_size);
    if (keyframe == 1 && stream_info->b_insert_sps_pps && stream_info->sps_pps_size && stream_info->sps_pps_buf) {
        if (buf_size >= (len + stream_info->sps_pps_size)) {
            memcpy(s_video_main_frame.pbuf + len, stream_info->sps_pps_buf, stream_info->sps_pps_size);
            len += stream_info->sps_pps_size;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->sps_pps_size);
        }
    }
    if (stream_info->size0) {
        if (buf_size >= (len + stream_info->size0)) {
            memcpy(s_video_main_frame.pbuf + len, stream_info->data0, stream_info->size0);
            len += stream_info->size0;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size0);
        }
    }
    if (stream_info->size1) {
        if (buf_size >= (len + stream_info->size1)) {
            memcpy(s_video_main_frame.pbuf + len, stream_info->data1, stream_info->size1);
            len += stream_info->size1;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size1);
        }
    }
    if (stream_info->size2) {
        if (buf_size >= (len + stream_info->size2)) {
            memcpy(s_video_main_frame.pbuf + len, stream_info->data2, stream_info->size2);
            len += stream_info->size2;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size2);
        }
    }

    if (len > 0) {
        s_video_main_frame.used_size = len;
        s_video_main_frame.frametype = (1 == keyframe) ? TKL_VIDEO_I_FRAME : TKL_VIDEO_PB_FRAME;
        s_video_main_frame.pts = stream_info->pts;
        tuya_video_stream_cb(&s_video_main_frame, TUYA_VIDEO_MAIN_CHANNEL);

#ifdef SAVE_STREAM_TF
        if(out_file_0 == NULL) {
            out_file_0 = fopen("/tmp/stream0_tuya.h264", "wb");
        } else {
            fwrite(s_video_main_frame.pbuf, len, 1, out_file_0);
        }
#endif
    }


    return;
}

void video_stream_cb_1(const AWVideoInput_StreamInfo* stream_info)
{
    int keyframe = stream_info->keyframe_flag;
    int buf_size = s_video_main2nd_frame.buf_size;

    if (stream_info->size0 == 0 || stream_info->data0 == NULL) {
        alogd("stream data error: data = %p, len = %d",stream_info->data0, stream_info->size0);
        return ;
    }
    if (video_finish_flag == 1) {
        return ;
    }

    int len = 0;

    memset(s_video_main2nd_frame.pbuf, 0, s_video_main2nd_frame.buf_size);
    if (keyframe == 1 && stream_info->b_insert_sps_pps && stream_info->sps_pps_size && stream_info->sps_pps_buf) {
        if (buf_size >= (len + stream_info->sps_pps_size)) {
            memcpy(s_video_main2nd_frame.pbuf + len, stream_info->sps_pps_buf, stream_info->sps_pps_size);
            len += stream_info->sps_pps_size;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->sps_pps_size);
        }
    }
    if (stream_info->size0) {
        if (buf_size >= (len + stream_info->size0)) {
            memcpy(s_video_main2nd_frame.pbuf + len, stream_info->data0, stream_info->size0);
            len += stream_info->size0;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size0);
        }
    }
    if (stream_info->size1) {
        if (buf_size >= (len + stream_info->size1)) {
            memcpy(s_video_main2nd_frame.pbuf + len, stream_info->data1, stream_info->size1);
            len += stream_info->size1;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size1);
        }
    }
    if (stream_info->size2) {
        if (buf_size >= (len + stream_info->size2)) {
            memcpy(s_video_main2nd_frame.pbuf + len, stream_info->data2, stream_info->size2);
            len += stream_info->size2;
        } else {
            alogw("fatal error! stream buf size %d is too small, len=%d !", buf_size, len + stream_info->size2);
        }
    }

    if (len > 0) {
        s_video_main2nd_frame.used_size = len;
        s_video_main2nd_frame.frametype = (1 == keyframe) ? TKL_VIDEO_I_FRAME : TKL_VIDEO_PB_FRAME;
        s_video_main2nd_frame.pts = stream_info->pts;
        tuya_video_stream_cb(&s_video_main2nd_frame, TUYA_VIDEO_SUB_CHANNEL);
    }


    return;
}

void channel_thread_exit(int channel)
{
    // sem_post(&finish_sem);
}

static int ChnCbImpl_AWVideoInputCallback(void *pAppData, int channel, AWVideoInput_EventType event, int nData1,
    int nData2, void *pEventData)
{
    int result = 0;
    switch(event)
    {
        case AWVideoInput_Event_DropFrame:
        {
            int nDropNum = nData1;
            alogd("recorder[%d] receives dropFrameNum[%d] event, need control other recorder to drop frame!", channel, nDropNum);
            if (mparam.en_second_channel)
            {
                result = AWVideoInput_DropFrame(mparam.use_vipp_num_1, nDropNum);
                if (result != 0)
                {
                    aloge("fatal error! second channel[%d] drop [%d]frame fail[%d]", mparam.use_vipp_num_1, nDropNum, result);
                }
            }
            else
            {
                result = 0;
            }
            break;
        }
        case AWVideoInput_Event_StreamReady:
        {
            alogd("need implement to replace Video_Input_cb");
            break;
        }
        case AWVideoInput_Event_WaitErrorExit:
        {
            alogd("need implement to replace Channel_Thread_exit");
            break;
        }
        default:
        {
            aloge("fatal error! unknown event:%d", event);
            result = -1;
            break;
        }

    }
    return result;
}

static void init_2d_param(s2DfilterParam *p2DfilterParam)
{
    //init 2dfilter(2dnr) is open
    p2DfilterParam->enable_2d_filter = 1;
    p2DfilterParam->filter_strength_y = 127;
    p2DfilterParam->filter_strength_uv = 128;
    p2DfilterParam->filter_th_y = 11;
    p2DfilterParam->filter_th_uv = 7;
    return ;
}

#if ENABLE_GET_MOTION_SEARCH_RESULT
static int motion_search_thread(void* param)
{
    if (mparam.c0_encoder_format == 1)
        return -1;
    usleep(2*1000*1000);
    int channel_id = *(int *)param;
    int ret = 0;
    int i = 0;
    int motion_count = 0;
    int motion_status = 0;
    int count = 0;
    if (mparam.enable_motion_search_use_default_cfg) {
        AWVideoInput_GetMotionSearchParam(channel_id, &mparam.motion_search_param);
        count = mparam.motion_search_param.hor_region_num * mparam.motion_search_param.ver_region_num;
        alogd("get from velib hor_region_num = %d, ver_region_num = %d", mparam.motion_search_param.hor_region_num, mparam.motion_search_param.ver_region_num);
    } else {
        count = MOTION_SEARCH_TOTAL_NUM;
    }
    int result_len = count*sizeof(VencMotionSearchRegion);
    VencMotionSearchRegion *pMotionSearchResult = (VencMotionSearchRegion *)malloc(result_len);
    if (pMotionSearchResult == NULL) {
        aloge("fatal error, malloc pMotionSearchResult failed! len=%d", result_len);
        return -1;
    }
    memset(pMotionSearchResult, 0, result_len);
    alogd("result_len = %d", result_len);
    while (get_motion_detect_status() == true)
    {
        if (get_human_detect_status() == true) { //tuya
            usleep(2000*1000);
            continue;
        }

        VencMotionSearchResult sMotion_result;
        memset(&sMotion_result, 0, sizeof(VencMotionSearchResult));
        VencMotionSearchRegion* region = pMotionSearchResult;

        sMotion_result.region = region;
        // alogd("get motion search result = %d", j);
        ret = AWVideoInput_GetMotionSearchResult(channel_id, &sMotion_result);
        for(i = 0; i < count; i++)
        {
            if (region[i].is_motion)
            {
                // alogd("i = %d, totalN = %d, intraN = %d, largeMvN = %d, smallMvN = %d, zeroMvN = %d, largeSadN = %d, is_motion = %d, region:%d,%d,%d,%d",
                // 	i, region[i].total_num, region[i].intra_num,
                // 	region[i].large_mv_num, region[i].small_mv_num,
                // 	region[i].zero_mv_num, region[i].large_sad_num,
                // 	region[i].is_motion,
                // 	region[i].pix_x_bgn, region[i].pix_x_end,
                // 	region[i].pix_y_bgn, region[i].pix_y_end);
                motion_count++;
            }
        }
        if (motion_count > motionAlarm_sensitivity)
        {
            motion_status = 1;
            // alogd("mrst:%d>%d", motion_count, motionAlarm_sensitivity);
        }
        else
        {
            motion_status = 0;
        }
        extern VOID IPC_APP_set_motion_status(int status);
        IPC_APP_set_motion_status(motion_status);
        motion_count = 0;
        usleep(42*1000);
    }
    free(pMotionSearchResult);
    pMotionSearchResult = NULL;
    return 0;
}
#endif

static void init_wbyuv_param_info(sWbYuvParam *pWbYuvParam, WbYuvFuncInfo *pWbYuvFuncInfo)
{
    pWbYuvParam->bEnableWbYuv = 1;
    pWbYuvParam->nWbBufferNum = 2;
    pWbYuvParam->scalerRatio  = VENC_ISP_SCALER_0;
    pWbYuvParam->bEnableCrop  = 0;
    pWbYuvParam->sWbYuvcrop.nHeight  = 640;
    pWbYuvParam->sWbYuvcrop.nWidth  = 640;

    pWbYuvFuncInfo->enable_wbyuv = 1;
    pWbYuvFuncInfo->get_num = 5;
    return ;
}

static void setParamBeforeStart(const demo_video_param* pmparam, int channel_id)
{
    if (pmparam->rotate_angle != 0)
        AWVideoInput_SetRotate(channel_id, pmparam->rotate_angle);

    if (pmparam->enable_gray)
        AWVideoInput_SetChmoraGray(channel_id, pmparam->enable_gray);

    if (pmparam->enable_wbyuv) {
        WbYuvFuncInfo *pWbYuvFuncInfo = NULL;
        sWbYuvParam mWbYuvParam;

        pWbYuvFuncInfo = calloc(1, sizeof(WbYuvFuncInfo));
        memset(&mWbYuvParam, 0, sizeof(sWbYuvParam));

        init_wbyuv_param_info(&mWbYuvParam, pWbYuvFuncInfo);
        if (pmparam->c0_dst_w && pmparam->c0_dst_h)
            AWVideoInput_SetWbYuv(channel_id, pWbYuvFuncInfo, &mWbYuvParam, pmparam->c0_dst_w, pmparam->c0_dst_h);
        else
            AWVideoInput_SetWbYuv(channel_id, pWbYuvFuncInfo, &mWbYuvParam, pmparam->c0_src_w, pmparam->c0_src_h);
    }

    int nRegionLinkEn = 1, nRegionLinkTexDetectEn = 1, nRegionLinkMotionDetectEn = 1;
    sRegionLinkParam region_link_param;
    memset(&region_link_param, 0, sizeof(sRegionLinkParam));
    AWVideoInput_GetRegionLink(channel_id, &region_link_param);

    if (region_link_param.mStaticParam.region_link_en != nRegionLinkEn)
        region_link_param.mStaticParam.region_link_en = nRegionLinkEn;
    if (region_link_param.mStaticParam.tex_detect_en != nRegionLinkTexDetectEn)
        region_link_param.mStaticParam.tex_detect_en = nRegionLinkTexDetectEn;
    if (region_link_param.mStaticParam.motion_detect_en != nRegionLinkMotionDetectEn)
        region_link_param.mStaticParam.motion_detect_en = nRegionLinkMotionDetectEn;

    AWVideoInput_SetRegionLink(channel_id, &region_link_param);

    return ;
}

static int setParam(demo_video_param* mparam, int channel_id, VideoInputConfig* pConfig)
{
    int ret = 0;
    if (mparam->enable_crop) {
        VencCropCfg rt_crop_info;
        rt_crop_info.bEnable = 1;
        rt_crop_info.Rect.nLeft = 80;
        rt_crop_info.Rect.nTop = 64;
        rt_crop_info.Rect.nWidth = 640;
        rt_crop_info.Rect.nHeight = 480;
        AWVideoInput_SetCrop(channel_id, &rt_crop_info);
        alogd("awChn[%d] set crop:%d-%d-%dx%d", channel_id, rt_crop_info.Rect.nLeft, rt_crop_info.Rect.nTop,
            rt_crop_info.Rect.nWidth, rt_crop_info.Rect.nHeight);
    #if TEST_CROP_FUNC
        usleep(1000*1000);
        rt_crop_info.bEnable = 1;
        rt_crop_info.Rect.nLeft = 80;
        rt_crop_info.Rect.nTop = 64;
        rt_crop_info.Rect.nWidth = 560;
        rt_crop_info.Rect.nHeight = 320;
        AWVideoInput_SetCrop(channel_id, &rt_crop_info);
        alogd("awChn[%d] set crop:%d-%d-%dx%d", channel_id, rt_crop_info.Rect.nLeft, rt_crop_info.Rect.nTop,
            rt_crop_info.Rect.nWidth, rt_crop_info.Rect.nHeight);
        usleep(1000*1000);
        rt_crop_info.bEnable = 1;
        rt_crop_info.Rect.nLeft = 80;
        rt_crop_info.Rect.nTop = 64;
        rt_crop_info.Rect.nWidth = 480;
        rt_crop_info.Rect.nHeight = 272;
        AWVideoInput_SetCrop(channel_id, &rt_crop_info);
        alogd("awChn[%d] set crop:%d-%d-%dx%d", channel_id, rt_crop_info.Rect.nLeft, rt_crop_info.Rect.nTop,
            rt_crop_info.Rect.nWidth, rt_crop_info.Rect.nHeight);
        usleep(1000*1000);
        rt_crop_info.bEnable = 0;
        AWVideoInput_SetCrop(channel_id, &rt_crop_info);
		alogd("awChn[%d] disable crop", channel_id);
    #endif
    }

    // if (mparam->en_2d_nr) {
    //     s2DfilterParam m2DfilterParam;
    //     init_2d_param(&m2DfilterParam);
    //     AWVideoInput_Set2dNR(channel_id, &m2DfilterParam);
    // }

    // if (mparam->en_3d_nr) {
    //     AWVideoInput_Set3dNR(channel_id, mparam->en_3d_nr);
    // }

    /*close 2d 3d nr*/
    s2DfilterParam m2DfilterParam;
    m2DfilterParam.enable_2d_filter = 0;
    AWVideoInput_Set2dNR(channel_id, &m2DfilterParam);
    AWVideoInput_Set3dNR(channel_id, 0);
    AWVideoInput_Set2dNR(channelId_1, &m2DfilterParam);
    AWVideoInput_Set3dNR(channelId_1, 0);

    if (mparam->enable_p_intra_refresh) {
        VencCyclicIntraRefresh sIntraRefresh;
        sIntraRefresh.bEnable = 1;
        sIntraRefresh.nBlockNumber = 10;
        AWVideoInput_SetPIntraRefresh(channel_id, &sIntraRefresh);
    }

#if ENABLE_GET_MOTION_SEARCH_RESULT
    if (mparam->enable_motion_search) {
        if (pConfig->encodeType == 0 || pConfig->encodeType == 2) {
            // init_motion_search_param(&mparam->motion_search_param, mparam->enable_motion_search_use_default_cfg);
	        memset(&mparam->motion_search_param, 0, sizeof(VencMotionSearchParam));
            mparam->motion_search_param.en_motion_search = 1;
            mparam->motion_search_param.update_interval = 3;
            mparam->motion_search_param.dis_default_para = 0;
            AWVideoInput_SetMotionSearchParam(channel_id, &mparam->motion_search_param);
        }
    }
#endif
#if TESET_ROI_FUNC
    VencROIConfig   stRoiConfigs[MAX_ROI_AREA];
    init_roi(stRoiConfigs);
    AWVideoInput_SetRoi(channel_id, stRoiConfigs);
#endif
#if ENABLE_RESET_ENCODER_TYPE
    AWVideoInput_Start(0, 0);
    AWVideoInput_ResetEncoderType(0, 2);

    fclose(out_file_0);
    out_file_0 = fopen(OUT_PUT_FILE_3, "wb");
    alogd("reset encodertype -- reopen : %p, path = %s", out_file_0, OUT_PUT_FILE_3);

    AWVideoInput_Start(0, 1);
#endif

#if ENABLE_GET_BIN_IMAGE_DATA
    get_bin_image_data(channel_id, pConfig);
#endif

#if ENABLE_GET_MV_INFO_DATA
    if(pConfig->encodeType == 0)//* h264
        get_mv_info_data_h264(channel_id, pConfig);
    else if(pConfig->encodeType == 2)//* h265
        get_mv_info_data_h265(channel_id, pConfig);
#endif

#if ENABLE_SET_HFLIP
    usleep(1000*1000);
    alogd("set h flip, channelId = %d\n", channel_id);
    AWVideoInput_SetHFlip(channel_id, 1);
#endif

#if ENABLE_SET_VFLIP
    usleep(1000*1000);
    alogd("set v flip, channelId = %d\n", channel_id);
    AWVideoInput_SetVFlip(channel_id, 1);
#endif

#if ENABLE_SET_PLF
    AWVideoInput_SetPowerLineFreq(channel_id, RT_FREQUENCY_50HZ);
#endif

#if ENABLE_DYNAMIC_SET_QP_AND_BITRATE_AND_FPS
    VencQPRange mqp_range;
    mqp_range.nMinqp = 35;
    mqp_range.nMaxqp = 51;
    mqp_range.nMinPqp = 35;
    mqp_range.nMaxPqp = 51;
    mqp_range.nQpInit = 35;
    mqp_range.bEnMbQpLimit = 0;

    AWVideoInput_SetQpRange(channel_id, &mqp_range);
    AWVideoInput_SetBitrate(channel_id, 1536*1024);
    AWVideoInput_SetFps(channel_id, 13);

    memset(&mqp_range, 0, sizeof(VencQPRange));
    AWVideoInput_GetQpRange(channel_id, &mqp_range);
    int bitrate = AWVideoInput_GetBitrate(channel_id);
    int fps = AWVideoInput_GetFps(channel_id);
    alogd("get info: i_min&max_qp = %d, %d, p_min&max_qp = %d, %d, bitrate = %d, fps = %d",
        mqp_range.nMinqp, mqp_range.nMaxqp, mqp_range.nMinPqp, mqp_range.nMaxPqp,
        bitrate, fps);

    usleep(3*1000*1000);
    AWVideoInput_SetFps(channel_id, 15);
#endif


#if ENABLE_OSD_FUNCTION
    VideoInputOSD OverlayInfo;
    int index = 0;
    memset(&OverlayInfo, 0, sizeof(VideoInputOSD));

    OverlayInfo.osd_num = 3;

    OverlayInfo.item_info[index].osd_type = NORMAL_OVERLAY;
    OverlayInfo.item_info[index].start_x = 16;
    OverlayInfo.item_info[index].start_y = 752;
    OverlayInfo.item_info[index].widht  = 464;
    OverlayInfo.item_info[index].height = 32;
    ret = init_overlay_info(&OverlayInfo, index);

    index = 1;
    OverlayInfo.item_info[index].osd_type = LUMA_REVERSE_OVERLAY;
    OverlayInfo.item_info[index].start_x = 480;
    OverlayInfo.item_info[index].start_y = 784;
    OverlayInfo.item_info[index].widht  = 464;
    OverlayInfo.item_info[index].height = 32;
    OverlayInfo.invert_mode = 3;
    OverlayInfo.invert_threshold = 90;
    ret = init_overlay_info(&OverlayInfo, index);

    index = 2;
    OverlayInfo.item_info[index].osd_type = COVER_OVERLAY;
    OverlayInfo.item_info[index].start_x = 800;
    OverlayInfo.item_info[index].start_y = 512;
    OverlayInfo.item_info[index].widht  = 464;
    OverlayInfo.item_info[index].height = 32;
    OverlayInfo.item_info[index].cover_yuv.cover_y = 0;
    OverlayInfo.item_info[index].cover_yuv.cover_u = 0;
    OverlayInfo.item_info[index].cover_yuv.cover_v = 0x64;
    ret |= init_overlay_info(&OverlayInfo, index);

    if(ret == 0)
        AWVideoInput_SetOSD(channel_id, &OverlayInfo);

    usleep(2*1000*1000);
    //AWVideoInput_SetOSD(channel_id, &OverlayInfo);

    #if 0
    unsigned int g2d_data_size = 464*32*4;
    unsigned char* g2d_data = malloc(g2d_data_size);
    AWVideoInput_GetG2DData(channelId_0, g2d_data);

    FILE *g2d_file = NULL;
    g2d_file = fopen("/tmp/g2d.dat", "wb");
    fwrite(g2d_data, 1, g2d_data_size, g2d_file);
    usleep(200*1000);
    fclose(g2d_file);
    #endif

#endif


#if ENABLE_RESET_SIZE
    reset_size(channel_id, &OverlayInfo);
#endif

    if(mparam->encode_local_yuv)
    {
        FILE * in_yuv_fp = fopen(mparam->InputFileName, "rb");
        alogd("fopen %s w&h = %d, %d", mparam->InputFileName, pConfig->width, pConfig->height);
        if(in_yuv_fp == NULL)
        {
            aloge("fopen failed: %s", mparam->InputFileName);
        }
        else
        {
            int yuv_size = pConfig->width*pConfig->height*3/2;
            VideoYuvFrame mYuvFrame;

            yuv_size = rt_cal_input_buffer_size(pConfig->width, pConfig->height, pConfig->pixelformat, 1);

            alogd("* yuv_size = %d, w&h = %d, %d", yuv_size, pConfig->width, pConfig->height);

            while (1)
            {
                if(video_finish_flag == 1)
                    break;

                memset(&mYuvFrame, 0, sizeof(VideoYuvFrame));
                ret = AWVideoInput_RequestEmptyYuvFrame(channel_id, &mYuvFrame);
                if(ret == 0)
                {
                    ret = fread(mYuvFrame.virAddr[0], 1, yuv_size, in_yuv_fp);
                    alogd("vir_addr = %p, ret = %d, yuv_size = %d",
                            mYuvFrame.virAddr[0], ret, yuv_size);

                    if(ret != yuv_size)
                    {
                        fseek(in_yuv_fp, 0, SEEK_SET);
                        ret = fread(mYuvFrame.virAddr[0], 1, yuv_size, in_yuv_fp);
                    }
                    AWVideoInput_SubmitFilledYuvFrame(channel_id, &mYuvFrame, yuv_size);
                }
                else
                    usleep(50*1000);

            }

            fclose(in_yuv_fp);
        }
    }

#if ENABLE_SET_VBR_PARAM
    VencVbrParam mVbrParam;
    memset(&mVbrParam, 0, sizeof(VencVbrParam));
    mVbrParam.uMaxBitRate = 2048*1024;
    mVbrParam.nMovingTh = 20;
    mVbrParam.nQuality = 10;
    mVbrParam.nIFrmBitsCoef = 10;
    mVbrParam.nPFrmBitsCoef = 10;

    AWVideoInput_SetVbrParam(channel_id, &mVbrParam);

    memset(&mVbrParam, 0, sizeof(VencVbrParam));

    AWVideoInput_GetVbrParam(channel_id, &mVbrParam);
    alogd("get vbr param: MaxBitRate = %d, MovingTh = %d, quality = %d, I&PFrmBitsCoef = %d, %d",
        mVbrParam.uMaxBitRate, mVbrParam.nMovingTh,
        mVbrParam.nQuality, mVbrParam.nIFrmBitsCoef, mVbrParam.nPFrmBitsCoef);
#endif

#if ENABLE_GET_SUM_MAD
    usleep(2*1000*1000);
    int sum_mad = AWVideoInput_GetSumMad(channel_id);
    alogd("sum_mad = %d",sum_mad);
#endif

#if ENABLE_GET_LV_SET_IR
    ret = AWVideoInput_GetLuminance(channel_id);
    alogd("the lv is : %d\n", ret);
    RTIrParam mIrParam;
    memset(&mIrParam, 0, sizeof(RTIrParam));
    mIrParam.grey = 1;
    mIrParam.ir_on = 1;
    mIrParam.ir_flash_on = 0;

    AWVideoInput_SetIrParam(channel_id, &mIrParam);
#endif

#if ENABLE_SET_SUPER_FRAME_PARAM
    VencSuperFrameConfig mSuperConfig;

    memset(&mSuperConfig, 0, sizeof(VencSuperFrameConfig));
    mSuperConfig.eSuperFrameMode = VENC_SUPERFRAME_NONE;
    mSuperConfig.nMaxRencodeTimes = 0;
    mSuperConfig.nMaxIFrameBits  = 200*1024*8; //* 200 KB
    mSuperConfig.nMaxPFrameBits  = mSuperConfig.nMaxIFrameBits / 3;
    mSuperConfig.nMaxP2IFrameBitsRatio = 0.33;

    AWVideoInput_SetSuperFrameParam(channel_id, &mSuperConfig);

#endif

    return 0;
}


static void exit_demo()
{
    int i = 0;
    AWVideoInput_DeInit();
    alogd("exit: deInit end\n");

    // sem_destroy(&finish_sem);

    for(i = 0; i < MAX_BITMAP_NUM; i++)
    {
        if(bit_map_info[i].argb_addr)
        {
            free(bit_map_info[i].argb_addr);
            bit_map_info[i].argb_addr = NULL;
        }
    }

#ifdef SAVE_STREAM_TF
    if(out_file_0)
    {
        fclose(out_file_0);
        out_file_0 = NULL;
    }
#endif

    // if(out_file_1)
    // {
    // 	fclose(out_file_1);
    // 	out_file_1 = NULL;
    // }

    alogd("aw_demo, finish!\n");
    return ;
}

static void init_color_space(VencH264VideoSignal *pvenc_video_signal, int eColorSpace)
{
    pvenc_video_signal->video_format = DEFAULT;
    switch(eColorSpace)
    {
        case V4L2_COLORSPACE_JPEG:
        {
            pvenc_video_signal->full_range_flag = 1;
            pvenc_video_signal->src_colour_primaries = VENC_YCC;
            pvenc_video_signal->dst_colour_primaries = VENC_YCC;
            break;
        }
        case V4L2_COLORSPACE_REC709:
        {
            pvenc_video_signal->full_range_flag = 1;
            pvenc_video_signal->src_colour_primaries = VENC_BT709;
            pvenc_video_signal->dst_colour_primaries = VENC_BT709;
            break;
        }
        case V4L2_COLORSPACE_REC709_PART_RANGE:
        {
            pvenc_video_signal->full_range_flag = 0;
            pvenc_video_signal->src_colour_primaries = VENC_BT709;
            pvenc_video_signal->dst_colour_primaries = VENC_BT709;
            break;
        }
        default:
        {
            pvenc_video_signal->full_range_flag = 1;
            pvenc_video_signal->src_colour_primaries = VENC_BT709;
            pvenc_video_signal->dst_colour_primaries = VENC_BT709;
            break;
        }
    }
}

static int create_yuv_for_test_dma_overlay(struct dma_overlay_para *dma_overlay_ctx)
{
    int i;
    int ySize;
    int uvSize;

    if (!dma_overlay_ctx) {
        aloge("dma_overlay_ctx is NULL!\n");
        return -1;
    }

    alogd("ready to do create_yuv_for_test_dma_overlay\n");
    ySize = dma_overlay_ctx->overlay_width * dma_overlay_ctx->overlay_height;
    uvSize = ySize >> 1;
    /* set gray for image */
    memset(dma_overlay_ctx->overlay_data, 128, dma_overlay_ctx->length);

    return 0;
}





/**
* @brief vi init
* 
* @param[in] pconfig: vi config
* @param[in] count: count of pconfig
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_init(TKL_VI_CONFIG_T *pconfig, INT32_T count)
{
    alogd("enter");
    int i = 0;
    int ret = 0;

    //解析成argv argc格式
    // char input[] = "demo_video_in -n 150 -s0 1920x1080 -ds0 2304x1296 -sfps 15 -dfps 15 -f0 0 -vn 0 -pf 12 -sp 1 -cs 3 -vb 4 -ms 1 -ms_def 1 -pdet 1";
    char input[] = "demo_video_in -s0 1920x1080 -ds0 2304x1296 -sfps 15 -dfps 15 -f0 0 -vn 0 -pf 12 -sp 1 -cs 3 -vb 4 -ms 1 -ms_def 1 -snd 1 -s1 640x360 -f1 0 -vn1 4 -pf1 1";
    char *argv[128];
    int argc = 0;

    char *token = strtok(input, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    printf("argc = %d\n", argc);
    // for (int i = 0; i < argc; i++) {
    // 	printf("argv[%d] = %s\n", i, argv[i]);
    // }

    //AWVideoInput_SetLogLevel(AWRT_LOG_LEVEL_DEBUG);

    VideoInputConfig config_1;
    memset(&config_1, 0, sizeof(VideoInputConfig));

    video_finish_flag = 0;

    memset(&mparam, 0, sizeof(demo_video_param));
    memset(bit_map_info, 0, MAX_BITMAP_NUM*sizeof(BitMapInfoS));
    mparam.c0_encoder_format = -1;
    mparam.c1_encoder_format = -1;
    mparam.pixelformat = RT_PIXEL_NUM;
    mparam.pixelformat_1 = RT_PIXEL_NUM;
    mparam.pixelformat_2 = RT_PIXEL_NUM;
    mparam.use_vipp_num = 0;
    mparam.use_vipp_num_1 = 4;
    mparam.use_vipp_num_2 = 4;
    mparam.jpg_width = JPEG_WIDTH;
    mparam.jpg_heigh = JPEG_HEIGHT;

    mparam.share_buf_num = 2;
    mparam.share_buf_num_1 = 2;
    mparam.enable_sharp = 0;
    mparam.enable_tdm_raw = MAIN_CHANNEL_TDM_RAW_ENABLE;
    mparam.tdm_rxbuf_cnt = MAIN_CHANNEL_TDM_RXBUF_CNT;
    strcpy(mparam.OutputFilePath, OUT_PUT_FILE_PREFIX);
    mparam.src_fps = MAIN_CHANNEL_FPS;
    mparam.product_mode = MAIN_CHANNEL_PRODUCT_MODE;
    /******** begin parse the config paramter ********/
    if(argc >= 2)
    {
        alogd("******************************\n");
        for(i = 1; i < (int)argc; i += 2)
        {
            ParseArgument(&mparam, argv[i], argv[i + 1]);
        }
        alogd("******************************\n");
    }
    else
    {
        alogd(" we need more arguments \n");
        PrintDemoUsage();
        //return 0;
    }

    check_param(&mparam);

    alogd("*demo param: c0: w&h = %d, %d; encode_format = %d, bitrate = %d",
            mparam.c0_src_w, mparam.c0_src_h, mparam.c0_encoder_format, mparam.c0_bitrate);
    alogd("*demo param: c1: w&h = %d, %d; encode_format = %d, bitrate = %d",
            mparam.c1_src_w, mparam.c1_src_h, mparam.c1_encoder_format, mparam.c1_bitrate);

    /* ********** */
    media_main_frame_size = mparam.c0_dst_w * mparam.c0_dst_h * 3 / 2 / 10;
    media_sub_frame_size = mparam.c1_src_w * mparam.c1_src_h * 3 / 2 / 10;
    alogd("main_frame_size:%d, sub_frame_size:%d", media_main_frame_size, media_sub_frame_size);

    UCHAR_T *frame_buff = NULL;
    frame_buff = (UCHAR_T *)malloc(media_main_frame_size);
    if (NULL == frame_buff) {
        aloge("malloc failed\n");
        return OPRT_COM_ERROR;
    }
    s_video_main_frame.pbuf = (char *)frame_buff;
    s_video_main_frame.buf_size = media_main_frame_size;

    UCHAR_T *frame_buff_2 = NULL;
    frame_buff_2 = (UCHAR_T *)malloc(media_sub_frame_size);
    if (NULL == frame_buff_2) {
        aloge("malloc failed\n");
        return OPRT_COM_ERROR;
    }
    s_video_main2nd_frame.pbuf = (char *)frame_buff_2;
    s_video_main2nd_frame.buf_size = media_sub_frame_size;
    /* ********** */

    time_start = get_cur_time_us();

    if(mparam.encoder_num > 0)
        max_bitstream_count = mparam.encoder_num;
    else
        max_bitstream_count = SAVE_BITSTREAM_COUNT;
    channelId_0 = mparam.use_vipp_num;
    channelId_1 = mparam.use_vipp_num_1;
    channelId_2 = mparam.use_vipp_num_2;
    VideoInputConfig config_0;
    memset(&config_0, 0, sizeof(VideoInputConfig));
    config_0.channelId = channelId_0;
    config_0.fps     = mparam.src_fps;
    config_0.dst_fps = mparam.dst_fps;
    config_0.gop     = MAIN_CHANNEL_GOP;
    config_0.product_mode = mparam.product_mode;
    config_0.mRcMode = MAIN_CHANNEL_RC_MODE;
    if (config_0.mRcMode == AW_VBR) {
        config_0.vbr_param.uMaxBitRate = mparam.c0_bitrate;
        config_0.vbr_param.nMovingTh = 20;
        config_0.vbr_param.nQuality = 10;
        config_0.vbr_param.nIFrmBitsCoef = 10;
        config_0.vbr_param.nPFrmBitsCoef = 10;
        config_0.vbr_opt_enable = 1;
    }
#if ENABLE_GET_MOTION_SEARCH_RESULT
    if (!(config_0.mRcMode == AW_VBR ||
        PRODUCT_STATIC_IPC == config_0.product_mode ||
        PRODUCT_MOVING_IPC == config_0.product_mode ||
        PRODUCT_DOORBELL == config_0.product_mode))
    {
        alogw("channel %d MOTION SEARCH only for VBR mode or IPC or DOORBELL, vbr=%d, product_mode=%d",
            channelId_0, config_0.mRcMode, config_0.product_mode);
    }
#endif
    config_0.qp_range.nMinqp = MAIN_CHANNEL_I_MIN_QP;
    config_0.qp_range.nMaxqp = MAIN_CHANNEL_I_MAX_QP;
    config_0.qp_range.nMinPqp = MAIN_CHANNEL_P_MIN_QP;
    config_0.qp_range.nMaxPqp = MAIN_CHANNEL_P_MAX_QP;
    config_0.qp_range.nQpInit = MAIN_CHANNEL_INIT_QP;
    config_0.profile = VENC_H264ProfileMain;
    config_0.level   = VENC_H264Level51;
    config_0.pixelformat = mparam.pixelformat;
    config_0.enable_sharp = mparam.enable_sharp;
    config_0.vin_buf_num = mparam.vin_buf_num;
    config_0.bonline_channel = mparam.bonline_channel;
    config_0.share_buf_num = mparam.share_buf_num;
    config_0.en_16_align_fill_data = 0;
    config_0.breduce_refrecmem = MAIN_CHANNEL_REDUCE_REFREC_MEM;
    config_0.enable_tdm_raw = mparam.enable_tdm_raw;
    config_0.tdm_rxbuf_cnt = mparam.tdm_rxbuf_cnt;
    alogd("ch%d enable_tdm_raw:%d, tdm_rxbuf_cnt:%d", config_0.channelId,
        config_0.enable_tdm_raw, config_0.tdm_rxbuf_cnt);
    init_color_space(&config_0.venc_video_signal, mparam.color_space);

    if (mparam.tdm_spdn_en) {
        alogd("ch%d enable_tdm_spdn_en, tdm_spdn_vaild_num:%d, tdm_spdn_invaild_num:%d", config_0.channelId,
                mparam.tdm_spdn_vaild_num,  mparam.tdm_spdn_invaild_num);
        config_0.tdm_spdn_cfg.tdm_speed_down_en = 1;
        config_0.tdm_spdn_cfg.tdm_tx_valid_num = mparam.tdm_spdn_vaild_num;
        config_0.tdm_spdn_cfg.tdm_tx_invalid_num = mparam.tdm_spdn_invaild_num;
    }

    if(mparam.encode_local_yuv)
    {
        config_0.qp_range.nMinqp = 20;
        config_0.qp_range.nMaxqp = 45;
        config_0.qp_range.nMinPqp = 20;
        config_0.qp_range.nMaxPqp = 45;
        config_0.qp_range.nQpInit = 20;
        config_0.output_mode = OUTPUT_MODE_ENCODE_FILE_YUV;

        mparam.en_second_channel = 0;
        mparam.en_third_channel = 0;
    }
    else
    {
        config_0.output_mode = OUTPUT_MODE_STREAM;
    }
    config_0.width      = mparam.c0_src_w;
    config_0.height     = mparam.c0_src_h;
    config_0.dst_width      = mparam.c0_dst_w;
    config_0.dst_height     = mparam.c0_dst_h;
    config_0.bitrate    = mparam.c0_bitrate;
    config_0.encodeType = mparam.c0_encoder_format;
    config_0.drop_frame_num = 0;
    config_0.enable_wdr = 0;
    config_0.dma_merge_mode = mparam.dma_merge_mode;
    alogd("mparam.dma_merge_mode = %d\n", mparam.dma_merge_mode);
    if (config_0.bonline_channel && (config_0.dma_merge_mode >= DMA_STITCH_HORIZONTAL)) {
        config_0.bEnableMultiOnlineSensor = 1;
        config_0.bEnableImageStitching = 1;
    }
    /* Only RTVI_DMA_STITCH_VERTICAL support insert dma overlay */
    if (config_0.dma_merge_mode == DMA_STITCH_VERTICAL) {
        if (mparam.dma_overlay_enable) {
            config_0.dma_overlay_ctx.dma_overlay_en = 1;
            if (mparam.dma_overlay_width == 0 || mparam.dma_overlay_height == 0) {
                alogd("mparam.dma_overlay: (%d, %d), set default 1920*336!\n", mparam.dma_overlay_width, mparam.dma_overlay_height);
                config_0.dma_overlay_ctx.overlay_width = 1920;
                config_0.dma_overlay_ctx.overlay_height = 336;
            } else {
                config_0.dma_overlay_ctx.overlay_width = mparam.dma_overlay_width;
                config_0.dma_overlay_ctx.overlay_height = mparam.dma_overlay_height;
            }
            alogd("It will set dma_overlay: (%d, %d)\n", config_0.dma_overlay_ctx.overlay_width,
                config_0.dma_overlay_ctx.overlay_height);
            config_0.dma_overlay_ctx.length = config_0.dma_overlay_ctx.overlay_width * config_0.dma_overlay_ctx.overlay_height
                * 3 / 2;
            config_0.dma_overlay_ctx.overlay_data = (unsigned char *)malloc(config_0.dma_overlay_ctx.length);
            if (config_0.dma_overlay_ctx.overlay_data) {
                create_yuv_for_test_dma_overlay(&config_0.dma_overlay_ctx);
            } else {
                aloge("overlay_data malloc failed!\n");
            }
        } else {
            alogw("It will not set dma overlay, please ensure that the resolution is correct\n");
        }
    }
    config_0.dma_merge_scaler_cfg.scaler_en = mparam.scaler_en;
    if (config_0.dma_merge_scaler_cfg.scaler_en) {
        config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.width = mparam.dma_merge_scaler_sensorA_width;
        config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.height = mparam.dma_merge_scaler_sensorA_height;
        config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.width = mparam.dma_merge_scaler_sensorB_width;
        config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.height = mparam.dma_merge_scaler_sensorB_height;
		alogd("channelId:%d, SensorA: %dx%d, SensorB: %dx%d\n", config_0.channelId, config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.width,
            config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.height, config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.width,
            config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.height);
    }

#if ENABLE_TEST_SET_CAMERA_MOVE_STATUS
    config_0.enable_isp2ve_linkage = 1;
    config_0.enable_ve2isp_linkage = 1;
#else
    config_0.enable_isp2ve_linkage = 1;
    config_0.enable_ve2isp_linkage = 1;
#endif
    if (config_0.encodeType == 1) {//jpg encode
        config_0.jpg_quality = 80;
        if (mparam.encoder_num > 1) {//mjpg
            config_0.jpg_mode = mparam.jpg_mode = 1;
            config_0.bitrate = 12*1024*1024;
            config_0.bit_rate_range.bitRateMin = 10*1024*1024;
            config_0.bit_rate_range.bitRateMax = 14*1024*1024;
        }
    }
#if ENABLE_GET_BIN_IMAGE_DATA
    config_0.enable_bin_image = 1;
    config_0.bin_image_moving_th = 20;
#endif

#if ENABLE_GET_MV_INFO_DATA
    config_0.enable_mv_info = 1;
#endif

    time_end = get_cur_time_us();
    //alogd("init start, time = %lld, max_count = %d\n",time_end - time_start, max_bitstream_count);

    AWVideoInput_Init();
    // sem_init(&finish_sem, 0, 0);
    int64_t time_end1 = get_cur_time_us();
    alogd("time of aw_init:  %lld\n",time_end1 - time_end);

    if(AWVideoInput_Configure(channelId_0, &config_0))
    {
        aloge("config err, exit!");
        goto _exit;
    }
    setParamBeforeStart(&mparam, channelId_0);
    int64_t time_end2 = get_cur_time_us();
    alogd("time of aw_config: = %lld\n",time_end2 - time_end1);

    AWVideoInput_CallBack(channelId_0, video_stream_cb, 1);
    AWVideoInput_SetChannelCallback(channelId_0, ChnCbImpl_AWVideoInputCallback, NULL);
    AWVideoInput_SetChannelThreadExitCb(channelId_0, channel_thread_exit);
    int64_t time_end3 = get_cur_time_us();
    //alogd("callback end, time = %lld\n",time_end3 - time_end2);
    ret = AWVideoInput_Start(channelId_0, 1);
    if (ret != 0)
    {
        aloge("fatal error! awChn[%d] start fail:%d", channelId_0, ret);
    }

    int64_t time_end4 = get_cur_time_us();
    alogd("time of aw_start: = %lld, total_time = %lld\n",
            time_end4 - time_end3,
            time_end4 - time_start);

    time_aw_start = time_end4;

    VideoChannelInfo mChannelInfo;
    memset(&mChannelInfo, 0, sizeof(VideoChannelInfo));

    AWVideoInput_GetChannelInfo(channelId_0, &mChannelInfo);
    alogd("state = %d, w&h = %d,%d, bitrate = %d bps, fps = %d, i_qp = %d~%d, p_qp = %d~%d",
            mChannelInfo.state,
            mChannelInfo.mConfig.width, mChannelInfo.mConfig.height,
            mChannelInfo.mConfig.bitrate, mChannelInfo.mConfig.fps,
            mChannelInfo.mConfig.qp_range.nMinqp, mChannelInfo.mConfig.qp_range.nMaxqp,
            mChannelInfo.mConfig.qp_range.nMinPqp, mChannelInfo.mConfig.qp_range.nMaxPqp);

    if (mparam.en_second_channel) {
        config_1.channelId = channelId_1;
        config_1.fps     = 15;
        config_1.gop     = 30;
        config_1.product_mode = 0;
        config_1.mRcMode     = 0;
        if (config_1.mRcMode == AW_VBR) {
            config_1.vbr_param.uMaxBitRate = mparam.c1_bitrate;
            config_1.vbr_param.nMovingTh = 20;
            config_1.vbr_param.nQuality = 10;
            config_1.vbr_param.nIFrmBitsCoef = 10;
            config_1.vbr_param.nPFrmBitsCoef = 10;
            config_1.vbr_opt_enable = 1;
        }
        config_1.qp_range.nMinqp = 25;
        config_1.qp_range.nMaxqp = 45;
        config_1.qp_range.nMinPqp = 25;
        config_1.qp_range.nMaxPqp = 45;
        config_1.qp_range.nQpInit = 35;
        config_1.profile = VENC_H264ProfileMain;
        config_1.level   = VENC_H264Level51;
    #if TEST_OUTSIDE_YUV
        config_1.output_mode = OUTPUT_MODE_ENCODE_OUTSIDE_YUV;
    #else
        config_1.output_mode = OUTPUT_MODE_STREAM;
    #endif
        config_1.width      = mparam.c1_src_w;
        config_1.height     = mparam.c1_src_h;
        config_1.bitrate    = mparam.c1_bitrate;
        config_1.encodeType = mparam.c1_encoder_format;
        config_1.pixelformat = mparam.pixelformat_1;
        config_1.enable_sharp = 1;
        config_1.bonline_channel = mparam.bonline_channel_1;
        config_1.share_buf_num = mparam.share_buf_num_1;
        config_1.vin_buf_num = mparam.vin_buf_num;//for example, not set, kernel will set to 3
        config_1.breduce_refrecmem = 1;
#if ENABLE_TEST_SET_CAMERA_MOVE_STATUS
        config_1.enable_isp2ve_linkage = 1;
        config_1.enable_ve2isp_linkage = 0; //only main channel need enable ve2isp
#else
        config_1.enable_isp2ve_linkage = 1;
        config_1.enable_ve2isp_linkage = 0;
#endif
        // if (mparam.c0_src_w)
        // 	config_1.ve_encpp_sharp_atten_coef_per = 100 * mparam.c1_src_w / mparam.c0_src_w;
        // else
        // 	config_1.ve_encpp_sharp_atten_coef_per = 100;

        config_1.dma_merge_mode = mparam.dma_merge_mode_1;
        alogd("config_1.dma_merge_mode = %d\n", config_1.dma_merge_mode);
        if (config_1.bonline_channel && (config_1.dma_merge_mode >= DMA_STITCH_HORIZONTAL)) {
            config_1.bEnableMultiOnlineSensor = 1;
            config_1.bEnableImageStitching = 1;
        }
        config_1.dma_merge_scaler_cfg.scaler_en = mparam.scaler_en_1;
        config_1.dma_merge_scaler_cfg.sensorA_scaler_cfg.width = mparam.dma_merge_scaler_sensorA_width_1;
        config_1.dma_merge_scaler_cfg.sensorA_scaler_cfg.height = mparam.dma_merge_scaler_sensorA_height_1;
        config_1.dma_merge_scaler_cfg.sensorB_scaler_cfg.width = mparam.dma_merge_scaler_sensorB_width_1;
        config_1.dma_merge_scaler_cfg.sensorB_scaler_cfg.height = mparam.dma_merge_scaler_sensorB_height_1;
        alogd("channelId:%d, SensorA: %dx%d, SensorB: %dx%d\n", config_1.channelId, config_1.dma_merge_scaler_cfg.sensorA_scaler_cfg.width,
            config_1.dma_merge_scaler_cfg.sensorA_scaler_cfg.height, config_1.dma_merge_scaler_cfg.sensorB_scaler_cfg.width,
            config_1.dma_merge_scaler_cfg.sensorB_scaler_cfg.height);

        init_color_space(&config_1.venc_video_signal, mparam.color_space);
        AWVideoInput_Configure(channelId_1, &config_1);
        AWVideoInput_CallBack(channelId_1, video_stream_cb_1, 1);
        AWVideoInput_Start(channelId_1, 1);
    }

    setParam(&mparam, channelId_0, &config_0);

#ifdef ENABLE_PDET
    /*pdet*/
    if (1) {
        VideoInputConfig config_2;
        memset(&config_2, 0, sizeof(VideoInputConfig));

        config_2.channelId = channelId_2;
        config_2.encodeType = 0;
        config_2.width = JPEG_WIDTH;
        config_2.height = JPEG_HEIGHT;
        config_2.fps   = 15;
        config_2.bitrate = 0;
        config_2.gop = 0;
        config_2.mRcMode = 0;
        config_2.output_mode = OUTPUT_MODE_YUV;
        config_2.pixelformat = mparam.pixelformat_2;
        config_2.enable_sharp = 0;
        alogd("ch%d, %dx%d", channelId_2, config_2.width, config_2.height);

        init_color_space(&config_2.venc_video_signal, mparam.color_space);

        AWVideoInput_Configure(channelId_2, &config_2);

        AWVideoInput_Start(channelId_2, 1);


        SampleDewbInfo *pDewbInfo = (SampleDewbInfo *)malloc(sizeof(SampleDewbInfo));
        memset(pDewbInfo, 0, sizeof(SampleDewbInfo));

        SamplePdetInfo *pPdetInfo = (SamplePdetInfo *)malloc(sizeof(SamplePdetInfo));
        memset(pPdetInfo, 0, sizeof(SamplePdetInfo));

        pMotionPdetParam = (MotionPdetParam *)malloc(sizeof(MotionPdetParam));
        if (!pMotionPdetParam)
        {
            aloge("malloc pMotionPdetParam fail");
            return -1;
        }
        pDewbInfo->mInputInfo.mWidth = config_2.width;
        pDewbInfo->mInputInfo.mHeight = config_2.height;
        pDewbInfo->mOutputInfo.mWidth = PDET_INPUT_W;
        pDewbInfo->mOutputInfo.mHeight = PDET_INPUT_H;

        pPdetInfo->pdet_conf_thres = 0.3;
        pPdetInfo->pdet_input_w = PDET_INPUT_W;
        pPdetInfo->pdet_input_h = PDET_INPUT_H;
        pPdetInfo->pdet_input_c = PDET_INPUT_C;

        MotionDetectContext *pMotionDetectContext = &pMotionPdetParam->mMotionDetectContext;

        pMotionPdetParam->pDewbInfo = pDewbInfo;
        pMotionPdetParam->pPdetInfo = pPdetInfo;

        pMotionDetectContext->mMdStaCfg.nPicWidth = rt_ALIGN_XXB(16, config_2.width);
        pMotionDetectContext->mMdStaCfg.nPicStride = rt_ALIGN_XXB(16, config_2.width);
        pMotionDetectContext->mMdStaCfg.nPicHeight = rt_ALIGN_XXB(16, config_2.height);
        pMotionDetectContext->mMdDynCfg.nHorRegionNum = pMotionDetectContext->mMdStaCfg.nPicWidth / 16;
        pMotionDetectContext->mMdDynCfg.nVerRegionNum = pMotionDetectContext->mMdStaCfg.nPicHeight / 16;
        pMotionDetectContext->mMdDynCfg.bDefaultCfgDis = 1;
        pMotionDetectContext->mMdDynCfg.bMorphologEn = 0;
        pMotionDetectContext->mMdDynCfg.nLargeMadTh = 10;
        pMotionDetectContext->mMdDynCfg.nBackgroundWeight = 0;
        pMotionDetectContext->mMdDynCfg.fLargeMadRatioTh = 10.0;

        pMotionDetectContext->mJudgeMotionNum = 50;
        pMotionDetectContext->mMotionDynamicThres = 0.43;
        pMotionDetectContext->mMotionPdetThres = 0.4;

        motion_pdet_set_sensitivity(pMotionPdetParam, MOTION_PDET_SENSITIVITY_MIDDLE);

        alogd("DewbInfo, in w:%d, h:%d, out w:%d, h:%d", pDewbInfo->mInputInfo.mWidth, pDewbInfo->mInputInfo.mHeight,
                    pDewbInfo->mOutputInfo.mWidth, pDewbInfo->mOutputInfo.mHeight);
        alogd("PdetInfo, w:%d h:%d", pPdetInfo->pdet_input_w, pPdetInfo->pdet_input_h);
        alogd("mMdStaCfg, w:%d h:%d", pMotionDetectContext->mMdStaCfg.nPicWidth, pMotionDetectContext->mMdStaCfg.nPicHeight);
    }
#endif

    alogd("tkl vi init success");
    return OPRT_OK;
_exit:
    exit_demo();
    return OPRT_OK;
}

/**
* @brief vi set mirror and flip
* 
* @param[in] chn: vi chn
* @param[in] flag: mirror and flip value
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_set_mirror_flip(TKL_VI_CHN_E chn, TKL_VI_MIRROR_FLIP_E flag)
{
    alogd("chn:%d, flag:%d", chn, flag);

    if (flag == TKL_VI_MIRROR_FLIP_NONE) {
        AWVideoInput_SetHFlip(0, 1); //actual contrary
        AWVideoInput_SetVFlip(0, 1);
    } else if (flag == TKL_VI_MIRROR_FLIP) {
        AWVideoInput_SetHFlip(0, 0);
        AWVideoInput_SetVFlip(0, 0);
    } else {
        aloge("flag err");
    }
    return OPRT_OK;
}

/**
* @brief vi get mirror and flip
* 
* @param[in] chn: vi chn
* @param[out] flag: mirror and flip value
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_get_mirror_flip(TKL_VI_CHN_E chn, TKL_VI_MIRROR_FLIP_E *flag)
{
    alogd("enter");
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief vi uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_uninit(VOID)
{
    alogd("enter");

    if (video_finish_flag == 1) {
        return OPRT_OK;
    }
    video_finish_flag = 1;
#if ENABLE_ISP_REG_GET
    //ISP REG Get
    VIN_ISP_REG_GET_CFG isp_reg_get_cfg;
    memset(&isp_reg_get_cfg, 0, sizeof(isp_reg_get_cfg));
#if 1
    alogd("ISP REG action: [print]\n");
    isp_reg_get_cfg.flag = 0;
    AWVideoInput_GetISPReg(channelId_0, &isp_reg_get_cfg);
#else
    alogd("ISP REG action: [save]\n");
    isp_reg_get_cfg.flag = 1;
    isp_reg_get_cfg.path = ISP_REG_SAVE_FILE;
    isp_reg_get_cfg.len = strlen(isp_reg_get_cfg.path);
    AWVideoInput_GetISPReg(channelId_0, &isp_reg_get_cfg);
#endif
#endif

    // thread_exit_flag = 1;

    // alogd("exit, stream_count_0 = %d\n", stream_count_0);
    if (mparam.en_second_channel && mparam.en_third_channel) {
        AWVideoInput_Start(MAX3(channelId_0, channelId_1, channelId_2), 0);
        AWVideoInput_Start(MEDIAN3(channelId_0, channelId_1, channelId_2), 0);
        AWVideoInput_Start(MIN3(channelId_0, channelId_1, channelId_2), 0);
    } else if (mparam.en_second_channel) {
        AWVideoInput_Start(AWMAX(channelId_0, channelId_1), 0);
        AWVideoInput_Start(AWMIN(channelId_0, channelId_1), 0);
    } else if (mparam.en_third_channel) {
        AWVideoInput_Start(AWMAX(channelId_0, channelId_2), 0);
        AWVideoInput_Start(AWMIN(channelId_0, channelId_2), 0);
    } else {
        AWVideoInput_Start(mparam.use_vipp_num, 0);
    }
    exit_demo();
    isp_config_to_flash();

    alogd("exit: deInit end\n");

    return OPRT_OK;
}

/**
* @brief  set sensor reg value
* 
* @param[in] chn: vi chn
* @param[in] pparam: reg info
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_sensor_reg_set( TKL_VI_CHN_E chn, TKL_VI_SENSOR_REG_CONFIG_T *parg)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief  get sensor reg value
* 
* @param[in] chn: vi chn
* @param[in] pparam: reg info
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_sensor_reg_get( TKL_VI_CHN_E chn, TKL_VI_SENSOR_REG_CONFIG_T *parg)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

static pthread_t motion_thread = 0;
static pthread_t pdet_thread = 0;
static bool motion_detect_status = false;
static bool human_detect_status = false;

void set_motion_detect_status(bool status)
{
    motion_detect_status = status;
}

int get_motion_detect_status()
{
    return motion_detect_status;
}

void set_human_detect_status(bool status)
{
    human_detect_status = status;
}

int get_human_detect_status()
{
    return human_detect_status;
}

/**
* @brief detect init   检测接口
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[in] pconfig: config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_init(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_CONFIG_T *p_config)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief detect start
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_start(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    alogd("enter,type:%d", type);
    int result = 0;

    if (type == TKL_MEDIA_DETECT_TYPE_MOTION) {
        if (get_motion_detect_status() == true) {
            alogd("no need");
            return OPRT_OK;
        }
        set_motion_detect_status(true);

        if (mparam.enable_motion_search) {
            result = pthread_create(&motion_thread, NULL, motion_search_thread, (void*)&channelId_0);
            if (result != 0)
            {
                aloge("fatal error! pthread create fail[%d]", result);
                return OPRT_COM_ERROR;
            }
        }

    } else if (type == TKL_MEDIA_DETECT_TYPE_HUMAN) {
        if (get_human_detect_status() == true) {
            alogd("no need");
            return OPRT_OK;
        }
        set_human_detect_status(true);
#ifdef ENABLE_PDET

        MotionDetectContext *pMotionDetectContext = &pMotionPdetParam->mMotionDetectContext;
        SampleDewbInfo *pDewbInfo = pMotionPdetParam->pDewbInfo;
        SamplePdetInfo *pPdetInfo = pMotionPdetParam->pPdetInfo;

        alogd("mMdStaCfg 2, w:%d h:%d", pMotionDetectContext->mMdStaCfg.nPicWidth, pMotionDetectContext->mMdStaCfg.nPicHeight);
        alogd("nBufSize 2, w %d h %d c %d", pDewbInfo->mOutputInfo.mWidth, pDewbInfo->mOutputInfo.mHeight, pPdetInfo->pdet_input_c);
        if (0 != motion_pdet_init(pMotionPdetParam))
        {
            aloge("fatal error! motion_pdet_init failed!");
            return OPRT_COM_ERROR;
        }

        alogd("Create Pdet pthread");
        PersonDetectExitFlag = 0;
        result = pthread_create(&pdet_thread, NULL, runPdet, (void *)pMotionPdetParam);
        if (result != 0)
        {
            aloge("fatal error! pthread create fail[%d]", result);
            return OPRT_COM_ERROR;
        }
#endif
    }



    return OPRT_OK;
}

/**
* @brief detect stop
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_stop(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    alogd("enter,type:%d", type);

    if (type == TKL_MEDIA_DETECT_TYPE_MOTION) {
        if (get_motion_detect_status() == false) {
            alogd("no need");
            return OPRT_OK;
        }
        set_motion_detect_status(false);

        if (mparam.enable_motion_search) {
            if(motion_thread != 0)
                pthread_join(motion_thread, NULL);
            motion_thread = 0;
            alogd("end motion thread");
        }

    } else if (type == TKL_MEDIA_DETECT_TYPE_HUMAN) {
        if (get_human_detect_status() == false) {
            alogd("no need");
            return OPRT_OK;
        }
        set_human_detect_status(false);
#ifdef ENABLE_PDET

        PersonDetectExitFlag = 1;
        pthread_join(pdet_thread, NULL);
        if(pdet_thread != 0)
            pthread_join(pdet_thread, NULL);
        pdet_thread = 0;
        motion_pdet_deinit(pMotionPdetParam);

        alogd("destroy Pdet pthread");
#endif
    }






    return OPRT_OK;
}

/**
* @brief get detection results
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[out] presult: detection results
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_get_result(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_RESULT_T *presult)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

/**
* @brief set detection param
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[in] pparam: detection param
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_set(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_PARAM_T *pparam)
{
    alogd("enter");
    int sensitivity = pparam->sensitivity;
    alogd("set sensitivity:%d", sensitivity);

    if (TKL_MEDIA_DETECT_TYPE_MOTION == type) {
        /*0 means low sensitivity,1 means medium sensitivity,2 means high sensitivity*/
        switch (sensitivity) {
        case 0:
            motionAlarm_sensitivity = 30;
            break;
        case 1:
            motionAlarm_sensitivity = 20;
            break;
        case 2:
            motionAlarm_sensitivity = 10;
            break;
        default:
            motionAlarm_sensitivity = 30;
            break;
        }
    } else if (TKL_MEDIA_DETECT_TYPE_DB == type) {
        /*0 means low sensitivity,1 means high sensitivity*/
        switch (sensitivity) {
        case 0:
            fDBThreshold = 37;
            break;
        case 1:
            fDBThreshold = 30;
            break;
        default:
            fDBThreshold = 37;
            break;
        }
    }


    return OPRT_OK;
}

/**
* @brief detect uninit
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_uninit(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    // --- BEGIN: user implements ---
    return OPRT_OK;
    // --- END: user implements ---
}

void add_md_pending_count(void)
{
    // md_pending_cnt++;
    // alogd("md_pending_cnt:%d", md_pending_cnt);
    // if (md_pending_cnt) {
    //     set_motion_detect_status(false);
    // }
}

void remove_md_pending_count(void)
{
    // md_pending_cnt--;
    // alogd("md_pending_cnt:%d", md_pending_cnt);
    // if (md_pending_cnt == 0) {
    //     usleep(1000*1000); //must
    //     set_motion_detect_status(true);
    // }
}

OPERATE_RET tkl_vi_set_day_night_mode(unsigned char mode)
{ //0:day 1:night
    int ret = 0;
    int irled_port = 111;
    RTIspCtrlAttr isp_ctrl_attr;

    memset(&isp_ctrl_attr, 0, sizeof(RTIspCtrlAttr));
    if (save_day_night_mod == mode) {
        return OPRT_OK;
    }
    alogd("changed, set_day_night_mode:%d", mode);

    if (first_init_flag == 0) {
        first_init_flag = 1;
        /*ircut*/
        system("echo enable > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_ctr");
        /*irled*/
        system("echo 111 > /sys/class/gpio/export");
        system("echo out > /sys/class/gpio/gpio111/direction");
    }

    //irled
    //ir-cut
    //isp
    if (mode == 0) {
        system("echo 0 > /sys/class/gpio/gpio111/value");
        system("echo close > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_status");

        usleep(200*1000);
        /* compile .h's tool*/
        isp_ctrl_attr.isp_attr_cfg.cfg_id = ISP_CTRL_IR_STATUS;
        isp_ctrl_attr.isp_attr_cfg.ir_status = 0;
        ret = AWVideoInput_SetIspAttrCfg(0, &isp_ctrl_attr);
        /*load bin's tool*/
        // if (access(ISP_RGB_BIN_PATH, F_OK) == 0) {
        //     isp_ctrl_attr.isp_attr_cfg.cfg_id = ISP_CTRL_READ_BIN_PARAM;
        //     strncpy(isp_ctrl_attr.isp_attr_cfg.path, ISP_RGB_BIN_PATH, 16);
        //     ret = AWVideoInput_SetIspAttrCfg(0, &isp_ctrl_attr);
        // } else {
        //     aloge("isp bin:%s no exist", ISP_RGB_BIN_PATH);
        // }
    } else if (mode == 1) {
        isp_ctrl_attr.isp_attr_cfg.cfg_id = ISP_CTRL_IR_STATUS;
        isp_ctrl_attr.isp_attr_cfg.ir_status = 1;
        ret = AWVideoInput_SetIspAttrCfg(0, &isp_ctrl_attr);

        // if (access(ISP_IR_BIN_PATH, F_OK) == 0) {
        //     isp_ctrl_attr.isp_attr_cfg.cfg_id = ISP_CTRL_READ_BIN_PARAM;
        //     strncpy(isp_ctrl_attr.isp_attr_cfg.path, ISP_IR_BIN_PATH, 16);
        //     ret = AWVideoInput_SetIspAttrCfg(0, &isp_ctrl_attr);
        // } else {
        //     aloge("isp bin:%s no exist", ISP_IR_BIN_PATH);
        // }
        usleep(200*1000);

        system("echo 1 > /sys/class/gpio/gpio111/value");
        system("echo open > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_status");
    }

    if (ret != 0) {
        aloge("ISP Switch fail!");
        return OPRT_COM_ERROR;
    }
    save_day_night_mod = mode;
    alogd("ISP Switch success!");
    return OPRT_OK;
}

unsigned char tkl_vi_get_day_night_mode(void)
{
    return save_day_night_mod;
}

OPERATE_RET tkl_vi_set_contrast(int value)
{
    return OPRT_OK;
}

OPERATE_RET tkl_vi_get_contrast(int value)
{
    return OPRT_OK;
}

OPERATE_RET tkl_vi_set_brightness(int value)
{
    return OPRT_OK;
}

OPERATE_RET tkl_vi_get_brightness(int value)
{
    return OPRT_OK;
}

OPERATE_RET tkl_vi_set_flicker(int value)
{
    alogd("enter, set flicker:%d", value); //0:disable 1:50HZ 2:60HZ
    int ret;

    ret = AWVideoInput_SetPowerLineFreq(0, value);

    if (ret != 0) {
        aloge("Set fail");
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}
