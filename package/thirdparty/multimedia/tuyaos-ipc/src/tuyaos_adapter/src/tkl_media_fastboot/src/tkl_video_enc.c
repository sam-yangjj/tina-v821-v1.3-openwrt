#include "tkl_video_enc.h"
#include "plat_log.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <time.h>
#include "rgb_ctrl.h"
#include "AW_VideoInput_API.h"
#include "aw_util.h"

static char SetOsdExitFlag = 1;
pthread_t mOsdMainRunThreadId;
pthread_t mOsdSubRunThreadId;
static int res_mem_size;
static int res_mem_size_sub;
static char *res_mem_buff;
static char *res_mem_buff_sub;

extern int getJpegStream(TKL_VENC_FRAME_T *pframe);

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

    alogd("enter");

    return OPRT_OK;
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
    return OPRT_OK;
    // --- END: user implements ---
}

static void *setMainOsdThread(void *pThreadData)
{
    char strThreadName[32];
    sprintf(strThreadName, "setOsdThr");
    prctl(PR_SET_NAME, (unsigned long)strThreadName, 0, 0, 0);

    FONT_SIZE_TYPE font_size = FONT_SIZE_64;

    int ret = load_font_file(font_size);
    if (ret < 0) {
        aloge("load_font_file %d fail! ret:%d\n", ret, font_size);
        return;
    }

    FONT_RGBPIC_S font_pic;
    memset(&font_pic, 0, sizeof(font_pic));
    font_pic.font_type = font_size;
    font_pic.rgb_type = OSD_RGB_32;
    font_pic.enable_bg = 0;
    font_pic.foreground[0] = 0xFF;
    font_pic.foreground[1] = 0xFF;
    font_pic.foreground[2] = 0xFF;
    font_pic.foreground[3] = 0xFF;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;

    RGB_PIC_S rgb_pic;
    memset(&rgb_pic, 0, sizeof(rgb_pic));
    rgb_pic.rgb_type = OSD_RGB_32;
    rgb_pic.enable_mosaic = 0;

    VideoInputOSD OverlayInfo;
    memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
    int index = 0;

    OverlayInfo.osd_num = 1;
    OverlayInfo.item_info[index].osd_type = LUMA_REVERSE_OVERLAY;
    OverlayInfo.argb_type = VENC_OVERLAY_ARGB8888;
    OverlayInfo.invert_mode = 2; //all inversion
    OverlayInfo.invert_threshold = 90;
    OverlayInfo.item_info[index].start_x = 48;
    OverlayInfo.item_info[index].start_y = 32;


    struct timeval tv;
    struct tm *tm_info;
    char time_str[64];

    alogd("run OsdThread");
    SetOsdExitFlag = 0;
    while (!SetOsdExitFlag)
    {
        gettimeofday(&tv, NULL);
        tm_info = localtime(&tv.tv_sec);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        create_font_rectangle(time_str, &font_pic, &rgb_pic);

        OverlayInfo.item_info[index].widht = rgb_pic.wide;
        OverlayInfo.item_info[index].height = rgb_pic.high;
        OverlayInfo.item_info[index].data_size = rgb_pic.pic_size;

        if (res_mem_size >= rgb_pic.pic_size) {
            OverlayInfo.item_info[index].data_buf = res_mem_buff;
            memset(OverlayInfo.item_info[index].data_buf, 0, rgb_pic.pic_size);
            memcpy(OverlayInfo.item_info[index].data_buf, rgb_pic.pic_addr, rgb_pic.pic_size);
        } else {
            if (res_mem_buff) {
                free(res_mem_buff);
                res_mem_buff = NULL;
            }
            res_mem_buff = malloc(rgb_pic.pic_size);
            if (!res_mem_buff) {
                aloge("fatal error! malloc item data buf %d Bytes fail!", rgb_pic.pic_size);
                return;
            }
            res_mem_size = rgb_pic.pic_size;
            OverlayInfo.item_info[index].data_buf = res_mem_buff;

            memset(OverlayInfo.item_info[index].data_buf, 0, rgb_pic.pic_size);
            memcpy(OverlayInfo.item_info[index].data_buf, rgb_pic.pic_addr, rgb_pic.pic_size);
        }
        AWVideoInput_SetOSD(0, &OverlayInfo); //channelId_0

        release_rgb_picture(&rgb_pic);
        usleep(1000 * 1000);
    }

    /* clean osd*/
    memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
    AWVideoInput_SetOSD(0, &OverlayInfo); //channelId_0

    ret = unload_font_file(font_size);
    if (ret < 0) {
        aloge("unload_font_file %d fail! ret:%d\n", ret, font_size);
    }
    if (res_mem_buff) {
        free(res_mem_buff);
        res_mem_buff = NULL;
        res_mem_size = 0;
    }
}

