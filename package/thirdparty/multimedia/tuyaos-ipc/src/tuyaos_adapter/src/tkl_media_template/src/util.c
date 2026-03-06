#include <stdio.h>
#include "tuya_error_code.h"
#include <zbar.h>

#include "plat_log.h"
#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include "media/mpi_sys.h"
#include "mpi_videoformat_conversion.h"
#include <SystemBase.h>
#include <confparser.h>
#include <VIDEO_FRAME_INFO_S.h>

#define MAX_FILE_PATH_SIZE  (256)
// #define DEBUG

typedef struct IdentifyQRCode_Cap_S {
    VIDEO_FRAME_INFO_S pstFrameInfo;
    pthread_t thid;
    int mThExitFlag;
    VI_DEV Dev;
    VI_CHN Chn;
    int s32MilliSec;
    void *mpContext;    //SampleIdentifyQRCodeContext*
    unsigned int mCurFrameId;
} IdentifyQRCode_Cap_S;

typedef struct SampleIdentifyQRCodeConfig
{
    int DevNum;
    int SrcWidth;
    int SrcHeight;
    int FrameRate;
    unsigned int SrcFormat;
    enum v4l2_colorspace mColorSpace;
    int mViDropFrmCnt;
    int mEncppEnable;
    unsigned int mTestDuration;
    char mStoreDirectory[MAX_FILE_PATH_SIZE];	//e.g.: /mnt/extsd
} SampleIdentifyQRCodeConfig;

typedef struct SampleIdentifyQRCodeContext
{
    SampleIdentifyQRCodeConfig mConfigPara;

    IdentifyQRCode_Cap_S privCap;
} SampleIdentifyQRCodeContext;

#define ISP_RUN 1

static SampleIdentifyQRCodeContext *gSampleIdentifyQRCodeContext = NULL;
extern int TUYA_IPC_Get_MqttStatus();
static bool is_initialized;


static int hal_virvi_start(VI_DEV ViDev, VI_CHN ViCh, void *pAttr)
{
    int ret = -1;

    ret = AW_MPI_VI_CreateVirChn(ViDev, ViCh, pAttr);
    if (ret < 0)
    {
        aloge("Create VI Chn failed,VIDev = %d, VIChn = %d", ViDev, ViCh);
        return ret;
    }
    ret = AW_MPI_VI_SetVirChnAttr(ViDev, ViCh, pAttr);
    if (ret < 0)
    {
        aloge("Set VI ChnAttr failed,VIDev = %d,VIChn = %d", ViDev, ViCh);
        return ret;
    }
    ret = AW_MPI_VI_EnableVirChn(ViDev, ViCh);
    if (ret < 0)
    {
        aloge("VI Enable VirChn failed,VIDev = %d, VIChn = %d", ViDev, ViCh);
        return ret;
    }

    return 0;
}

static int hal_virvi_end(VI_DEV ViDev, VI_CHN ViCh)
{
    int ret = -1;
    ret = AW_MPI_VI_DisableVirChn(ViDev, ViCh);
    if (ret < 0)
    {
        aloge("Disable VI Chn failed,VIDev = %d, VIChn = %d", ViDev, ViCh);
        return ret;
    }
    ret = AW_MPI_VI_DestroyVirChn(ViDev, ViCh);
    if (ret < 0)
    {
        aloge("Destory VI Chn failed,VIDev = %d, VIChn = %d", ViDev, ViCh);
        return ret;
    }
    return 0;
}

static ERRORTYPE loadConfigPara(SampleIdentifyQRCodeConfig *pConfig, const char *conf_path)
{
    if (NULL == pConfig)
    {
        aloge("fatal error! param is null");
        return FAILURE;
    }

    memset(pConfig, 0, sizeof(SampleIdentifyQRCodeConfig));
    pConfig->DevNum = 0;
    pConfig->FrameRate = 15;
    pConfig->SrcWidth = 400;
    pConfig->SrcHeight = 400;
	pConfig->SrcFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	pConfig->mColorSpace = V4L2_COLORSPACE_JPEG;
    strcpy(pConfig->mStoreDirectory, "/mnt/sdcard/qr.raw");
	pConfig->mViDropFrmCnt = 0;
    pConfig->mEncppEnable = 1;
    pConfig->mTestDuration = 15;
    return SUCCESS;
}

