 /**
 * @file tkl_video_in.c
 * @brief video input
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 * Video input configuration: for sensor, ISP or VI attribute configuration, 
 * such as image flip, anti flicker, compensation mode, profile, etc.
 */

#include "tkl_video_in.h"
#include "plat_log.h"
#include "sample_smartIPC_demo_ty.h"

extern SampleSmartIPCDemoContext *gpSampleSmartIPCDemoContext;
extern void *getPdetRunThread(void *pThreadData);
extern void *getPdetCSIThread(void *pThreadData);

char HumanDetectExitFlag;
static char save_day_night_mod = -1;
static unsigned char first_init_flag = 0;

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
    char input[] = "sample_smartIPC_demo -path /etc/sample_smartIPC_demo.conf";
    char *argv[128];
    int argc = 0;

    char *token = strtok(input, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (sample_main(argc, argv) != 0) {
        aloge("start fail");
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

// Solving the hardware itself is the flip problem
//#define HW_DIRE_FLIP

#ifdef HW_DIRE_FLIP
#define ENABLE_MIRROR 0
#define ENABLE_FLIP   0
#else
#define ENABLE_MIRROR 1
#define ENABLE_FLIP   1
#endif

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
        AW_MPI_VI_SetVippMirror(4, !ENABLE_MIRROR);
        AW_MPI_VI_SetVippFlip(4, !ENABLE_FLIP);
#ifdef IPC_DUAL
        AW_MPI_VI_SetVippMirror(1, !ENABLE_MIRROR);
        AW_MPI_VI_SetVippFlip(1, !ENABLE_FLIP);
#endif
    } else if (flag == TKL_VI_MIRROR_FLIP) {
        AW_MPI_VI_SetVippMirror(4, ENABLE_MIRROR);
        AW_MPI_VI_SetVippFlip(4, ENABLE_FLIP);
#ifdef IPC_DUAL
        AW_MPI_VI_SetVippMirror(1, ENABLE_MIRROR);
        AW_MPI_VI_SetVippFlip(1, ENABLE_FLIP);
#endif
    } else {
        aloge("flag err");
    }
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
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
    sample_exit();
    alogd("sample exit");
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
    return OPRT_NOT_SUPPORTED;
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
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}


static bool motion_detect_status = false;
static bool human_detect_status = false;
static unsigned char md_pending_cnt = 0;

void set_motion_detect_status(bool status)
{
    motion_detect_status = status;
}

int get_motion_detect_status()
{
    return motion_detect_status;
}

void add_md_pending_count(void)
{
    md_pending_cnt++;
    alogd("md_pending_cnt:%d", md_pending_cnt);
    if (md_pending_cnt) {
        set_motion_detect_status(false);
    }
}