static void *setSubOsdThread(void *pThreadData)
{
    char strThreadName[32];
    sprintf(strThreadName, "setOsdThrSub");
    prctl(PR_SET_NAME, (unsigned long)strThreadName, 0, 0, 0);

    FONT_SIZE_TYPE font_size = FONT_SIZE_16;

    int ret = load_font_file(font_size);
    if (ret < 0) {
        aloge("load_font_file %d fail! ret:%d\n", ret, font_size);
        return;
    }

    FONT_RGBPIC_S font_pic;
    memset(&font_pic, 0, sizeof(font_pic));
    font_pic.font_type = font_size;
    font_pic.rgb_type = OSD_RGB_32;
    font_pic.enable_bg = 0;
    font_pic.foreground[0] = 0xFF;
    font_pic.foreground[1] = 0xFF;
    font_pic.foreground[2] = 0xFF;
    font_pic.foreground[3] = 0xFF;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;
    font_pic.background[0] = 0x88;

    RGB_PIC_S rgb_pic;
    memset(&rgb_pic, 0, sizeof(rgb_pic));
    rgb_pic.rgb_type = OSD_RGB_32;
    rgb_pic.enable_mosaic = 0;

    VideoInputOSD OverlayInfo;
    memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
    int index = 0;

    OverlayInfo.osd_num = 1;
    OverlayInfo.item_info[index].osd_type = LUMA_REVERSE_OVERLAY;
    OverlayInfo.argb_type = VENC_OVERLAY_ARGB8888;
    OverlayInfo.invert_mode = 2; //all inversion
    OverlayInfo.invert_threshold = 90;
    OverlayInfo.item_info[index].start_x = 32;
    OverlayInfo.item_info[index].start_y = 16;


    struct timeval tv;
    struct tm *tm_info;
    char time_str[64];

    alogd("run OsdThread");
    SetOsdExitFlag = 0;
    while (!SetOsdExitFlag)
    {
        gettimeofday(&tv, NULL);
        tm_info = localtime(&tv.tv_sec);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        create_font_rectangle(time_str, &font_pic, &rgb_pic);

        OverlayInfo.item_info[index].widht = rgb_pic.wide;
        OverlayInfo.item_info[index].height = rgb_pic.high;
        OverlayInfo.item_info[index].data_size = rgb_pic.pic_size;

        if (res_mem_size_sub >= rgb_pic.pic_size) {
            OverlayInfo.item_info[index].data_buf = res_mem_buff_sub;
            memset(OverlayInfo.item_info[index].data_buf, 0, rgb_pic.pic_size);
            memcpy(OverlayInfo.item_info[index].data_buf, rgb_pic.pic_addr, rgb_pic.pic_size);
        } else {
            if (res_mem_buff_sub) {
                free(res_mem_buff_sub);
                res_mem_buff_sub = NULL;
            }
            res_mem_buff_sub = malloc(rgb_pic.pic_size);
            if (!res_mem_buff_sub) {
                aloge("fatal error! malloc item data buf %d Bytes fail!", rgb_pic.pic_size);
                return;
            }
            res_mem_size_sub = rgb_pic.pic_size;
            OverlayInfo.item_info[index].data_buf = res_mem_buff_sub;

            memset(OverlayInfo.item_info[index].data_buf, 0, rgb_pic.pic_size);
            memcpy(OverlayInfo.item_info[index].data_buf, rgb_pic.pic_addr, rgb_pic.pic_size);
        }
        AWVideoInput_SetOSD(4, &OverlayInfo); //channelId_1

        release_rgb_picture(&rgb_pic);
        usleep(1000 * 1000);
    }

    /* clean osd*/
    memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
    AWVideoInput_SetOSD(4, &OverlayInfo); //channelId_1

    ret = unload_font_file(font_size);
    if (ret < 0) {
        aloge("unload_font_file %d fail! ret:%d\n", ret, font_size);
    }
    if (res_mem_buff_sub) {
        free(res_mem_buff_sub);
        res_mem_buff_sub = NULL;
        res_mem_size_sub = 0;
    }
}

int switchColorBlock(bool enable)
{
    alogd("enter, switchColorBlock:%d", enable);
    int ret = 0;
    VideoInputOSD OverlayInfo;

    if (enable) {
        memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
        int index = 0;

        OverlayInfo.osd_num = 1;

        OverlayInfo.item_info[index].osd_type = COVER_OVERLAY;
        OverlayInfo.item_info[index].start_x = 480;
        OverlayInfo.item_info[index].start_y = 272;
        OverlayInfo.item_info[index].widht  = 960;
        OverlayInfo.item_info[index].height = 544;
        OverlayInfo.item_info[index].cover_yuv.cover_y = 0;
        OverlayInfo.item_info[index].cover_yuv.cover_u = 0;
        OverlayInfo.item_info[index].cover_yuv.cover_v = 0x64;
        ret = AWVideoInput_SetOSD(0, &OverlayInfo); //channelId_0
        ret |= AWVideoInput_SetOSD(4, &OverlayInfo); //channelId_1

    } else {
        /* clean osd*/
        memset(&OverlayInfo, 0, sizeof(VideoInputOSD));
        ret = AWVideoInput_SetOSD(0, &OverlayInfo); //channelId_0
        ret |= AWVideoInput_SetOSD(4, &OverlayInfo); //channelId_1
    }
    return ret;
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
    int result = 0;

    if (posd->enable == 2) {
        /*Not authenticated, TODO osd coverlay*/
        posd->enable = 1;
    }

    if (posd->enable == 1) {
        if(SetOsdExitFlag == 0) //already run
            return OPRT_OK;
        alogd("Create Osd pthread");
        result = pthread_create(&mOsdMainRunThreadId, NULL, setMainOsdThread, NULL);
        if (result != 0) {
            aloge("fatal error! pthread create fail[%d]", result);
            return OPRT_COM_ERROR;
        }
        result = pthread_create(&mOsdSubRunThreadId, NULL, setSubOsdThread, NULL);
        if (result != 0) {
            aloge("fatal error! pthread create fail[%d]", result);
            return OPRT_COM_ERROR;
        }
    } else if (posd->enable == 0) {
        if(SetOsdExitFlag == 1)
            return OPRT_OK;
        void *pRetVal = NULL;
        void *pRetVal_sub = NULL;
        SetOsdExitFlag = 1;
        pthread_join(mOsdMainRunThreadId, &pRetVal);
        pthread_join(mOsdSubRunThreadId, &pRetVal_sub);
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
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
    return OPRT_OK;
    // --- END: user implements ---
}
