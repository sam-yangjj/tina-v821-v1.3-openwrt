#include <stdio.h>
#include "tuya_error_code.h"
#include "media/mpi_sys.h"
#include "plat_log.h"
#include <zbar.h>

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
#include <time.h>

#define LOG_TAG "demo_video"

#include <linux/videodev2.h>
#include <media/sunxi_camera_v2.h>
#include "AW_VideoInput_API.h"
#include "aw_util.h"

// #define DUMP_YUV

static int max_bitstream_count = SAVE_BITSTREAM_COUNT;
static bool is_in_recognition;
static demo_video_param mparam;
extern int TUYA_IPC_Get_MqttStatus();
static bool is_initialized;


int64_t get_systime()
{
	long long curr;
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &t);
	curr = ((long long)(t.tv_sec)*1000000000LL + t.tv_nsec)/1000LL;
	return (int64_t)curr;
}

static int get_yuv_frame_data(int channel, VideoInputConfig* config, char *qrcode_str, int timeout)
{
	uint64_t pStartTime = 0;    // unit: ms
	uint64_t pTestDuration = 0; // unit: ms
	int ret = 0;
	VideoYuvFrame mYuvFrame;
	int ImgScanret = 0;
#ifdef DUMP_YUV
	FILE *out_yuv_file = NULL;
	unsigned char file_name[160] = {0};

	sprintf(file_name, "/mnt/sdcard/%dx%d.yuv", config->width, config->height);
	out_yuv_file = fopen(file_name, "wb");
	if(out_yuv_file == NULL)
	{
		aloge("fopen failed");
	}
#endif

	if (qrcode_str == NULL)
	{
		aloge("qrcode_str == NULL");
		return -1;
	}

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

	pStartTime = get_systime() / 1000;
	pTestDuration = timeout * 1000;
	while(1)
	{

		if (TUYA_IPC_Get_MqttStatus() == true)
		{
			aloge("Get_MqttStatus true");
			ImgScanret = 3;
			break;
		}

		if ((get_systime() / 1000 - pStartTime) >= pTestDuration)
		{
			aloge("scanner QRCode fail! scan timeout");
			break;
		}

		if (AWVideoInput_Check_Wait_Start(channel)) {
			usleep(200*1000);
			aloge("wait start");
			continue;
		}

		memset(&mYuvFrame, 0, sizeof(VideoYuvFrame));
		ret = AWVideoInput_GetYuvFrame(channel, &mYuvFrame);
		if(ret != 0)
		{
			aloge("Getting yuv failed, continue!");
			continue;
		}
		if (NULL == mYuvFrame.virAddr[0])
		{
			aloge("mpVirAddr[0] is null");
			continue;
		}

		if (is_in_recognition == FALSE)
		{
			is_in_recognition = TRUE;
			// alogd("ret = %d; frame info: w&h = %d, %d; phyAddr = %p, %p, %p, virAddr = %p, %p, %p\n",
			// 		ret, mYuvFrame.widht, mYuvFrame.height,
			// 		mYuvFrame.phyAddr[0], mYuvFrame.phyAddr[1], mYuvFrame.phyAddr[2],
			// 		mYuvFrame.virAddr[0], mYuvFrame.virAddr[1], mYuvFrame.virAddr[2]);

#ifdef DUMP_YUV
			int buf_size = mYuvFrame.widht*mYuvFrame.height*3/2;
			if (out_yuv_file)
				fwrite(mYuvFrame.virAddr[0], 1, buf_size, out_yuv_file);
#endif

			zbar_image_t *image = zbar_image_create();
			if(NULL == image)
			{
				aloge("fatal error! zbar_image_create failed");
				continue;
			}

			unsigned int pWidth = mYuvFrame.widht;
			unsigned int pHeight = mYuvFrame.height;
			zbar_image_set_format(image, *(unsigned long *)"Y800");
			zbar_image_set_size(image, pWidth, pHeight);
			zbar_image_set_data(image, (const void *)mYuvFrame.virAddr[0], pWidth * pHeight, NULL);

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
			is_in_recognition = FALSE;
		}

		AWVideoInput_ReleaseYuvFrame(channel, &mYuvFrame);

		if (1 == ImgScanret || 2 == ImgScanret)
		{
			break;
		}

		usleep(10*1000);
	}
	zbar_image_scanner_destroy(scanner);
#ifdef DUMP_YUV
	if (out_yuv_file != NULL)
		fclose(out_yuv_file);
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
		aloge("scan timeout, mTestDuration:%d", timeout);
		return -2;
	}
}