void remove_md_pending_count(void)
{
    md_pending_cnt--;
    alogd("md_pending_cnt:%d", md_pending_cnt);
    if (md_pending_cnt == 0) {
        usleep(1000*1000); //must
        set_motion_detect_status(true);
    }
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
* @brief detect init
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
    return OPRT_NOT_SUPPORTED;
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
    if (type == TKL_MEDIA_DETECT_TYPE_MOTION) {
        if (get_motion_detect_status() == true) {
            alogd("no need");
            return OPRT_OK;
        }
        set_motion_detect_status(true);
    } else if (type == TKL_MEDIA_DETECT_TYPE_HUMAN) {
        if (get_human_detect_status() == true) {
            alogw("no need");
            return OPRT_OK;
        }
        set_human_detect_status(true);

        //只有在使用的时候，才开启人形过滤线程，并关闭移动检测
        if (gpSampleSmartIPCDemoContext == NULL) {
            aloge("gpSampleSmartIPCDemoContext is null");
            return OPRT_COM_ERROR;
        }
        SampleSmartIPCDemoContext *pContext = gpSampleSmartIPCDemoContext;
        VencStreamContext *pStreamContext = &pContext->mMain2ndStream;
        int result = 0;

        if (pStreamContext->mPdetEnable)
        {
            cdx_sem_init(&pStreamContext->mSemPdetFrameReady, 0);
            pthread_mutex_init(&pStreamContext->mPdetLock, NULL);
            if (0 != motion_pdet_init(&pStreamContext->mMotionPdetParam))
            {
                aloge("fatal error! motion_pdet_init failed!");
            }

            alogd("Create Pdet pthread");
            pStreamContext->mPdetRunInterval = pContext->mConfigPara.mMain2ndPdetRunIinterval;
            AW_MPI_VI_CreateVirChn(pStreamContext->mVipp, pStreamContext->mPdetViChn, NULL);
            AW_MPI_VI_EnableVirChn(pStreamContext->mVipp, pStreamContext->mPdetViChn);
            HumanDetectExitFlag = 0;
            result = pthread_create(&pStreamContext->mPdetRunThreadId, NULL, getPdetRunThread, (void *)pStreamContext);
            if (result != 0)
            {
                aloge("fatal error! pthread create fail[%d]", result);
                return OPRT_COM_ERROR;
            }
            result = pthread_create(&pStreamContext->mGetPdetCSIThreadId, NULL, getPdetCSIThread, (void *)pStreamContext);
            if (result != 0)
            {
                aloge("fatal error! pthread create fail[%d]", result);
                return OPRT_COM_ERROR;
            }
        }
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
    } else if (type == TKL_MEDIA_DETECT_TYPE_HUMAN) {
        if (get_human_detect_status() == false) {
            alogd("no need");
            return OPRT_OK;
        }
        set_human_detect_status(false);

        if (gpSampleSmartIPCDemoContext == NULL) {
            aloge("gpSampleSmartIPCDemoContext is null");
            return OPRT_COM_ERROR;
        }
        SampleSmartIPCDemoContext *pContext = gpSampleSmartIPCDemoContext;
        VencStreamContext *pStreamContext = &pContext->mMain2ndStream;
        void *pRetVal = NULL;

        if (pStreamContext->mPdetEnable)
        {
            HumanDetectExitFlag = 1;
            pthread_join(pStreamContext->mGetPdetCSIThreadId, &pRetVal);
            alogd("main2nd GetPdetCSI pRetVal=%p", pRetVal);
            cdx_sem_up(&pStreamContext->mSemPdetFrameReady);
            pthread_join(pStreamContext->mPdetRunThreadId, &pRetVal);
            alogd("main2nd PdetRun pRetVal=%p", pRetVal);
            AW_MPI_VI_DisableVirChn(pStreamContext->mVipp, pStreamContext->mPdetViChn);
            AW_MPI_VI_DestroyVirChn(pStreamContext->mVipp, pStreamContext->mPdetViChn);
            pthread_mutex_destroy(&pStreamContext->mMotionPdetParam.mPdetLock);
            cdx_sem_deinit(&pStreamContext->mMotionPdetParam.mSemPdetFrameReady);
            motion_pdet_deinit(&pStreamContext->mMotionPdetParam);
            alogd("destroy Pdet pthread");
        }
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
    return OPRT_NOT_SUPPORTED;
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
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
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
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

OPERATE_RET tkl_vi_set_contrast(int value)
{
    alogd("enter, set_contrast:%d", value);

    int conver_value = value * 512 / 100;
    alogd("conver_value:%d", conver_value);

    AW_MPI_ISP_SetContrast(0, conver_value);
#ifdef IPC_DUAL
    AW_MPI_ISP_SetContrast(1, conver_value);
#endif

    return OPRT_OK;
}

int tkl_vi_get_contrast(void)
{
    int value = 0;
    int ret = AW_MPI_ISP_GetContrast(0, &value);
    if (ret !=0) {
        aloge("Get fail");
        return OPRT_COM_ERROR;
    }
    int conver_value = value * 100 / 512;
    alogd("value:%d, conver_value:%d", value, conver_value);
    return conver_value;
}

OPERATE_RET tkl_vi_set_brightness(int value)
{
    alogd("enter, set_brightness:%d", value);
    int ret;

    int conver_value = value * 512 / 100;
    alogd("conver_value:%d", conver_value);

    ret = AW_MPI_ISP_SetBrightness(0, conver_value);
#ifdef IPC_DUAL
    ret = AW_MPI_ISP_SetBrightness(1, conver_value);
#endif
    if (ret != 0) {
        aloge("Set fail");
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

int tkl_vi_get_brightness(void)
{
    int value = 0;
    int ret = AW_MPI_ISP_GetBrightness(0, &value);
    if (ret !=0) {
        aloge("Get fail");
        return OPRT_COM_ERROR;
    }
    int conver_value = value * 100 / 512;
    alogd("value:%d, conver_value:%d", value, conver_value);
    return conver_value;
}

OPERATE_RET tkl_vi_set_day_night_mode(unsigned char mode)
{ //0:day 1:night
    int ret = 0;
    int irled_port = 358;
    if (save_day_night_mod == mode) {
        return OPRT_OK;
    }
    alogd("changed, set_day_night_mode:%d", mode);

    if (first_init_flag == 0) {
        first_init_flag = 1;
        /*ircut*/
        system("echo enable > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_ctr");
        /*irled*/
        system("echo 358 > /sys/class/gpio/export");
        system("echo out > /sys/class/gpio/gpio358/direction");
    }

    if (mode == 0) { //night-->day: isp-->irled-->ircut
        ret = AW_MPI_ISP_SwitchIspConfig(0, NORMAL_CFG);
        system("echo 0 > /sys/class/gpio/gpio358/value");
        system("echo close > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_status");
    } else if (mode == 1) { //day-->night: irled-->ircut-->isp
        system("echo 1 > /sys/class/gpio/gpio358/value");
        system("echo open > /sys/devices/platform/soc@2002000/42000358.ircut/sunxi_ircut/ircut/ircut_status");
        ret = AW_MPI_ISP_SwitchIspConfig(0, IR_NORMAL_CFG);
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

OPERATE_RET tkl_vi_set_flicker(unsigned int value)
{
    alogd("enter, set flicker:%d", value);
    int ret;

    ret = AW_MPI_ISP_SetFlicker(0, value);
#ifdef IPC_DUAL
    ret = AW_MPI_ISP_SetFlicker(1, value);
#endif
    if (ret != 0) {
        aloge("Set fail");
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}