static int GetCSIFrame(void *pArg, char *qrcode_str)
{
    uint64_t pStartTime = 0;    // unit: ms
    uint64_t pTestDuration = 0; // unit: ms
    VI_DEV ViDev;
    VI_CHN ViCh;
    int ret = 0;
    int ImgScanret = 0;

    if (qrcode_str == NULL)
    {
        aloge("qrcode_str == NULL");
        return -1;
    }
    IdentifyQRCode_Cap_S *pCap = (IdentifyQRCode_Cap_S *)pArg;
    SampleIdentifyQRCodeContext *pContext = (SampleIdentifyQRCodeContext*)pCap->mpContext;
    ViDev = pCap->Dev;
    ViCh = pCap->Chn;
    alogd("Cap threadid=0x%lx, ViDev = %d, ViCh = %d", pCap->thid, ViDev, ViCh);

#ifdef DEBUG
        FILE *fpQR = fopen(pContext->mConfigPara.mStoreDirectory, "wb");
#endif

    zbar_image_scanner_t *scanner = zbar_image_scanner_create();
    if (!scanner)
    {
        aloge("fatal error! create zbar image scanner fail!");
        return -1;
    }

    if(zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1) != 0)
    {
        aloge("fatal error! zbar_image_scanner_set_config failed\n");
        return -1;
    }

    pStartTime = CDX_GetSysTimeUsMonotonic() / 1000;
    pTestDuration = pContext->mConfigPara.mTestDuration * 1000;
    while (!pCap->mThExitFlag)
    {
        if (TUYA_IPC_Get_MqttStatus() == true)
        {
            aloge("Get_MqttStatus true");
            ImgScanret = 3;
            break;
        }

        if ((CDX_GetSysTimeUsMonotonic() / 1000 - pStartTime) >= pTestDuration)
        {
            aloge("scanner QRCode fail! scan timeout");
            break;
        }

        // vi get frame
        if ((ret = AW_MPI_VI_GetFrame(ViDev, ViCh, &pCap->pstFrameInfo, pCap->s32MilliSec)) != 0)
        {
            continue;
        }

        zbar_image_t *image = zbar_image_create();
        if(NULL == image)
        {
            aloge("fatal error! zbar_image_create failed");
            continue;
        }
        if (NULL == pCap->pstFrameInfo.VFrame.mpVirAddr[0])
        {
            aloge("mpVirAddr[0] is null");
            continue;
        }
        unsigned int pWidth = pCap->pstFrameInfo.VFrame.mWidth;
        unsigned int pHeight = pCap->pstFrameInfo.VFrame.mHeight;
        // alogd("viraddr[%p] width[%d], height[%d]", pCap->pstFrameInfo.VFrame.mpVirAddr[0], pWidth, pHeight);
#ifdef DEBUG
        if (fpQR)
            fwrite(pCap->pstFrameInfo.VFrame.mpVirAddr[0], 1, pWidth * pHeight, fpQR);
#endif
        zbar_image_set_format(image, *(unsigned long *)"Y800");
        zbar_image_set_size(image, pWidth, pHeight);
        zbar_image_set_data(image, (const void *)pCap->pstFrameInfo.VFrame.mpVirAddr[0], pWidth * pHeight, NULL);

        /* scan the image for barcodes */
        zbar_scan_image(scanner, image);
        for(zbar_symbol_t* symbol = zbar_image_first_symbol(image); symbol; symbol = zbar_symbol_next(symbol)) {
            /* do something useful with results */
            zbar_symbol_type_t type = zbar_symbol_get_type(symbol);
            const char *data = zbar_symbol_get_data(symbol);

            alogd("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(type), data);
            if (strstr(data, "\"p\":") != NULL && strstr(data, "\"s\":") != NULL && strstr(data, "\"t\":") != NULL)
            {
                alogd("find WIFI QR code");
                ImgScanret = 2;
                if (strlen(data) < 256)
                    strcpy(qrcode_str, data);
                else
                    alogd("len error, %d", strlen(data));
            }
            else
            {
                alogd("but not find WIFI QR code");
                ImgScanret = 1;
            }

        }

        /* clean up */
        zbar_image_destroy(image);
        // vi release frame
        AW_MPI_VI_ReleaseFrame(ViDev, ViCh, &pCap->pstFrameInfo);

        if (1 == ImgScanret || 2 == ImgScanret)
        {
            break;
        }

        usleep(10 * 1000);
    }

    zbar_image_scanner_destroy(scanner);

#ifdef DEBUG
    if(fpQR != NULL) {
        fclose(fpQR);
        fpQR = NULL;
    }
#endif

    if (2 == ImgScanret)
    {
        aloge("get Img!");
        return 0;
    }
    else if (3 == ImgScanret)
    {
        aloge("mqtt already connected by other way");
        return 1;
    }
    else if (1 == ImgScanret)
    {
        aloge("scan, but not find WIFI QR code");
        return -1;
    }
    else
    {
        aloge("scan timeout, mTestDuration:%d", pContext->mConfigPara.mTestDuration);
        return -2;
    }
}