int qrcode_mode_main(char *p_str, int timeout)
{
	alogd("enter timeout:%d", timeout);
	int ret = 0;
	int i = 0;
	int channelId_0 = 0;

	//解析成argv argc格式
	char input[] = "demo_get_yuv -s0 400x400 -pf 1";
	char *argv[128];
	int argc = 0;

	char *token = strtok(input, " ");
	while (token != NULL) {
		argv[argc++] = token;
		token = strtok(NULL, " ");
	}
	// printf("argc = %d\n", argc);
	// for (int i = 0; i < argc; i++) {
	// 	printf("argv[%d] = %s\n", i, argv[i]);
	// }


	memset(&mparam, 0, sizeof(demo_video_param));
	mparam.c0_encoder_format = -1;
	mparam.pixelformat = RT_PIXEL_NUM;
	mparam.use_vipp_num = 0;
	// strcpy(mparam.OutputFilePath, OUT_PUT_FILE_2);
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
		aloge(" we need more arguments \n");
		PrintDemoUsage();
	}

	check_param(&mparam);
	if(mparam.encoder_num > 0)
		max_bitstream_count = mparam.encoder_num;
	else
		max_bitstream_count = 1;
	channelId_0 = mparam.use_vipp_num;
	VideoInputConfig config_0;
	memset(&config_0, 0, sizeof(VideoInputConfig));
	config_0.channelId = channelId_0;
	config_0.gop     = 30;
	config_0.pixelformat = mparam.pixelformat;

	config_0.venc_video_signal.video_format = DEFAULT;
	switch(mparam.color_space)
	{
		case V4L2_COLORSPACE_JPEG:
		{
			config_0.venc_video_signal.full_range_flag = 1;
			config_0.venc_video_signal.src_colour_primaries = VENC_YCC;
			config_0.venc_video_signal.dst_colour_primaries = VENC_YCC;
			break;
		}
		case V4L2_COLORSPACE_REC709:
		{
			config_0.venc_video_signal.full_range_flag = 1;
			config_0.venc_video_signal.src_colour_primaries = VENC_BT709;
			config_0.venc_video_signal.dst_colour_primaries = VENC_BT709;
			break;
		}
		case V4L2_COLORSPACE_REC709_PART_RANGE:
		{
			config_0.venc_video_signal.full_range_flag = 0;
			config_0.venc_video_signal.src_colour_primaries = VENC_BT709;
			config_0.venc_video_signal.dst_colour_primaries = VENC_BT709;
			break;
		}
		default:
		{
			config_0.venc_video_signal.full_range_flag = 1;
			config_0.venc_video_signal.src_colour_primaries = VENC_BT709;
			config_0.venc_video_signal.dst_colour_primaries = VENC_BT709;
			break;
		}
	}
	config_0.output_mode = OUTPUT_MODE_YUV;
	config_0.width      = mparam.c0_src_w;
	config_0.height     = mparam.c0_src_h;
	config_0.bitrate    = mparam.c0_bitrate;
	config_0.encodeType = mparam.c0_encoder_format;
	config_0.dma_merge_mode = mparam.dma_merge_mode;
	alogd("mparam.dma_merge_mode = %d\n", mparam.dma_merge_mode);
	/* Only RTVI_DMA_STITCH_VERTICAL support insert dma overlay */
	// if (config_0.dma_merge_mode == DMA_STITCH_VERTICAL) {
	// 	if (mparam.dma_overlay_enable) {
	// 		config_0.dma_overlay_ctx.dma_overlay_en = 1;
	// 		if (mparam.dma_overlay_width == 0 || mparam.dma_overlay_height == 0) {
	// 			alogw("mparam.dma_overlay: (%d, %d), set default 1920*336!\n", mparam.dma_overlay_width, mparam.dma_overlay_height);
	// 			config_0.dma_overlay_ctx.overlay_width = 1920;
	// 			config_0.dma_overlay_ctx.overlay_height = 336;
	// 		} else {
	// 			config_0.dma_overlay_ctx.overlay_width = mparam.dma_overlay_width;
	// 			config_0.dma_overlay_ctx.overlay_height = mparam.dma_overlay_height;
	// 		}
	// 		alogd("It will set dma_overlay: (%d, %d)\n", config_0.dma_overlay_ctx.overlay_width,
	// 			config_0.dma_overlay_ctx.overlay_height);
	// 		config_0.dma_overlay_ctx.length = config_0.dma_overlay_ctx.overlay_width * config_0.dma_overlay_ctx.overlay_height
	// 			* 3 / 2;
	// 		config_0.dma_overlay_ctx.overlay_data = (unsigned char *)malloc(config_0.dma_overlay_ctx.length);
	// 		if (config_0.dma_overlay_ctx.overlay_data) {
	// 			create_yuv_for_test_dma_overlay(&config_0.dma_overlay_ctx);
	// 		} else {
	// 			aloge("overlay_data malloc failed!\n");
	// 		}
	// 	} else {
	// 		alogw("It will not set dma overlay, please ensure that the resolution is correct\n");
	// 	}
	// }
	config_0.dma_merge_scaler_cfg.scaler_en = mparam.scaler_en;
	if (config_0.dma_merge_scaler_cfg.scaler_en) {
		config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.width = mparam.dma_merge_scaler_sensorA_width;
		config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.height = mparam.dma_merge_scaler_sensorA_height;
		config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.width = mparam.dma_merge_scaler_sensorB_width;
		config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.height = mparam.dma_merge_scaler_sensorB_height;
		alogd("SensorA: %dx%d, SensorB:  %dx%d\n", config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.width,
			config_0.dma_merge_scaler_cfg.sensorA_scaler_cfg.height, config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.width,
			config_0.dma_merge_scaler_cfg.sensorB_scaler_cfg.height);
	}

	AWVideoInput_Init();

	if(AWVideoInput_Configure(channelId_0, &config_0))
	{
		aloge("config err, exit!");
		goto _exit;
	}
	AWVideoInput_Start(channelId_0, 1);

	ret = get_yuv_frame_data(channelId_0, &config_0, p_str, timeout);
	if ( ret < 0) {
		aloge("get pic failed,%d", ret);
	}

	AWVideoInput_Start(channelId_0, 0);

_exit:
	AWVideoInput_DeInit();
	return ret;
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