int qrcode_mode_main(char *p_str, int timeout)
{
    alogd("enter");
    int ret = 0;
    int result = 0;
    int vipp_dev = 0;
	int virvi_chn = 0;

    SampleIdentifyQRCodeContext *pContext = (SampleIdentifyQRCodeContext*)malloc(sizeof(SampleIdentifyQRCodeContext));
    if (NULL == pContext)
    {
        aloge("fatal error, malloc failed! size %d", sizeof(SampleIdentifyQRCodeContext));
        result = -1;
        goto _exit;
    }
    memset(pContext, 0, sizeof(SampleIdentifyQRCodeContext));
    gSampleIdentifyQRCodeContext = pContext;
    /* parse config file. */
    if(loadConfigPara(&pContext->mConfigPara, NULL) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }
    pContext->mConfigPara.mTestDuration = timeout;
    vipp_dev = pContext->mConfigPara.DevNum;

    alogd("======================================.\n");
    system("cat /proc/meminfo | grep Committed_AS");
    alogd("======================================.\n");

    VI_ATTR_S stAttr;
    /*Set VI Channel Attribute*/
    memset(&stAttr, 0, sizeof(VI_ATTR_S));
    stAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    stAttr.memtype = V4L2_MEMORY_MMAP;
    stAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pContext->mConfigPara.SrcFormat);
    stAttr.format.field = V4L2_FIELD_NONE;
    stAttr.format.colorspace = pContext->mConfigPara.mColorSpace;
    stAttr.format.width = pContext->mConfigPara.SrcWidth;
    stAttr.format.height = pContext->mConfigPara.SrcHeight;
    stAttr.nbufs = 3;
    stAttr.nplanes = 2;
    stAttr.fps = pContext->mConfigPara.FrameRate;
    /* update configuration anyway, do not use current configuration */
    stAttr.use_current_win = 0;
    stAttr.wdr_mode = 0;
    stAttr.capturemode = V4L2_MODE_VIDEO; /* V4L2_MODE_VIDEO; V4L2_MODE_IMAGE; V4L2_MODE_PREVIEW */
    stAttr.drop_frame_num = pContext->mConfigPara.mViDropFrmCnt; // drop 2 second video data, default=0
    AW_MPI_VI_CreateVipp(vipp_dev);
    AW_MPI_VI_SetVippAttr(vipp_dev, &stAttr);
#if ISP_RUN
    int iIspDev = 0;
    AW_MPI_ISP_Run(iIspDev);
#endif
    AW_MPI_VI_EnableVipp(vipp_dev);

    alogd("start init virvi cap");
    pContext->privCap.Dev = pContext->mConfigPara.DevNum;
    pContext->privCap.Chn = virvi_chn;
    pContext->privCap.s32MilliSec = 5000;
    pContext->privCap.mpContext = (void*)pContext;
    pContext->privCap.mThExitFlag = 0;
    ret = hal_virvi_start(vipp_dev, virvi_chn, NULL); /* For H264 */
    if(ret != 0)
    {
        aloge("virvi start failed!");
        result = -1;
        goto _destroy;
    }
    // pContext->privCap.thid = 0;
    // ret = pthread_create(&pContext->privCap.thid, NULL, GetCSIFrameThread, (void *)&pContext->privCap);
    // if (ret < 0)
    // {
    //     alogd("pthread_create failed, Dev[%d], Chn[%d].\n", pContext->privCap.Dev, pContext->privCap.Chn);
    // }
    // else
    // {
    //     alogd("vipp[%d]virChn[%d]: get csi frame thread id=[0x%lx]", vipp_dev, virvi_chn, pContext->privCap.thid);
    // }

    // pthread_join(pContext->privCap.thid, NULL);
    result = GetCSIFrame((void *)&pContext->privCap, p_str);
    if (result < 0)
    {
        aloge("get pic failed,%d", ret);
    }

    ret = hal_virvi_end(vipp_dev, virvi_chn);
    if (ret != 0)
    {
        aloge("virvi end failed!");
        result = -1;
    }

_destroy:
    AW_MPI_VI_DisableVipp(vipp_dev);
#if ISP_RUN
    AW_MPI_ISP_Stop(iIspDev);
#endif
    AW_MPI_VI_DestroyVipp(vipp_dev);

_exit:
    if (pContext)
    {
        free(pContext);
    }
    return result;
}

int mpp_sys_init(int log_level)
{
    int ret;

    if (true == is_initialized) {
        return 0;
    }
    is_initialized = true;

    GLogConfig stGLogConfig =
    {
        .FLAGS_logtostderr = 1,
        .FLAGS_colorlogtostderr = 1,
        .FLAGS_stderrthreshold = _GLOG_INFO,
        .FLAGS_minloglevel = _GLOG_INFO,
        .FLAGS_logbuflevel = -1,
        .FLAGS_logbufsecs = 0,
        .FLAGS_max_log_size = 1,
        .FLAGS_stop_logging_if_full_disk = 1,
    };
    strcpy(stGLogConfig.LogDir, "/tmp");
    strcpy(stGLogConfig.InfoLogFileNameBase, "LOG-");
    strcpy(stGLogConfig.LogFileNameExtension, "IPC-");
    if (log_level == 3) {
        stGLogConfig.FLAGS_stderrthreshold = _GLOG_WARN;
        stGLogConfig.FLAGS_minloglevel = _GLOG_WARN;
    } else if (log_level <= 2) {
        stGLogConfig.FLAGS_stderrthreshold = _GLOG_ERROR;
        stGLogConfig.FLAGS_minloglevel = _GLOG_ERROR;
    }

    log_init("tuya_app", &stGLogConfig);

    MPP_SYS_CONF_S stSysConf;
    memset(&stSysConf, 0, sizeof(MPP_SYS_CONF_S));
    stSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&stSysConf);
    ret = AW_MPI_SYS_Init();
    if (ret < 0)
    {
        aloge("fatal error! sys Init failed! ret=%d", ret);
        return -1;
    }
    return 0;
}

int mpp_sys_uninit(void)
{
    if (false == is_initialized) {
        return 0;
    }
    is_initialized = false;
    int ret;
    ret = AW_MPI_SYS_Exit();
    if (ret < 0)
    {
        aloge("sys exit failed!");
        return -1;
    }
    log_quit();
    return 0;
}
