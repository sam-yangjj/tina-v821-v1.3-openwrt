
/*
 ******************************************************************************
 *
 * isp.c
 *
 * Hawkview ISP - isp.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/05/27	VIDEO INPUT
 *
 *****************************************************************************
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /*in order to use pthread_setname_np*/
#endif

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <linux/spi/spidev.h>
#include <pthread.h>

#include "device/isp_dev.h"
#include "isp_dev/tools.h"

#include "isp_events/events.h"
#include "isp_tuning/isp_tuning_priv.h"
#include "isp_math/isp_math_util.h"
#include "isp.h"

#include "iniparser/src/iniparser.h"
#include "include/isp_cmd_intf.h"
#include "include/V4l2Camera/sunxi_camera_v2.h"

#define MEDIA_DEVICE		"/dev/media0"

static struct hw_isp_media_dev media_params;
static struct isp_lib_context isp_ctx[HW_ISP_DEVICE_NUM] = {
	[0] = {
		.isp_ini_cfg = {
			.isp_test_settings = {
				.ae_en = 1,
				.awb_en = 1,
				.wb_en = 1,
				.hist_en = 1,
			},
			.isp_3a_settings = {
				.ae_stat_sel = 1,
				.ae_delay_frame = 0,
				.exp_delay_frame = 1,
				.gain_delay_frame = 1,

				.awb_interval = 2,
				.awb_speed = 32,
				.awb_stat_sel = 1,
				.awb_light_num = 9,
				.awb_light_info = {
					  254,	 256,	104,   256,   256,   256,    50,  1900,    32,	  80,
					  234,	 256,	108,   256,   256,   256,    50,  2500,    32,	  85,
					  217,	 256,	114,   256,   256,   256,    50,  2800,    32,	  90,
					  160,	 256,	153,   256,   256,   256,    50,  4000,    64,	  90,
					  141,	 256,	133,   256,   256,   256,    50,  4100,    64,	 100,
					  142,	 256,	174,   256,   256,   256,    50,  5000,   100,	 100,
					  118,	 256,	156,   256,   256,   256,    50,  5300,    64,	 100,
					  127,	 256,	195,   256,   256,   256,    50,  6500,    64,	 90,
					  115,	 256,	215,   256,   256,   256,    50,  8000,    64,	 80
				},
			},
		},
	},
};
static struct isp_tuning *tuning[HW_ISP_DEVICE_NUM];
static struct event_list events_arr[HW_ISP_DEVICE_NUM];

static int __ae_done(struct isp_lib_context *lib,
			ae_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
	int first_sync = -1;
	int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			*result = isp_ctx[first_sync].ae_entity_ctx.ae_result;
			ISP_LIB_LOG(ISP_LOG_AE, "merge mode : isp%d ae result -> isp%d ae result\n", first_sync, lib->isp_id);
			return 1;
		}
	}
#endif
	FUNCTION_LOG;
	return 0;
}
static int __af_done(struct isp_lib_context *lib,
			af_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
	int first_sync = -1;
	int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			*result = isp_ctx[first_sync].af_entity_ctx.af_result;
			ISP_LIB_LOG(ISP_LOG_AF, "merge mode : isp%d af result -> isp%d af result\n", first_sync, lib->isp_id);
			return 1;
		}
	}
#endif
	FUNCTION_LOG;
	return 0;
}
static int __awb_done(struct isp_lib_context *lib,
			awb_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
    	int first_sync = -1;
    	int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			*result = isp_ctx[first_sync].awb_entity_ctx.awb_result;
			ISP_LIB_LOG(ISP_LOG_AWB, "slave mode : isp%d awb result -> isp%d awb result\n", first_sync, lib->isp_id);
			return 1;
		}
	}
#endif
	FUNCTION_LOG;
	return 0;
}
static int __afs_done(struct isp_lib_context *lib,
			afs_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
    	int first_sync = -1;
	int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			*result = isp_ctx[first_sync].afs_entity_ctx.afs_result;
			ISP_LIB_LOG(ISP_LOG_AFS, "merge mode : isp%d afs result -> isp%d afs result\n", first_sync, lib->isp_id);
			return 1;
		}
	}
#endif
	FUNCTION_LOG;
	return 0;
}

static int __md_done(struct isp_lib_context *lib,
			md_result_t *result __attribute__((__unused__)))
{
	FUNCTION_LOG;
	return 0;
}

static int __pltm_done(struct isp_lib_context *lib,
			pltm_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
        int first_sync = -1;
        int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			*result = isp_ctx[first_sync].pltm_entity_ctx.pltm_result;
			ISP_LIB_LOG(ISP_LOG_PLTM, "merge mode : isp%d pltm result -> isp%d pltm result\n", first_sync, lib->isp_id);
			return 1;
		}
	}
#endif
	FUNCTION_LOG;
	return 0;
}

static int __gtm_done(struct isp_lib_context *lib,
			gtm_result_t *result __attribute__((__unused__)))
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i;
	int first_sync = -1;
	int sync_isp_flag = 0;
	int count = (media_params.isp_sync_mode >> 16) & 0xff;
	if (count > 1) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (media_params.isp_sync_mode & (0x1 << i)) {
				/* first sync isp index */
				if (first_sync == -1)
					first_sync = i;

				if (i == lib->isp_id)
					sync_isp_flag = 1;
			}
		}
		if (sync_isp_flag && lib->isp_id != first_sync) {
			result->ldci_merge_mode = 1;
			memcpy(&lib->module_cfg.drc_cfg.drc_table[0], &isp_ctx[first_sync].module_cfg.drc_cfg.drc_table[0], ISP_DRC_TBL_SIZE*sizeof(unsigned short));
			ISP_LIB_LOG(ISP_LOG_GTM, "merge mode : isp%d drc_table -> isp%d drc_table\n", first_sync, lib->isp_id);
			return 1;
		}
	} else {
		result->ldci_merge_mode = 0;
	}
#else
	result->ldci_merge_mode = 0;
#endif
	FUNCTION_LOG;
	return 0;
}

static int __awb_limit(struct isp_lib_context *lib,
			awb_result_t *result __attribute__((__unused__)))
{
	int isp_base = lib->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[0];
	int rgain_min, bgain_min, rgain_max, bgain_max;

	rgain_min = isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.r_gain * lib->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[1] >> 8;
	bgain_min = isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.b_gain * lib->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[2] >> 8;
	rgain_max = isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.r_gain * lib->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[3] >> 8;
	bgain_max = isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.b_gain * lib->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[4] >> 8;
	if (isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.r_gain < 256 || isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.b_gain < 256) {
		ISP_LIB_LOG(ISP_LOG_AWB, "master rgain/bgain %d %d is invaild.\n", isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.r_gain,
			isp_ctx[isp_base].awb_entity_ctx.awb_result.wb_gain_output.b_gain);
		return 0;
	}

	result->wb_gain_output.r_gain = clamp(result->wb_gain_output.r_gain, rgain_min, rgain_max);
	result->wb_gain_output.b_gain = clamp(result->wb_gain_output.b_gain, bgain_min, bgain_max);

	if (result->wb_gain_output.r_gain < 256)
		result->wb_gain_output.r_gain = 256;
	if (result->wb_gain_output.b_gain < 256)
		result->wb_gain_output.b_gain = 256;

	ISP_LIB_LOG(ISP_LOG_AWB, "isp%d awb_limit: rgain/bgain=%d, %d ---- isp_base=%d, rgain_min=%d, rgain_max=%d, bgain_min=%d, bgain_max=%d\n",
		lib->isp_id, result->wb_gain_output.r_gain, result->wb_gain_output.b_gain, isp_base, rgain_min, rgain_max, bgain_min, bgain_max);

	FUNCTION_LOG;
	return 0;
}

#if ISP_LIB_USE_FLASH
void __isp_flash_open(struct hw_isp_device *isp)
{
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	ae_result_t *ae_result = &isp_gen->ae_entity_ctx.ae_result;

	isp_gen->ae_settings.take_pic_start_cnt = isp_gen->ae_frame_cnt;
	ISP_DEV_LOG(ISP_LOG_FLASH, "%s: TORCH_ON, ev_set.ev_idx:%d, take_pic_start_cnt:%d, flash_delay:%d.\n",
		__FUNCTION__, ae_result->sensor_set.ev_set.ev_idx,
		isp_gen->ae_settings.take_pic_start_cnt,
		isp_gen->isp_ini_cfg.isp_tunning_settings.flash_delay_frame);

	ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_TORCH;
	isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_TORCH);
	isp_gen->ae_settings.flash_open = 1;
}

void isp_flash_update_status(struct hw_isp_device *isp)
{
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	isp_ae_entity_context_t *isp_ae_cxt = &isp_gen->ae_entity_ctx;
	ae_result_t *ae_result = &isp_gen->ae_entity_ctx.ae_result;
	int flash_gain = isp_gen->isp_ini_cfg.isp_tunning_settings.flash_gain;
	int flash_delay = isp_gen->isp_ini_cfg.isp_tunning_settings.flash_delay_frame;
	int auto_focus_en = isp_gen->isp_ini_cfg.isp_test_settings.af_en;
	enum auto_focus_status af_status = isp_gen->af_entity_ctx.af_result.af_status_output;
	static int pre_flash_end_cnt = 0;
	int ev_idx_flash = 0;

	switch (isp_gen->ae_settings.flash_mode) {
	case FLASH_MODE_ON:
		if (isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_FLASH) {
			if (isp_gen->ae_settings.flash_open == 0) {
				isp_gen->ae_settings.flash_switch_flag = true;
				__isp_flash_open(isp);
				/*when flash open, awb speedup*/
				isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_interval = 1;
				isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_speed = 10;
				isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_green_zone_dist = 128;
				isp_gen->af_entity_ctx.af_param->auto_focus_trigger = 1;
				isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
			} else {
				if (auto_focus_en == 1 && pre_flash_end_cnt == 0) {
					/* when af_en = 1, should wait for af end */
					if (isp_gen->ae_frame_cnt >= (isp_gen->ae_settings.take_pic_start_cnt + flash_delay)
						&& (af_status == AUTO_FOCUS_STATUS_REACHED || af_status == AUTO_FOCUS_STATUS_FAILED))
						pre_flash_end_cnt = isp_gen->ae_frame_cnt;

				} else if (auto_focus_en == 0 && pre_flash_end_cnt == 0) {
					if (isp_gen->ae_frame_cnt == (isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
						pre_flash_end_cnt = isp_gen->ae_frame_cnt;
				}
				if (isp_gen->ae_frame_cnt == pre_flash_end_cnt) {
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
					isp_gen->ae_settings.flash_switch_flag = false;

					isp_gen->ae_settings.exposure_lock = true;
					isp_gen->af_settings.focus_lock = true;
					isp_gen->awb_settings.white_balance_lock = true;

					if (ae_result->ae_flash_ev_cumul >= 100)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul < 100 && ae_result->ae_flash_ev_cumul >= flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul >= -500 && ae_result->ae_flash_ev_cumul < flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/256;
					}
					else
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/512;
					}
					ev_idx_flash = clamp(ev_idx_flash, 1, ae_result->sensor_set.ev_idx_max);

					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_OFF, ae_flash_ev_cumul:%d, ev_idx_expect:%d, ev_idx_flash:%d, flash_gain:%d.\n",
						__FUNCTION__,	ae_result->ae_flash_ev_cumul,
						ae_result->sensor_set.ev_idx_expect, ev_idx_flash,
						isp_gen->isp_ini_cfg.isp_tunning_settings.flash_gain);

					isp_ae_cxt->ae_param->ae_pline_index = ev_idx_flash;
					isp_ae_set_params_helper(isp_ae_cxt, ISP_AE_SET_EXP_IDX);
				}
				else if (isp_gen->ae_frame_cnt == (1 + pre_flash_end_cnt))
				{
					//isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 1;
					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_ON.\n", __FUNCTION__);
					isp_gen->ae_settings.flash_switch_flag = true;
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_FLASH;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_FLASH);
				} /* because sensor exp delay n+1/n+2, so we should delay 3 frames report hal */
				else if (isp_gen->ae_frame_cnt == (3 + pre_flash_end_cnt))
				{
					isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 1;
				}
				else if (isp_gen->ae_frame_cnt == (7 + pre_flash_end_cnt))
				{
					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_OFF.\n", __FUNCTION__);

					isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 3;
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
					/*when flash close, reset awb speed*/
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_interval = isp_gen->isp_ini_cfg.isp_3a_settings.awb_interval;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_speed = isp_gen->isp_ini_cfg.isp_3a_settings.awb_speed;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_green_zone_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_green_zone_dist;
					isp_gen->ae_settings.exposure_lock = false;
					isp_gen->af_settings.focus_lock = false;
					isp_gen->awb_settings.white_balance_lock = false;
				}
			}
		} else { /* touch focus */
			if (auto_focus_en == 1) {
				if (isp_gen->ae_settings.flash_open == 0) {
					isp_gen->ae_settings.flash_switch_flag = true;
					__isp_flash_open(isp);
					isp_gen->af_entity_ctx.af_param->auto_focus_trigger = 1;
					isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
				} else {
					if (isp_gen->ae_frame_cnt >= (isp_gen->ae_settings.take_pic_start_cnt + flash_delay)
						&& (af_status == AUTO_FOCUS_STATUS_REACHED || af_status == AUTO_FOCUS_STATUS_FAILED)) {
						ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
						isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
						isp_gen->ae_settings.flash_open = 0;
						isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 3;
					}
				}
			}
		}
		break;
	case FLASH_MODE_AUTO:
		if (isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_FLASH) {
			if (isp_gen->ae_settings.flash_open == 0) {
				if (ae_result->sensor_set.ev_set.ev_idx > (ae_result->sensor_set.ev_idx_max - 20)) {
					ISP_LIB_LOG(ISP_LOG_FLASH, "ev_idx is %d, ev_idx_max is %d\n",
							ae_result->sensor_set.ev_set.ev_idx, ae_result->sensor_set.ev_idx_max);
					isp_gen->ae_settings.flash_switch_flag = true;
					__isp_flash_open(isp);
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_interval = 1;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_speed = 10;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_green_zone_dist = 128;
					isp_gen->af_entity_ctx.af_param->auto_focus_trigger = 1;
					isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
				} else {
					isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 2;
				}
			} else {
				if (auto_focus_en == 1 && pre_flash_end_cnt == 0) {
					/* when af_en = 1, should wait for af end */
					if (isp_gen->ae_frame_cnt >= (isp_gen->ae_settings.take_pic_start_cnt + flash_delay)
						&& (af_status == AUTO_FOCUS_STATUS_REACHED || af_status == AUTO_FOCUS_STATUS_FAILED))
						pre_flash_end_cnt = isp_gen->ae_frame_cnt;

				} else if (auto_focus_en == 0 && pre_flash_end_cnt == 0) {
					if (isp_gen->ae_frame_cnt == (isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
						pre_flash_end_cnt = isp_gen->ae_frame_cnt;
				}
				if (isp_gen->ae_frame_cnt == pre_flash_end_cnt) {
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
					isp_gen->ae_settings.flash_switch_flag = false;

					isp_gen->ae_settings.exposure_lock = true;
					isp_gen->af_settings.focus_lock = true;
					isp_gen->awb_settings.white_balance_lock = true;

					if (ae_result->ae_flash_ev_cumul >= 100)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul < 100 && ae_result->ae_flash_ev_cumul >= flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul >= -500 && ae_result->ae_flash_ev_cumul < flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/256;
					}
					else
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/512;
					}
					ev_idx_flash = clamp(ev_idx_flash, 1, ae_result->sensor_set.ev_idx_max);

					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_OFF, ae_flash_ev_cumul:%d, ev_idx_expect:%d, ev_idx_flash:%d, flash_gain:%d.\n",
						__FUNCTION__,	ae_result->ae_flash_ev_cumul,
						ae_result->sensor_set.ev_idx_expect, ev_idx_flash,
						isp_gen->isp_ini_cfg.isp_tunning_settings.flash_gain);

					isp_ae_cxt->ae_param->ae_pline_index = ev_idx_flash;
					isp_ae_set_params_helper(isp_ae_cxt, ISP_AE_SET_EXP_IDX);
				}
				else if (isp_gen->ae_frame_cnt == (1 + pre_flash_end_cnt))
				{
					//isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 1;
					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_ON.\n", __FUNCTION__);
					isp_gen->ae_settings.flash_switch_flag = true;
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_FLASH;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_FLASH);
				} /* because sensor exp delay n+1/n+2, so we should delay 3 frames report hal */
				else if (isp_gen->ae_frame_cnt == (3 + pre_flash_end_cnt))
				{
					isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 1;
				}
				else if (isp_gen->ae_frame_cnt == (7 + pre_flash_end_cnt))
				{
					ISP_LIB_LOG(ISP_LOG_FLASH, "%s: FLASH_OFF.\n", __FUNCTION__);

					isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 3;
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
					/*when flash close, reset awb speed*/
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_interval = isp_gen->isp_ini_cfg.isp_3a_settings.awb_interval;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_speed = isp_gen->isp_ini_cfg.isp_3a_settings.awb_speed;
					isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_green_zone_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_green_zone_dist;
					isp_gen->ae_settings.exposure_lock = false;
					isp_gen->af_settings.focus_lock = false;
					isp_gen->awb_settings.white_balance_lock = false;
				}
			}
		} else {  /* touch focus */
			if (auto_focus_en == 1) {
				if (isp_gen->ae_settings.flash_open == 0) {
					if (ae_result->sensor_set.ev_set.ev_idx > (ae_result->sensor_set.ev_idx_max - 20)) {
						ISP_LIB_LOG(ISP_LOG_FLASH, "%s: ev_idx is %d, ev_idx_max is %d\n", __FUNCTION__,
								ae_result->sensor_set.ev_set.ev_idx, ae_result->sensor_set.ev_idx_max);
						isp_gen->ae_settings.flash_switch_flag = true;
						__isp_flash_open(isp);
						isp_gen->af_entity_ctx.af_param->auto_focus_trigger = 1;
						isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
					} else {
						isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 3;
					}
				} else {
					if (isp_gen->ae_frame_cnt >= (isp_gen->ae_settings.take_pic_start_cnt + flash_delay)
						&& (af_status == AUTO_FOCUS_STATUS_REACHED || af_status == AUTO_FOCUS_STATUS_FAILED)) {
						ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
						isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
						isp_gen->ae_settings.flash_open = 0;
						isp_gen->image_params.isp_image_params.image_para.bits.flash_ok = 3;
					}
				}
			}
		}
		break;
	case FLASH_MODE_TORCH:
		ISP_LIB_LOG(ISP_LOG_FLASH, "%s: TORCH_ON, FLASH_MODE TORCH_ON.\n", __FUNCTION__);
		isp_gen->ae_settings.flash_switch_flag = true;
		ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_TORCH;
		if (isp_gen->ae_settings.flash_open == 0) {
			isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_TORCH);
			isp_gen->ae_settings.flash_open = 1;
		}
		break;
	case FLASH_MODE_OFF:
		if (isp_gen->ae_settings.flash_switch_flag) {
			ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
			isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
			isp_gen->ae_settings.flash_switch_flag = false;
			isp_gen->ae_settings.flash_open = 0;
		}
		isp_gen->ae_settings.exposure_lock = false;
		isp_gen->af_settings.focus_lock = false;
		isp_gen->awb_settings.white_balance_lock = false;
		pre_flash_end_cnt = 0;
		break;
	case FLASH_MODE_RED_EYE:
	case FLASH_MODE_NONE:
		break;
	default:
		ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
		isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
		break;
	}
}
#endif

static void isp_ctx_get_sensor_info(struct hw_isp_device *isp)
{
	struct isp_lib_context *ctx;
	struct sensor_temp temp;
	struct sensor_flip flip;
	static HW_U8 sync_debug_info_enable = 1;

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL)
		return;

	if (ctx->flip_info.enable && (ctx->alg_frame_cnt % max(ctx->sensor_info.fps_fixed, 1) == 0)) {//get sensor flip
		if (isp_sensor_get_flip(isp, &flip) < 0) {
			ctx->flip_info.enable = 0;
		} else {
			if (ctx->flip_info.hflip != flip.hflip || ctx->flip_info.vflip != flip.vflip) {
				ctx->flip_info.hflip = flip.hflip;
				ctx->flip_info.vflip = flip.vflip;
				ISP_PRINT("sensor set hflip : %d, vflip : %d\n", ctx->flip_info.hflip, ctx->flip_info.vflip);
			}
		}
	}

	if (ctx->temp_info.enable) {//get sensor temperature
		if (isp_sensor_get_temp(isp, &temp) < 0) {
			ctx->temp_info.enable = 0;
		} else {
			ctx->sensor_info.temperature = temp.temp;
		}
	}

	if (sync_debug_info_enable) {
		isp_get_debug_info(isp);
		if (isp_sync_debug_info(isp, &ctx->debug_param_info) < 0) {
			ISP_WARN("isp debugfs node not support!!!\n");
			sync_debug_info_enable = 0;
		} else {
			sync_debug_info_enable = 1;
		}
	}
}

static HW_S32 __isp_frame_process(struct hw_isp_device *isp, struct isp_lib_context *ctx)
{
	static HW_U8 ir_flag_change_frame_cnt[HW_ISP_DEVICE_NUM] = {0};
	static HW_S32 tdnf_level_save[HW_ISP_DEVICE_NUM] = {0};
	struct isp_module_config *mod_cfg = &ctx->module_cfg;

	if (!isp || !ctx) {
		ISP_ERR("isp device or ctx is NULL!!!\n");
		return -1;
	}

	// MIPI_SWITCH PROCESS
	if (ctx->switch_info.mipi_switch_enable) {
		ctx->awb_settings.white_balance_lock = true;
		ctx->ae_settings.exposure_lock = true;
	} else {
		ctx->awb_settings.white_balance_lock = false;
		ctx->ae_settings.exposure_lock = false;
	}

	// ir to color, disable d3d and enable it later
	if (ctx->isp_ir_flag_last == ISP_IR_MODE && ctx->isp_ir_flag == ISP_COLOR_MODE) {
		if (ir_flag_change_frame_cnt[isp->id] == FRAME_ID_1) {
			ir_flag_change_frame_cnt[isp->id] = 0;
			ctx->tune.tdf_level = tdnf_level_save[isp->id];
			ISP_PRINT("ISP%d, tdf_level = %d\n", isp->id, ctx->tune.tdf_level);
			ISP_PRINT("ISP%d, Eanable_d3d....\n", isp->id);
			ctx->isp_ir_flag_last = ctx->isp_ir_flag;
		} else {
			tdnf_level_save[isp->id] = ctx->tune.tdf_level;
			ctx->tune.tdf_level = 0;
			ISP_PRINT("ISP%d, tdf_level = %d, tdnf_level_save = %d\n",
				isp->id, ctx->tune.tdf_level, tdnf_level_save[isp->id]);
			ISP_PRINT("ISP%d, Disable_d3d....\n", isp->id);
			ir_flag_change_frame_cnt[isp->id]++;
		}
	} else {
		ctx->isp_ir_flag_last = ctx->isp_ir_flag;
	}

	// MIPI_SWITCH PROCESS
	if (ctx->switch_info.mipi_switch_enable) {
		isp_sensor_mipi_switch_comp_process(ctx->isp_id, ctx, &ctx->switch_info.mipi_switch_info);
		if (ctx->switch_info.frame_cnt++ >= (MIPI_SWITCH_FRAME_SET_OFFSET + 1)) {
			ctx->switch_info.frame_cnt = 0;
		}
	} else {
		ctx->switch_info.frame_cnt = 0;
	}
	return 0;
}
static void __isp_stats_process(struct hw_isp_device *isp, const void *buffer)
{
	struct isp_lib_context *ctx;
	struct isp_table_reg_map reg;
	struct sensor_exp_gain exp_gain;
	ae_result_t *ae_result = NULL;
	awb_result_t *awb_result = NULL;
	af_result_t *af_result = NULL;
	const void *buffer0 = NULL, *buffer1 = NULL;
	int i;
	int sync_isp_flag = 0;

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL)
		return;

	isp_ctx_get_sensor_info(isp);

	ctx->isp_stat_buf = buffer;
#if (HW_ISP_DEVICE_NUM > 1)
	if (media_params.isp_sync_mode) {
		if (((media_params.isp_sync_mode >> 16) & 0xff) > 1) {
			for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
				if (media_params.isp_sync_mode & (0x1 << i)) {
					if (buffer0 == NULL)
						buffer0 = isp_ctx[i].isp_stat_buf;
					else
						buffer1 = isp_ctx[i].isp_stat_buf;

					if (i == isp->id)
						sync_isp_flag = 1;
				}
			}
			if (sync_isp_flag && buffer0 && buffer1) {
				isp_ctx_stats_prepare_sync(ctx, buffer0, buffer1);
			} else {
				isp_ctx_stats_prepare(ctx, ctx->isp_stat_buf);
			}
		} else {
			isp_ctx_stats_prepare(ctx, ctx->isp_stat_buf);
		}
	} else {
		isp_ctx_stats_prepare(ctx, ctx->isp_stat_buf);
	}
#else
	isp_ctx_stats_prepare(ctx, ctx->isp_stat_buf);
#endif
	isp_stat_save_run(ctx);

	isp_ini_tuning_run(isp);

	__isp_frame_process(isp, ctx);
	FUNCTION_LOG;

	isp_ctx_front_algo_run(ctx);
	FUNCTION_LOG;

	af_result = &ctx->af_entity_ctx.af_result;
	if (ctx->isp_ini_cfg.isp_test_settings.af_en || ctx->isp_ini_cfg.isp_test_settings.isp_test_focus) {
		if (af_result->last_code_output != af_result->real_code_output) {
#if (HW_ISP_DEVICE_NUM > 1)
			/*isp0 and isp1 are opened and have same head sensor, so we only use isp0 control af*/
			if (isp->id == 0 && media_params.isp_dev[0] != NULL && media_params.isp_dev[1] != NULL &&
			    !strcmp(media_params.isp_dev[0]->sensor.info.name, media_params.isp_dev[1]->sensor.info.name)) {
				ISP_DEV_LOG(ISP_LOG_ISP, "isp0 and isp1 are opened and have same head, so we only use isp0 to do af!\n");
			} else {
				isp_act_set_pos(isp, af_result->real_code_output);
			}
#else
			isp_act_set_pos(isp, af_result->real_code_output);
#endif
			af_result->last_code_output = af_result->real_code_output;
		}
		ISP_DEV_LOG(ISP_LOG_ISP, "set sensor pos real_code_output: %d.\n", af_result->real_code_output);
	}

#if ISP_LIB_USE_FLASH
#if (HW_ISP_DEVICE_NUM > 1)
	/*isp0 and isp1 are opened and have same head sensor, so we only use isp0 control flash*/
	if (isp->id == 0 && media_params.isp_dev[0] != NULL && media_params.isp_dev[1] != NULL &&
	    !strcmp(media_params.isp_dev[0]->sensor.info.name, media_params.isp_dev[1]->sensor.info.name)) {
		ISP_DEV_LOG(ISP_LOG_ISP, "isp0 and isp1 are opened and have same head, so we only use isp0 to set flash!\n");
	} else {
		isp_flash_update_status(isp);
	}
#else
	isp_flash_update_status(isp);
#endif
#endif

	ae_result = &ctx->ae_entity_ctx.ae_result;
	awb_result = &ctx->awb_entity_ctx.awb_result;
	exp_gain.exp_val = ae_result->sensor_set.ev_set_curr.ev_sensor_exp_line;
	exp_gain.exp_mid_val = ae_result->sensor_set_short.ev_set_curr.ev_sensor_exp_line;
	exp_gain.gain_val = ae_result->sensor_set.ev_set_curr.ev_analog_gain >> 4;
	exp_gain.gain_mid_val = ae_result->sensor_set_short.ev_set_curr.ev_analog_gain >> 4;
	if (awb_result->wb_gain_output.gr_gain && awb_result->wb_gain_output.gb_gain) {
		exp_gain.r_gain = awb_result->wb_gain_output.r_gain * 256 / awb_result->wb_gain_output.gr_gain;
		exp_gain.b_gain = awb_result->wb_gain_output.b_gain * 256 / awb_result->wb_gain_output.gb_gain;
	} else {
		exp_gain.r_gain = 256;
		exp_gain.b_gain = 256;
	}
	exp_gain.r_gain |= (isp->id << 16);
	exp_gain.b_gain |= (isp->id << 16);

#if (HW_ISP_DEVICE_NUM > 1)
	/*isp0 and isp1 are opened and have same head sensor, so we only use isp0 control ae*/
	if (isp->id == 1 && media_params.isp_dev[0] != NULL && media_params.isp_dev[1] != NULL &&
	    !strcmp(media_params.isp_dev[0]->sensor.info.name, media_params.isp_dev[1]->sensor.info.name)) {
		ISP_DEV_LOG(ISP_LOG_ISP, "isp0 and isp1 are opened and have same head, so we only use isp0 to do ae!\n");
	} else {
		isp_sensor_set_exp_gain(isp, &exp_gain);
	}
#else
	isp_sensor_set_exp_gain(isp, &exp_gain);
#endif
	FUNCTION_LOG;

	isp_ctx_rear_algo_run(ctx);

#if (HW_ISP_DEVICE_NUM > 1)
	if (((media_params.isp_sync_mode >> 16) & 0xff) > 1) {
		ctx->module_cfg.rgb2yuv = isp_ctx[0].module_cfg.rgb2yuv;
		ctx->module_cfg.bdnf_cfg = isp_ctx[0].module_cfg.bdnf_cfg;
		ctx->module_cfg.tdf_cfg = isp_ctx[0].module_cfg.tdf_cfg;
		ctx->module_cfg.sharp_cfg = isp_ctx[0].module_cfg.sharp_cfg;
		ctx->module_cfg.gamma_cfg = isp_ctx[0].module_cfg.gamma_cfg;
	}
#endif

	FUNCTION_LOG;

	isp_log_save_run(ctx);

	reg.addr = ctx->load_reg_base;
	reg.size = ISP_LOAD_DRAM_SIZE;
	isp_set_load_reg(isp, &reg);
}

static void __isp_fsync_process(struct hw_isp_device *isp, struct v4l2_event *event)
{
	struct isp_lib_context *ctx = isp_dev_get_ctx(isp);

	if(ctx->sensor_info.color_space != event->u.data[1]) {
		ctx->sensor_info.color_space = event->u.data[1];
		ctx->isp_3a_change_flags |= ISP_SET_HUE;
	}

	isp_lib_log_param = (event->u.data[3] << 8) | event->u.data[2] | ctx->isp_ini_cfg.isp_test_settings.isp_log_param;

	if (event->u.data[4] == 1)
		ctx->gtm_entity_ctx.gtm_param->ldci_video_sel = ISP_LDCI_VIDEO_KERNEL;
}

static void isp_ctrl_process_run(struct isp_lib_context *isp_gen, HW_U32 ctrl_id, int value)
{
	HW_S32 iso_qmenu[] = { 100, 200, 400, 800, 1600, 3200, 6400};
	HW_S32 exp_bias_qmenu[] = { -4, -3, -2, -1, 0, 1, 2, 3, 4, };

	switch(ctrl_id) {
	case V4L2_CID_BRIGHTNESS:
		isp_s_brightness(isp_gen, value);
		break;
	case V4L2_CID_CONTRAST:
		isp_s_contrast(isp_gen, value);
		break;
	case V4L2_CID_SATURATION:
		isp_s_saturation(isp_gen, value);
		break;
	case V4L2_CID_HUE:
		isp_s_hue(isp_gen, value);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		isp_s_auto_white_balance(isp_gen, value);
		break;
	case V4L2_CID_EXPOSURE:
		isp_s_exposure(isp_gen, value);
		break;
	case V4L2_CID_AUTOGAIN:
		isp_s_auto_gain(isp_gen, value);
		break;
	case V4L2_CID_GAIN:
		isp_s_gain(isp_gen, value);
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		isp_s_power_line_frequency(isp_gen, value);
		break;
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		isp_s_white_balance_temperature(isp_gen, value);
		break;
	case V4L2_CID_SHARPNESS:
		isp_s_sharpness(isp_gen, value);
		break;
	case V4L2_CID_AUTOBRIGHTNESS:
		isp_s_auto_brightness(isp_gen, value);
		break;
	case V4L2_CID_BAND_STOP_FILTER:
		isp_s_band_stop_filter(isp_gen, value);
		break;
	case V4L2_CID_ILLUMINATORS_1:
		isp_s_illuminators_1(isp_gen, value);
		break;
	case V4L2_CID_ILLUMINATORS_2:
		isp_s_illuminators_2(isp_gen, value);
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		isp_s_exposure_auto(isp_gen, value);
		break;
	case V4L2_CID_EXPOSURE_ABSOLUTE:
		isp_s_exposure_absolute(isp_gen, value);
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		isp_s_focus_absolute(isp_gen, value);
		break;
	case V4L2_CID_FOCUS_RELATIVE:
		isp_s_focus_relative(isp_gen, value);
		break;
	case V4L2_CID_FOCUS_AUTO:
		isp_s_focus_auto(isp_gen, value);
		break;
	case V4L2_CID_AUTO_EXPOSURE_BIAS:
		isp_s_auto_exposure_bias(isp_gen, exp_bias_qmenu[value]);
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		isp_s_auto_n_preset_white_balance(isp_gen, value);
		break;
	case V4L2_CID_ISO_SENSITIVITY:
		isp_s_iso_sensitivity(isp_gen, iso_qmenu[value]);
		break;
	case V4L2_CID_ISO_SENSITIVITY_AUTO:
		isp_s_iso_sensitivity_auto(isp_gen, value);
		break;
	case V4L2_CID_EXPOSURE_METERING:
		isp_s_ae_metering_mode(isp_gen, value);
		break;
	case V4L2_CID_SCENE_MODE:
		isp_s_scene_mode(isp_gen, value);
		break;
	case V4L2_CID_3A_LOCK:
		//isp_s_3a_lock(isp_gen, value);
		break;
	case V4L2_CID_AUTO_FOCUS_START:
		isp_s_auto_focus_start(isp_gen, value);
		break;
	case V4L2_CID_AUTO_FOCUS_STOP:
		isp_s_auto_focus_stop(isp_gen, value);
		break;
	case V4L2_CID_AUTO_FOCUS_RANGE:
		isp_s_auto_focus_range(isp_gen, value);
		break;
	case V4L2_CID_TAKE_PICTURE:
		isp_s_take_picture(isp_gen, value);
		break;
	case V4L2_CID_FLASH_LED_MODE:
		isp_s_flash_mode(isp_gen, value);
		break;
	case V4L2_CID_FLASH_LED_MODE_V1:
		isp_s_flash_mode_v1(isp_gen, value);
		break;
	default:
		ISP_ERR("Unknown ctrl.\n");
		break;
	}
}

void __isp_ctrl_process(struct hw_isp_device *isp, struct v4l2_event *event)
{
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	int i, mode, count = (media_params.isp_sync_mode >> 16) & 0xff;

	if (isp_gen == NULL)
		return;

	if (count > 1) {
		mode = media_params.isp_sync_mode & 0xffff;
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (mode & (0x1 << i)) {
				if (media_params.isp_use_cnt[i] == 0) {
					ISP_ERR("isp%d is not used\n", i);
					return;
				}
				isp = media_params.isp_dev[i];
				if (!isp) {
					ISP_ERR("isp%d device is NULL!\n", i);
					return;
				}
				isp_gen = isp_dev_get_ctx(isp);
				isp_ctrl_process_run(isp_gen, event->id, event->u.ctrl.value);
			}
		}
	} else {
		isp_ctrl_process_run(isp_gen, event->id, event->u.ctrl.value);
	}
}

static void __isp_stream_off(struct hw_isp_device *isp __attribute__((__unused__)))
{
	if ((isp->id >= HW_ISP_DEVICE_NUM) || (isp->id == -1))
		ISP_ERR("ISP ID is invalid, __isp_stream_off failed!\n");

	events_stop(&events_arr[isp->id]);
}

static void __isp_monitor_fd(int id, int fd, enum hw_isp_event_type type,
			      void(*callback)(void *), void *priv)
{
	events_monitor_fd(&events_arr[id], fd, type, callback, priv);
}

static void __isp_unmonitor_fd(int id, int fd)
{
	events_unmonitor_fd(&events_arr[id], fd);
}

static struct isp_ctx_operations isp_ctx_ops = {
	.ae_done = __ae_done,
	.af_done = __af_done,
	.awb_done = __awb_done,
	.afs_done = __afs_done,
	.md_done = __md_done,
	.pltm_done = __pltm_done,
	.gtm_done = __gtm_done,

	.awb_limit = __awb_limit,
};

static struct isp_dev_operations isp_dev_ops = {
	.stats_ready = __isp_stats_process,
	.fsync = __isp_fsync_process,
	.stream_off = __isp_stream_off,
	.ctrl_process = __isp_ctrl_process,
	.monitor_fd = __isp_monitor_fd,
	.unmonitor_fd = __isp_unmonitor_fd,
};

static void *__isp_thread(void *arg)
{
	int ret = 0;
	struct hw_isp_device *isp = (struct hw_isp_device *)arg;

	if ((isp->id >= HW_ISP_DEVICE_NUM) || (isp->id == -1))
		ISP_ERR("ISP ID is invalid, isp_run failed!\n");

	ret = isp_dev_start(isp);
	if (ret < 0)
		goto end;
	if (events_loop(&events_arr[isp->id]))
		goto end;
end:
	isp_dev_stop(isp);
	return NULL;
}

int isp_set_sync(int mode)
{
#if (HW_ISP_DEVICE_NUM > 1)
	int i, count = 0;
	for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
		if (mode & (0x1 << i)) {
			count++;
		}
	}
	media_params.isp_sync_mode = (count << 16) | (mode & 0xffff);
	ISP_PRINT("ISP Set Sync Mode = 0x%x, isp_sync_mode = 0x%x\n", mode, media_params.isp_sync_mode);
#else
	media_params.isp_sync_mode = 0;
#endif
	return 0;
}

int isp_set_ldci_source(int dev_id, int mode)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param == NULL) {
		ISP_ERR("Gtm is not initialized!\n");
		return -1;
	}

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_video_sel == ISP_LDCI_VIDEO_KERNEL) {
		ISP_ERR("LDCI video sel is ISP_LDCI_VIDEO_KERNEL, cannot set other mode\n");
		return -1;
	}

	if (!mode) {
		isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_video_sel = ISP_LDCI_VIDEO_INTERNAL;
		ISP_PRINT("Set LDCI Source -> ISP_LDCI_VIDEO_INTERNAL \n");
	} else {
		isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_video_sel = ISP_LDCI_VIDEO_EXTERNAL;
		if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf == NULL) {
			/* ldci_external_buf free in gtm */
			isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf = (HW_U8 *)malloc(LDCI_PIC_WIDTH * LDCI_PIC_HEIGHT);
			if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf == NULL) {
				ISP_ERR("alloc ldci_external_buf err!");
				return -1;
			}
		}
		ISP_PRINT("Set LDCI Source -> ISP_LDCI_VIDEO_EXTERNAL \n");
	}
	return 0;
}

int isp_set_ldci_frame(int dev_id, ldci_frame_config_t *frame_params)
{
	HW_U32 i, j;
	HW_U8 subsample_en;
	HW_U8 *dst = NULL;
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param == NULL) {
		ISP_ERR("Gtm is not initialized!\n");
		return -1;
	}

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_video_sel == ISP_LDCI_VIDEO_INTERNAL) {
		ISP_ERR("LDCI video sel is ISP_LDCI_VIDEO_INTERNAL, cannot set frame to ldci\n");
		return -1;
	}

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_video_sel == ISP_LDCI_VIDEO_KERNEL) {
		ISP_ERR("LDCI video sel is ISP_LDCI_VIDEO_KERNEL, cannot set frame to ldci\n");
		return -1;
	}

	if ((frame_params == NULL) || (frame_params->buf == NULL) || (frame_params->size < LDCI_PIC_WIDTH*LDCI_PIC_HEIGHT)) {
		ISP_ERR("Invaild ldci frame info option ! (size = %d, %dx%d, buf = %p)\n", frame_params->size, frame_params->width, frame_params->height, frame_params->buf);
		return -1;
	}

	if (isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf == NULL) {
		ISP_ERR("ldci_external_buf = NULL!\n");
		return -1;
	}

	if (frame_params->size == LDCI_PIC_WIDTH*LDCI_PIC_HEIGHT) {
		subsample_en = 0;
	} else {
		if (frame_params->width < LDCI_PIC_WIDTH || frame_params->height < LDCI_PIC_HEIGHT) {
			ISP_ERR("width/height failed!(%d, %d)\n", frame_params->width, frame_params->height);
			return -1;
		}
		subsample_en = 1;
	}

	pthread_mutex_lock(&isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf_lock);
	if (!subsample_en) {
		memcpy(isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf, frame_params->buf, frame_params->size);
	} else {
		dst = isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf;
		for (i = 0; i < LDCI_PIC_HEIGHT; i++) {
			for (j = 0; j < LDCI_PIC_WIDTH; j++) {
				*dst = frame_params->buf[(i * frame_params->width * frame_params->height / LDCI_PIC_HEIGHT) + (j * frame_params->width / LDCI_PIC_WIDTH)];
				dst++;
			}
		}
	}
	pthread_mutex_unlock(&isp_ctx[dev_id].gtm_entity_ctx.gtm_param->ldci_external_buf_lock);
	return 0;
}

int isp_set_stitch_mode(int isp_id, enum stitch_mode_t stitch_mode)
{
#if (HW_ISP_DEVICE_NUM > 1)

	if (isp_id >= HW_ISP_DEVICE_NUM) {
		ISP_ERR("isp_id = %d is error!!\n", isp_id);
		return -1;
	}

	isp_ctx[isp_id].stitch_mode = stitch_mode;
	switch (stitch_mode) {
	case STITCH_2IN1_LINNER:
		if (isp_id % 2 == 0)
			isp_ctx[isp_id + 1].stitch_mode = stitch_mode;
		else
			isp_ctx[isp_id - 1].stitch_mode = stitch_mode;
		ISP_PRINT("STITCH_2IN1_LINNER\n");
		break;
	default:
		ISP_ERR("not recognized stitch mode\n");
		break;
	}
#else
	isp_ctx[dev_id].stitch_mode = STITCH_NONE;
	ISP_ERR("not support stitch mode!!\n");
	return -1;
#endif
	return 0;
}

int media_dev_init(void)
{
	/*must be called before all isp init*/
	//memset(&media_params, 0, sizeof(media_params));

	isp_version_info();

	return 0;
}

void media_dev_exit(void)
{
	/*must be called after all isp exit*/
#if (HW_ISP_DEVICE_NUM > 1)
	if ((media_params.isp_use_cnt[0] == 0) && (media_params.isp_use_cnt[1] == 0)) {
		if (media_params.mdev) {
			media_close(media_params.mdev);
			media_params.mdev = NULL;
		}
	}
#else
	if (media_params.isp_use_cnt[0] == 0) {
		if (media_params.mdev) {
			media_close(media_params.mdev);
			media_params.mdev = NULL;
		}
	}
#endif
}

int isp_ir_reset(int dev_id, int mode_flag)
{
	int ret = 0, rst_en = 0;
	struct isp_table_reg_map reg;

	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	if (mode_flag & 0x01) {
		ISP_WARN("ISP select wdr config fail\n");
	}

	if (mode_flag & 0x02) {
		if (mode_flag & 0x04) {
			if (isp_ctx[dev_id].isp_ir_flag != ISP_AIISP_MODE) //original flag
				rst_en = 1;
			isp_ctx[dev_id].isp_ir_flag = ISP_AIISP_MODE;
			ISP_PRINT("ISP select ai-isp config\n");
		} else {
			if (isp_ctx[dev_id].isp_ir_flag == ISP_AIISP_MODE) //original flag
				rst_en = 1;
			isp_ctx[dev_id].isp_ir_flag = ISP_IR_MODE;
			ISP_PRINT("ISP select ir config\n");
		}
	} else {
		if (isp_ctx[dev_id].isp_ir_flag == ISP_AIISP_MODE) //original flag
			rst_en = 1;
		isp_ctx[dev_id].isp_ir_flag = ISP_COLOR_MODE;
	}

	isp_params_parse(isp, &isp_ctx[dev_id].isp_ini_cfg, isp_ctx[dev_id].isp_ir_flag, media_params.isp_sync_mode);
	ret = isp_tuning_reset(isp, &isp_ctx[dev_id].isp_ini_cfg);
	if (ret) {
		ISP_ERR("error: unable to reset isp tuning\n");
	}

	if (rst_en) {
		reg.addr = isp_ctx[dev_id].load_reg_base;
		reg.size = ISP_LOAD_DRAM_SIZE;
		isp_set_load_reg(isp, &reg);
	}
	return ret;
}

int isp_reset(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_ctx[dev_id].isp_ir_flag = ISP_COLOR_MODE;
	isp_params_parse(isp, &isp_ctx[dev_id].isp_ini_cfg, isp_ctx[dev_id].isp_ir_flag, media_params.isp_sync_mode);
	ret = isp_tuning_reset(isp, &isp_ctx[dev_id].isp_ini_cfg);
	if (ret) {
		ISP_ERR("error: unable to reset isp tuning\n");
	}

	return ret;
}

int isp_init(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;
	struct isp_table_reg_map reg;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id]++ > 0)
		return 0;

	/*update media entity and links*/
	if (media_params.mdev) {
		media_close(media_params.mdev);
		media_params.mdev = NULL;
	}

	media_params.mdev = media_open(MEDIA_DEVICE, 0);
	if (media_params.mdev == NULL) {
		ISP_ERR("isp%d update media entity and links failed!\n", dev_id);
		return -1;
	}

	ret = isp_dev_open(&media_params, dev_id);
	if (ret < 0)
		return ret;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_dev_register(isp, &isp_dev_ops);
	isp_dev_banding_ctx(isp, &isp_ctx[dev_id]);

	isp_ctx[dev_id].isp_id = dev_id;

	isp_sensor_otp_init(isp);
	isp_config_sensor_info(isp);

	isp_ctx_save_init(&isp_ctx[dev_id]);
	isp_stat_save_init(&isp_ctx[dev_id]);
	isp_log_save_init(&isp_ctx[dev_id]);

	isp_ctx[dev_id].isp_ir_flag = ISP_COLOR_MODE; // default close ir config , colorful
	ret = isp_params_parse(isp, &isp_ctx[dev_id].isp_ini_cfg, isp_ctx[dev_id].isp_ir_flag, media_params.isp_sync_mode);
	if (ret < 0) {
		if (dev_id >= 1)
			isp_ctx[dev_id].isp_ini_cfg = isp_ctx[0].isp_ini_cfg;
	}

	FUNCTION_LOG;
	isp_ctx_algo_init(&isp_ctx[dev_id], &isp_ctx_ops);
	FUNCTION_LOG;

	tuning[dev_id] = isp_tuning_init(isp, &isp_ctx[dev_id].isp_ini_cfg);
	if (tuning[dev_id] == NULL) {
		ISP_ERR("error: unable to initialize isp tuning\n");
		return -1;
	}
	FUNCTION_LOG;

	if (isp_ctx[dev_id].isp_ini_cfg.isp_test_settings.af_en || isp_ctx[dev_id].isp_ini_cfg.isp_test_settings.isp_test_focus)
		isp_act_init_range(isp, isp_ctx[dev_id].isp_ini_cfg.isp_3a_settings.vcm_min_code, isp_ctx[dev_id].isp_ini_cfg.isp_3a_settings.vcm_max_code);

	events_arr[dev_id].isp_id = dev_id;
	events_init(&events_arr[dev_id]);
	events_star(&events_arr[dev_id]);
	FUNCTION_LOG;

	reg.addr = isp_ctx[dev_id].load_reg_base;
	reg.size = ISP_LOAD_DRAM_SIZE;
	isp_set_load_reg(isp, &reg);
	strcpy(isp_ctx[dev_id].debug_param_info.libs_version, REPO_COMMIT);

	ISP_DEV_LOG(ISP_LOG_ISP, "isp%d init end!!!\n", dev_id);

	return 0;
}

int isp_update(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ret = isp_tuning_update(isp);
	if (ret) {
		ISP_ERR("error: unable to update isp tuning\n");
	}
	return ret;
}

int isp_get_imageparams(int dev_id, isp_image_params_t *pParams)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	memcpy(pParams, &isp_gen->image_params, sizeof(isp_image_params_t));
	return 0;
}

int isp_stop(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] == 1)
		events_stop(&events_arr[dev_id]);

	return 0;
}

/* stop ldci to close LDCI_VIDEO_CHN(video4) to events_stop __isp_thread*/
int isp_stop_ldci(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (isp_ctx[dev_id].isp_ini_cfg.isp_test_settings.gtm_en && isp_ctx[dev_id].isp_ini_cfg.isp_tunning_settings.gtm_type == ISP_GTM_LDCI)
		isp_ctx[dev_id].isp_ini_cfg.isp_tunning_settings.gtm_type = 0;

	return 0;
}

int isp_exit(int dev_id)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if ((media_params.isp_use_cnt[dev_id] == 0) || (--media_params.isp_use_cnt[dev_id] > 0))
		return 0;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	/*wait to exit until the thread is finished*/
	pthread_join(media_params.isp_tid[dev_id], NULL);

	/*clear stitch info*/
	isp_ctx[dev_id].stitch_mode = STITCH_NONE;
	media_params.isp_sync_mode = 0;
	isp_ctx[dev_id].isp_stat_buf = NULL;

	/* Clear the AWB parameters to ensure that there is no residue in the next process */
	isp_ctx[dev_id].module_cfg.wb_gain_cfg.wb_gain.r_gain = 0;
	isp_ctx[dev_id].module_cfg.wb_gain_cfg.wb_gain.gr_gain = 0;
	isp_ctx[dev_id].module_cfg.wb_gain_cfg.wb_gain.gb_gain = 0;
	isp_ctx[dev_id].module_cfg.wb_gain_cfg.wb_gain.b_gain = 0;
	memset(&isp_ctx[dev_id].awb_entity_ctx.awb_result, 0, sizeof(awb_result_t));

	isp_log_save_exit(&isp_ctx[dev_id]);
	isp_stat_save_exit(&isp_ctx[dev_id]);
	isp_ctx_save_exit(&isp_ctx[dev_id]);
	isp_tuning_exit(isp);
	isp_ctx_algo_exit(&isp_ctx[dev_id]);
	isp_sensor_otp_exit(isp);
	isp_dev_close(&media_params, dev_id);
	media_dev_exit();
	ISP_DEV_LOG(ISP_LOG_ISP, "isp%d exit end!!!\n", dev_id);

	return 0;
}

int isp_run(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] > 1)
		return 0;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ISP_PRINT("create isp%d server thread!\n", dev_id);

	ret = pthread_create(&media_params.isp_tid[dev_id], NULL, __isp_thread, isp);
	if(ret != 0)
		ISP_ERR("%s: %s\n",__func__, strerror(ret));
	pthread_setname_np(media_params.isp_tid[dev_id], "isp_thread");

	return ret;
}

/*
 *if donot want to run isp_stop/isp_exit and isp_init.
 *run isp_events_stop to stop isp run, and then run isp_events_restar and isp_run to rerun isp.
 */
int isp_events_stop(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	events_stop(&events_arr[dev_id]);

	/*wait to exit until the thread is finished*/
	pthread_join(media_params.isp_tid[dev_id], NULL);

	return 0;
}

int isp_events_restar(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	events_init(&events_arr[dev_id]);
	events_star(&events_arr[dev_id]);

	isp_ctx[dev_id].af_frame_cnt  = 0;
	isp_ctx[dev_id].ae_frame_cnt  = 0;
	isp_ctx[dev_id].awb_frame_cnt = 0;
	isp_ctx[dev_id].gtm_frame_cnt = 0;

	isp_ctx[dev_id].md_frame_cnt  = 0;
	isp_ctx[dev_id].afs_frame_cnt  = 0;
	isp_ctx[dev_id].iso_frame_cnt = 0;
	isp_ctx[dev_id].rolloff_frame_cnt = 0;
	isp_ctx[dev_id].alg_frame_cnt = 0;

	return 0;
}

HW_S32 isp_pthread_join(int dev_id)
{
#if 0
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] == 1)
		pthread_join(media_params.isp_tid[dev_id], NULL);
#endif
	return 0;
}

HW_S32 isp_get_cfg(int dev_id, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_tuning_get_cfg(isp, group_id, cfg_ids, cfg_data);
}
HW_S32 isp_set_cfg(int dev_id, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_tuning_set_cfg(isp, group_id, cfg_ids, cfg_data);
}

HW_S32 isp_stats_req(int dev_id, struct isp_stats_context *stats_ctx)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *ctx = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL)
		return -1;

	return isp_ctx_stats_req(ctx, stats_ctx);
}

HW_S32 isp_set_saved_ctx(int dev_id)
{
	return isp_ctx_save_exit(&isp_ctx[dev_id]);
}

int isp_set_fps(int dev_id, int s_fps)
{
	struct hw_isp_device *isp = NULL;
	struct sensor_fps fps;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	fps.fps = s_fps;
	isp_sensor_set_fps(isp, &fps);
	if(s_fps > 1)
		isp_ctx_update_ae_tbl(&isp_ctx[dev_id], s_fps);

	return 0;
}

HW_S32 isp_get_sensor_info(int dev_id, struct sensor_config *cfg)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	memset(cfg, 0, sizeof(struct sensor_config));

	isp_sensor_get_configs(isp, cfg);

	return 0;
}

HW_S32 isp_tdm_map(int dev_id, struct isp_tdm_map_cfg *map)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_set_tmd_addr_map(isp, map);
}

HW_S32 isp_get_tdm_data(int dev_id, struct vin_isp_tdm_data *data)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_requset_tdm_data(isp, data);
}

void isp_register_tdmbuffer_done_callback(int dev_id, void *func)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return;
	}

	isp->isp_tdm_buffer_process = func;
}

int isp_return_tdmbuffer(int dev_id, struct vin_isp_tdm_event_status *status)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_dev_tdm_return_buffer(isp, status);
}

/*******************isp for video buffer*********************/
HW_S32 isp_get_lv(int dev_id)
{
	struct hw_isp_device *isp;
	struct isp_lib_context *ctx;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		//ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL) {
		//ISP_ERR("isp%d get isp ctx failed!\n", dev_id);
		return -1;
	}

	return ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_lv;
}
HW_S32 isp_get_encpp_cfg(int dev_id, HW_U32 ctrl_id, void *value)
{
#ifdef USE_ENCPP
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	switch(ctrl_id) {
		case ISP_CTRL_ENCPP_EN:
			*(HW_S32 *)value = isp_gen->isp_ini_cfg.isp_test_settings.encpp_en;
			break;
		case ISP_CTRL_ENCPP_STATIC_CFG:
			*(struct encpp_static_sharp_config *)value = isp_gen->encpp_static_sharp_cfg;
			break;
		case ISP_CTRL_ENCPP_DYNAMIC_CFG:
			*(struct encpp_dynamic_sharp_config *)value = isp_gen->iso_entity_ctx.iso_result.encpp_dynamic_sharp_cfg;
			break;
		case ISP_CTRL_ENCODER_3DNR_CFG:
			*(struct encoder_3dnr_config *)value = isp_gen->iso_entity_ctx.iso_result.encoder_3dnr_cfg;
			break;
		case ISP_CTRL_ENCODER_2DNR_CFG:
			*(struct encoder_2dnr_config *)value = isp_gen->iso_entity_ctx.iso_result.encoder_2dnr_cfg;
			break;
		default:
			ISP_ERR("Unknown ctrl.\n");
			break;
	}
	return 0;
#else
	ISP_ERR("isp%d don't support ctrl encpp param!\n", dev_id);
	return -1;
#endif
}

HW_S32 isp_get_attr_cfg(int dev_id, HW_U32 ctrl_id, void *value)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	switch(ctrl_id) {
		case ISP_CTRL_MODULE_EN:
			break;
		case ISP_CTRL_DIGITAL_GAIN:
			*(HW_S32 *)value = isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_digital_gain;
			break;
		case ISP_CTRL_PLTMWDR_STR:
			*(HW_S32 *)value = isp_gen->tune.pltmwdr_level;
			break;
		case ISP_CTRL_PLTM_HARDWARE_STR:
			*(HW_S32 *)value = isp_gen->pltm_entity_ctx.pltm_result.pltm_auto_stren;
			break;
		case ISP_CTRL_DN_STR:
			*(HW_S32 *)value = isp_gen->tune.denoise_level;
			break;
		case ISP_CTRL_3DN_STR:
			*(HW_S32 *)value = isp_gen->tune.tdf_level;
			break;
		case ISP_CTRL_HIGH_LIGHT:
			*(HW_S32 *)value = isp_gen->tune.highlight_level;
			break;
		case ISP_CTRL_BACK_LIGHT:
			*(HW_S32 *)value = isp_gen->tune.backlight_level;
			break;
		case ISP_CTRL_WB_MGAIN:
			*(struct isp_wb_gain *)value = isp_gen->awb_entity_ctx.awb_result.wb_gain_output;
			break;
		case ISP_CTRL_AGAIN_DGAIN:
			*(struct gain_cfg *)value = isp_gen->tune.gains;
			break;
		case ISP_CTRL_COLOR_EFFECT:
			*(HW_S32 *)value = isp_gen->tune.effect;
			break;
		case ISP_CTRL_AE_ROI:
			*(struct isp_h3a_coor_win *)value = isp_gen->ae_settings.ae_coor;
			break;
		case ISP_CTRL_AE_FACE_CFG:
			*(struct ae_face_cfg *)value = isp_gen->ae_settings.face_cfg;
			break;
		case ISP_CTRL_COLOR_TEMP:
			*(HW_S32 *)value = isp_gen->awb_entity_ctx.awb_result.color_temp_output;
			break;
		case ISP_CTRL_EV_IDX:
			*(HW_S32 *)value = isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_idx;
			break;
		case ISP_CTRL_ISO_LUM_IDX:
			*(HW_S32 *)value = isp_gen->iso_entity_ctx.iso_result.lum_idx;
			break;
		case ISP_CTRL_COLOR_SPACE:
			*(HW_S32 *)value = isp_gen->sensor_info.color_space;
			break;
		default:
			ISP_ERR("Unknown ctrl.\n");
			break;
	}
	return 0;
}

static void isp_set_attr_cfg_run(struct isp_lib_context *isp_gen, HW_U32 ctrl_id, void *value)
{
	switch(ctrl_id) {
		case ISP_CTRL_MODULE_EN:
			break;
		case ISP_CTRL_DIGITAL_GAIN:
			break;
		case ISP_CTRL_PLTMWDR_STR:
			isp_gen->tune.pltmwdr_level = *(HW_S32 *)value;
			break;
		case ISP_CTRL_DN_STR:
			isp_gen->tune.denoise_level = *(HW_S32 *)value;
			break;
		case ISP_CTRL_3DN_STR:
			isp_gen->tune.tdf_level = *(HW_S32 *)value;
			break;
		case ISP_CTRL_HIGH_LIGHT:
			isp_gen->tune.highlight_level = *(HW_S32 *)value;
			break;
		case ISP_CTRL_BACK_LIGHT:
			isp_gen->tune.backlight_level = *(HW_S32 *)value;
			break;
		case ISP_CTRL_WB_MGAIN:
			isp_gen->awb_settings.wb_gain_manual = *(struct isp_wb_gain *)value;
			break;
		case ISP_CTRL_AGAIN_DGAIN:
			if (memcmp(&isp_gen->tune.gains, value, sizeof(struct gain_cfg))) {
				isp_gen->tune.gains = *(struct gain_cfg *)value;
				isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
			}
			break;
		case ISP_CTRL_COLOR_EFFECT:
			if (isp_gen->tune.effect != *(HW_S32 *)value) {
				isp_gen->tune.effect = *(HW_S32 *)value;
				isp_gen->isp_3a_change_flags |= ISP_SET_EFFECT;
			}
			break;
		case ISP_CTRL_AE_ROI:
			isp_s_ae_roi(isp_gen, AE_METERING_MODE_SPOT, value);
			break;
		case ISP_CTRL_AE_FACE_CFG:
			isp_s_ae_face_roi(isp_gen, AE_METERING_MODE_FACE_SPOT, value);
			break;
		case ISP_CTRL_AF_METERING:
			isp_s_af_metering_mode(isp_gen, value);
			break;
		case ISP_CTRL_VENC2ISP_PARAM:
			isp_gen->VencVe2IspParam = *(struct enc_VencVe2IspParam *)value;
			break;
		case ISP_CTRL_NPU_NR_PARAM:
			isp_gen->npu_nr_cfg = *(struct npu_face_nr_config *)value;
			break;
		case ISP_CTRL_SET_AE_TARGER:
			isp_gen->isp_ini_cfg.isp_test_settings.ae_forced = ((struct target_val *)value)->ae_forced;
			isp_gen->isp_ini_cfg.isp_test_settings.lum_forced = ((struct target_val *)value)->lum_forced;
			isp_ctx_config_update(isp_gen);
			break;
		case ISP_CTRL_SET_AE_WEIGHT:
			memcpy(&isp_gen->isp_ini_cfg.isp_3a_settings.ae_win_weight[0], (HW_S32 *)value, 64 * sizeof(int));
			isp_ctx_config_update(isp_gen);
			break;
		default:
			ISP_ERR("Unknown ctrl.\n");
			break;
	}
}

HW_S32 isp_set_attr_cfg(int dev_id, HW_U32 ctrl_id, void *value)
{
	struct hw_isp_device *isp = NULL;
	int i, mode, count = (media_params.isp_sync_mode >> 16) & 0xff;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	if (count > 1) {
		mode = media_params.isp_sync_mode & 0xffff;
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (mode & (0x1 << i)) {
				if (media_params.isp_use_cnt[i] == 0) {
					ISP_ERR("isp%d is not used\n", i);
					return -1;
				}
				isp = media_params.isp_dev[i];
				if (!isp) {
					ISP_ERR("isp%d device is NULL!\n", i);
					return -1;
				}
				isp_gen = isp_dev_get_ctx(isp);
				isp_set_attr_cfg_run(isp_gen, ctrl_id, value);
			}
		}
	} else {
		isp_set_attr_cfg_run(isp_gen, ctrl_id, value);
	}
	return 0;
}

void* isp_get_ctx_addr(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM) {
		return NULL;
	}

	return (void*)&isp_ctx[dev_id];
}

HW_S32 isp_get_info_length(HW_S32* i3a_length, HW_S32* debug_length)
{
	HW_S32 data_len = 0;

	*i3a_length =
		sizeof(ae_result_t) + sizeof(ae_param_t)+ sizeof(struct isp_ae_stats_s) // ae info
		+ sizeof(awb_result_t)+ sizeof(awb_param_t)+ sizeof(struct isp_awb_stats_s) // awb info
		+ sizeof(af_result_t)+ sizeof(af_param_t)+ sizeof(struct isp_af_stats_s); // af info

	*debug_length =
		sizeof(iso_result_t)
		+sizeof(iso_param_t) // iso info
		+sizeof(struct isp_module_config)  // isp module info
		+sizeof(int) 						// otp enable flag
		+16*16*3*sizeof(unsigned short)	// msc tbl
		+4*2*sizeof(unsigned short);	// wb otp data

	data_len = *i3a_length + *debug_length;
		ISP_PRINT("i3a_length:%d, debug_length:%d.\n", *i3a_length, *debug_length);
		ISP_PRINT("af_result_t:%d, af_param_t:%d, isp_af_stats_s:%d.\n", (unsigned int)sizeof(ae_result_t), (unsigned int)sizeof(ae_param_t), (unsigned int)sizeof(struct isp_ae_stats_s));
		ISP_PRINT("af_result_t:%d, af_param_t:%d, isp_af_stats_s:%d.\n", (unsigned int)sizeof(awb_result_t), (unsigned int)sizeof(awb_param_t), (unsigned int)sizeof(struct isp_awb_stats_s));
		ISP_PRINT("af_result_t:%d, af_param_t:%d, isp_af_stats_s:%d.\n", (unsigned int)sizeof(af_result_t), (unsigned int)sizeof(af_param_t), (unsigned int)sizeof(struct isp_af_stats_s));
	return data_len;
}

HW_S32 isp_get_3a_parameters(int dev_id, void* params)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
	return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;
	if (isp_gen->ae_entity_ctx.ae_stats.ae_stats == NULL
			|| isp_gen->awb_entity_ctx.awb_stats.awb_stats == NULL
			|| isp_gen->af_entity_ctx.af_stats.af_stats == NULL) {
		ISP_ERR("isp:%d device have empty info!\n", dev_id);
		return -1;
	}
	void * ptr = params;
	int isp_3a_size = 0;
	// ae info
	memcpy(ptr, &(isp_gen->ae_entity_ctx.ae_result), sizeof(ae_result_t));
	ptr += sizeof(ae_result_t);

	memcpy(ptr, isp_gen->ae_entity_ctx.ae_param, sizeof(ae_param_t));
	ptr += sizeof(ae_param_t);

	memcpy(ptr, isp_gen->ae_entity_ctx.ae_stats.ae_stats, sizeof(struct isp_ae_stats_s));
	ptr += sizeof(struct isp_ae_stats_s);

	// awb info
	memcpy(ptr, &(isp_gen->awb_entity_ctx.awb_result), sizeof(awb_result_t));
	ptr += sizeof(awb_result_t);

	memcpy(ptr, isp_gen->awb_entity_ctx.awb_param, sizeof(awb_param_t));
	ptr += sizeof(awb_param_t);

	memcpy(ptr, isp_gen->awb_entity_ctx.awb_stats.awb_stats, sizeof(struct isp_awb_stats_s));
	ptr += sizeof(struct isp_awb_stats_s);

	// af info
	memcpy(ptr, &(isp_gen->af_entity_ctx.af_result), sizeof(af_result_t));
	ptr += sizeof(af_result_t);

	memcpy(ptr, isp_gen->af_entity_ctx.af_param, sizeof(af_param_t));
	ptr += sizeof(af_param_t);

	memcpy(ptr, isp_gen->af_entity_ctx.af_stats.af_stats, sizeof(struct isp_af_stats_s));
	ptr += sizeof(struct isp_af_stats_s);

	isp_3a_size = sizeof(ae_result_t) + sizeof(ae_param_t) +
		sizeof(struct isp_ae_stats_s) + sizeof(awb_result_t) +
		sizeof(awb_param_t) + sizeof(struct isp_awb_stats_s) +
		sizeof(af_result_t) + sizeof(af_param_t) +
		sizeof(struct isp_af_stats_s);

	ptr = NULL;
	return isp_3a_size;
}

HW_S32 isp_get_debug_msg(int dev_id, void* msg)
{
	struct hw_isp_device *isp = NULL;
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	void * ptr = msg;
	int isp_debug_msg_size = 0;
	memcpy(ptr, &(isp_gen->iso_entity_ctx.iso_result), sizeof(iso_result_t));
	ptr += sizeof(iso_result_t);

	memcpy(ptr, isp_gen->iso_entity_ctx.iso_param, sizeof(iso_param_t));
	ptr += sizeof(iso_param_t);

	memcpy(ptr, &isp_gen->module_cfg, sizeof(struct isp_module_config));
	ptr += sizeof(struct isp_module_config);

	memcpy(ptr, &isp_gen->sensor_otp.otp_enable, sizeof(unsigned char));
	ptr += sizeof(unsigned char);

	// shading msc rgb tbl 16x16x3
	if (isp_gen->sensor_otp.pmsc_table) {
		memcpy(ptr, isp_gen->sensor_otp.pmsc_table, 16*16*3*sizeof(unsigned short));
		ptr += 16*16*3*sizeof(unsigned short);
	}

	// wb otp & golden data
	if (isp_gen->sensor_otp.pwb_table) {
		memcpy(ptr, isp_gen->sensor_otp.pwb_table, 4*2*sizeof(unsigned short));
		ptr += 4*2*sizeof(unsigned short);
	}

	isp_debug_msg_size = sizeof(iso_result_t) +
		sizeof(iso_param_t) +
		sizeof(struct isp_module_config) +
		sizeof(unsigned char) +
		16*16*3*sizeof(unsigned short) +
		4*2*sizeof(unsigned short);

	ptr = NULL;
	return isp_debug_msg_size;
}

HW_S32 isp_set_ae_flicker_comp(int dev_id, HW_S16 enable)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if ((enable < 0) || (enable > 1)) {
		ISP_ERR("Invaild ae_flicker_comp option !\n");
		return -1;
	}

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_blowout_pre_en = enable;
	ISP_PRINT("Current flicker comp = %d\n", isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_blowout_attr);

	return 0;
}

HW_S32 isp_save_debug_info(int dev_id, int is_save_buf,
	const char *file_name, char *isp_info_param)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;
	FILE *file = NULL;
	char *buffer = NULL;
	int buffer_index = 0;
	int buffer_length = 0;
	int k = 0, m = 0;
	int r_cnt = 0;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	if (!isp_gen)
		return -1;

	if(file_name != NULL) {
		file = fopen(file_name, "wb+");
		if (!file)
			return -1;
	}

	buffer = calloc(90 * 1024,sizeof(char));
	if (!buffer) {
		fclose(file);
		return -1;
	}

	buffer_length = snprintf(buffer, 256, "struct isp_3a_parameters_t isp_3a_parameters = {\n");
	if(file_name != NULL) {
		fwrite(buffer, buffer_length, 1, file);
	}
	if(is_save_buf) {
		memcpy(isp_info_param,buffer,buffer_length);
		isp_info_param += buffer_length;
		r_cnt += buffer_length;
	}

	/* ae result */
	buffer_index = buffer_length = snprintf(buffer, 8192,
		"\t.ae_result = {\n"
		"\t\t.ae_status = %d,\n"
		"\t\t.sensor_set = {\n"
		"\t\t\t.ev_set = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_set_last = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_set_curr = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_idx_max = %d,\n"
		"\t\t\t.ev_idx_expect = %d,\n"
		"\t\t},\n",
		isp_gen->ae_entity_ctx.ae_result.ae_status,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set.ev_idx,

		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_last.ev_idx,

		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_idx,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_idx_max,
		isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_idx_expect
		);

	buffer_length = snprintf(&buffer[buffer_index], 4096,
		"\t\t.sensor_set_short = {\n"
		"\t\t\t.ev_set = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_set_last = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_set_curr = {\n"
		"\t\t\t\t.ev_exposure_time = %u,\n"
		"\t\t\t\t.ev_analog_gain = %u,\n"
		"\t\t\t\t.ev_digital_gain = %u,\n"
		"\t\t\t\t.ev_total_gain = %u,\n"
		"\t\t\t\t.ev_sensor_exp_line = %u,\n"
		"\t\t\t\t.ev_sensor_true_exp_line = %u,\n"
		"\t\t\t\t.ev_f_number = %u,\n"
		"\t\t\t\t.ev_fno2 = %u,\n"
		"\t\t\t\t.ev_av = %u,\n"
		"\t\t\t\t.ev_tv = %u,\n"
		"\t\t\t\t.ev_sv = %u,\n"
		"\t\t\t\t.ev_lv = %d,\n"
		"\t\t\t\t.ev = %u,\n"
		"\t\t\t\t.ev_idx = %d,\n"
		"\t\t\t},\n"
		"\t\t\t.ev_idx_max = %d,\n"
		"\t\t\t.ev_idx_expect = %d,\n"
		"\t\t},\n",
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set.ev_idx,

		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_last.ev_idx,

		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_exposure_time,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_analog_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_digital_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_total_gain,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_sensor_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_sensor_true_exp_line,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_f_number,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_fno2,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_av,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_tv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_sv,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_lv,
		(HW_U32)(isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev * 10000),
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_idx,

		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_idx_max,
		isp_gen->ae_entity_ctx.ae_result.sensor_set_short.ev_idx_expect
		);

	buffer_index += buffer_length;
	buffer_length = snprintf(&buffer[buffer_index], 4096,
		"\t\t.BrightPixellValue = %d,\n"
		"\t\t.DarkPixelValue = %d,\n"
		"\t\t.ae_gain = %u,\n"
		"\t\t.ae_target = %d,\n"
		"\t\t.ae_avg_lum = %d,\n"
		"\t\t.ae_weight_lum = %d,\n"
		"\t\t.ae_delta_exp_idx = %d,\n"
		"\t\t.ev_lv_adj = %d,\n"
		"\t\t.ae_flash_ev_cumul = %d,\n"
		"\t\t.ae_flash_ok = %u,\n"
		"\t\t.ae_flash_led = %u,\n"
		//"\t\t\t.sensor = %d,\n"
		"\t\t\t.isp_hardware = %d,\n"
		"\t\t\t.tmp = %d,\n"
		"\t\t\t.last = %d,\n"
		"\t\t},\n"
		"\t\t.ae_wdr_delay = %d,\n"
		"\t\t.wdr_hi_th = %d,\n"
		"\t\t.wdr_low_th = %d,\n"
		"\t\t.hist_low = %u,\n"
		"\t\t.hist_mid = %u,\n"
		"\t\t.hist_hi = %u,\n"
		"\t\t.backlight = %u,\n"
		"\t\t.gain_ratio = %f,\n"
		"\t},\n",
		isp_gen->ae_entity_ctx.ae_result.BrightPixellValue, isp_gen->ae_entity_ctx.ae_result.DarkPixelValue,
		isp_gen->ae_entity_ctx.ae_result.ae_gain, isp_gen->ae_entity_ctx.ae_result.ae_target,
		isp_gen->ae_entity_ctx.ae_result.ae_avg_lum, isp_gen->ae_entity_ctx.ae_result.ae_weight_lum,
		isp_gen->ae_entity_ctx.ae_result.ae_delta_exp_idx, isp_gen->ae_entity_ctx.ae_result.ev_lv_adj,
		isp_gen->ae_entity_ctx.ae_result.ae_flash_ev_cumul, isp_gen->ae_entity_ctx.ae_result.ae_flash_ok,
		isp_gen->ae_entity_ctx.ae_result.ae_flash_led, /* isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.sensor,*/
		isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.isp_hardware, isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.tmp,
		isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.last, isp_gen->ae_entity_ctx.ae_result.ae_wdr_delay,
		isp_gen->ae_entity_ctx.ae_result.wdr_hi_th, isp_gen->ae_entity_ctx.ae_result.wdr_low_th,
		isp_gen->ae_entity_ctx.ae_result.hist_low, isp_gen->ae_entity_ctx.ae_result.hist_mid,
		isp_gen->ae_entity_ctx.ae_result.hist_hi, isp_gen->ae_entity_ctx.ae_result.backlight,
		isp_gen->ae_entity_ctx.ae_result.gain_ratio);
	buffer_index += buffer_length;

	/* ae_param */
	/* isp_ae_stats */
	buffer_length = snprintf(&buffer[buffer_index], 256,
		"\t.isp_ae_stats = {\n"
		"\t\t.win_pix_n = %u,\n",
		isp_gen->ae_entity_ctx.ae_stats.ae_stats->win_pix_n);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.avg = {\n\t\t\t");
	buffer_index += buffer_length;
	int ae_avg_cnt = 0;
	for(k = 0; k < ISP_AE_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AE_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->avg[ae_avg_cnt]);
			ae_avg_cnt++;
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AE_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->avg[ae_avg_cnt]);
		ae_avg_cnt++;
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%u\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->avg[ae_avg_cnt]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.hist = {\n\t\t\t");
	buffer_index += buffer_length;
	int hist_cnt = 0;
	int hist1_cnt = 0;
	for(k = 0; k < 16 - 1; k++)
	{
		for(m = 0; m < 16;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist[hist_cnt]);
			hist_cnt++;
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < 16 - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist[hist_cnt]);
		hist_cnt++;
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%u\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist[hist_cnt]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.hist1 = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < 16 - 1; k++)
	{
		for(m = 0; m < 16;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist1[hist1_cnt]);
			hist1_cnt++;
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < 16 - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%u, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist1[hist1_cnt]);
		hist1_cnt++;
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%u\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->hist1[hist1_cnt]);
	buffer_index += buffer_length;
	/*buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.accum_r = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AE_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AE_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_r[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AE_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_r[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_r[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.accum_g = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AE_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AE_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_g[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AE_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_g[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_g[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.accum_b = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AE_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AE_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_b[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AE_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_b[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->ae_entity_ctx.ae_stats.ae_stats->accum_b[k][m]);
	buffer_index += buffer_length;*/
	buffer_length = snprintf(&buffer[buffer_index], 32,"\t},\n");/* end isp_ae_stats */
	buffer_index += buffer_length;

	/* awb_param */
	/* isp_awb_stats */
	buffer_length = snprintf(&buffer[buffer_index], 256, "\t.isp_awb_stats = {\n");
	buffer_index += buffer_length;

	/*buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_sum_r = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_r[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_r[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_r[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_sum_g = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_g[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_g[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_g[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_sum_b = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_b[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_b[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_b[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_sum_cnt = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_cnt[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_cnt[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_sum_cnt[k][m]);
	buffer_index += buffer_length;*/

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_avg_r = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_r[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_r[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_r[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_avg_g = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_g[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_g[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_g[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_avg_b = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_b[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_b[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_avg_b[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.avg = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->avg[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->avg[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->avg[k][m]);
	buffer_index += buffer_length;

	/*buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_deal_r = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_r[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_r[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_r[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_deal_g = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_g[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_g[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_g[k][m]);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 256, "\t\t.awb_deal_b = {\n\t\t\t");
	buffer_index += buffer_length;
	for(k = 0; k < ISP_AWB_ROW - 1; k++)
	{
		for(m = 0; m < ISP_AWB_COL;m++)
		{
			buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_b[k][m]);
			buffer_index += buffer_length;
		}
		buffer_length = snprintf(&buffer[buffer_index], 32, "\n\t\t\t");
		buffer_index += buffer_length;
	}
	for(m = 0; m < ISP_AWB_COL - 1;m++)
	{
		buffer_length = snprintf(&buffer[buffer_index], 64, "%d, ", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_b[k][m]);
		buffer_index += buffer_length;
	}
	buffer_length = snprintf(&buffer[buffer_index], 64, "%d\n\t\t},\n", isp_gen->awb_entity_ctx.awb_stats.awb_stats->awb_deal_b[k][m]);
	buffer_index += buffer_length;*/

	buffer_length = snprintf(&buffer[buffer_index], 32,"\t},\n");/* end isp_awb_stats */
	buffer_index += buffer_length;

	/* af_result */
	buffer_length = snprintf(&buffer[buffer_index], 256,
		"\t.af_result = {\n"
		"\t\t.af_status_output = %d,\n"
		"\t\t.last_code_output = %u,\n"
		"\t\t.real_code_output = %u,\n"
		"\t\t.std_code_output = %u,\n"
		"\t\t.af_sap_lim_output = %u,\n"
		"\t\t.af_sharp_output = %u,\n"
		"\t},\n",
		isp_gen->af_entity_ctx.af_result.af_status_output, isp_gen->af_entity_ctx.af_result.last_code_output,
		isp_gen->af_entity_ctx.af_result.real_code_output, isp_gen->af_entity_ctx.af_result.std_code_output,
		isp_gen->af_entity_ctx.af_result.af_sap_lim_output, isp_gen->af_entity_ctx.af_result.af_sharp_output);
	buffer_index += buffer_length;

	buffer_length = snprintf(&buffer[buffer_index], 32,"};\n");
	buffer_index += buffer_length;

	/* af_param */
	/* isp_af_stats */
	buffer_length = snprintf(&buffer[buffer_index], AF_PRINT_LENGTH + 4,
		"%s\n",
		isp_gen->af_entity_ctx.af_result.af_printf_out);
	buffer_index += buffer_length;

	if(file_name != NULL) {
		fwrite(buffer, buffer_index, 1, file);
	}
	if(is_save_buf) {
		memcpy(isp_info_param,buffer,buffer_index);
		r_cnt += buffer_index;
	}

	if(file_name != NULL) {
		fclose(file);
	}
	free(buffer);

	return r_cnt;
}

int isp_print_out_af(int dev_id)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	if (!isp_gen)
		return -1;

	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_PRINT_INFO);

	return 0;
}

HW_S32 isp_get_flash_delay_frame(int dev_id, int* flash_delay_frame)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	if (!isp_gen)
		return -1;

	ret = isp_gen->isp_ini_cfg.isp_tunning_settings.flash_delay_frame;
	*flash_delay_frame = ret;
	return ret;
}

HW_S32 isp_get_iso_value(int dev_id)
{
	HW_S32 iso_value = 0;
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;
	HW_S32 total_gain = 0;
	struct ev_setting *ev_set_curr = NULL;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	if (!isp_gen)
		return -1;

	ev_set_curr = &(isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr);
	if (!ev_set_curr) {
		ISP_ERR("isp%d get ev set error!\n", dev_id);
		return -1;
	}

	total_gain = (ev_set_curr->ev_analog_gain * ev_set_curr->ev_digital_gain) >> 8;
	iso_value = ((total_gain * 100) / 1024 + 25) / 50 * 50;

	return iso_value;
}

int stein(int x, int y)
{
	if (x < y)
	{
		int tmp = x;
		x = y;
		y = tmp;
	}
	if ( x%y == 0)
	{
		return y;
	}
	if (x % 2 == 0 && y % 2 == 0)
	{
		return 2*stein(x >> 1, y >> 1);
	}
	else if (x%2 == 0 && y%2 != 0)
	{
		return stein(x >> 1, y);
	}
	else if (x % 2 != 0 && y % 2 == 0)
	{
		return stein(x, y >> 1);
	}
	else if (x % 2 != 0 && y % 2 != 0)
	{
		return stein(x, (x - y) >> 1);
	}
	return 1;
}

HW_S32 isp_get_exp_time(int dev_id, unsigned int* num, unsigned int* den)
{
	HW_S32 exp_time_value = 0;
	HW_S32 greatest_com_divisor = 0;
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen = NULL;
	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_gen = isp_dev_get_ctx(isp);
	if (!isp_gen)
		return -1;
	exp_time_value =
		((isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_exposure_time / 1000 + 1) / 10) * 10;
	if(exp_time_value <= 0) {
		exp_time_value = 1;
	}
	greatest_com_divisor = stein(1000,exp_time_value);
	*num = exp_time_value / greatest_com_divisor;
	*den = 1000 / greatest_com_divisor;
	return 0;
}

int isp_calc_wb_gain(HW_U32 r_mean, HW_U32 g_mean, HW_U32 b_mean,
						HW_U32 *r_gain, HW_U32 *b_gain, HW_U32 precision)
{
	if ((!r_gain) || (!b_gain)) {
		ISP_ERR("Invaild r_gain b_gain\n");
		return -1;
	}

	*r_gain = (g_mean * precision) / clamp(r_mean, 1, 255);
	*b_gain = (g_mean * precision) / clamp(b_mean, 1, 255);

	return 0;
}

int isGrayBlock(HW_U32 r_mean, HW_U32 g_mean, HW_U32 b_mean,
			HW_U32 rg_min, HW_U32 rg_max, HW_U32 bg_min, HW_U32 bg_max, HW_U32 precision)
{
	HW_U32 rg;
	HW_U32 bg;

	if ((g_mean < 55) || (g_mean >= 200)) {
		return 0;
	}

	isp_calc_wb_gain(r_mean, g_mean, b_mean, &rg, &bg, precision);
	if ((IS_BETWEEN(rg, rg_min, rg_max)) && IS_BETWEEN(bg, bg_min, bg_max)) {
		return 1;
	} else {
		return 0;
	}
}

HW_S32 isp_malloc_awb_fe_stats(struct isp_lib_context *ctx)
{
	if (!ctx) {
		ISP_ERR("ctx is NULL, it will not to set ae comp!!!\n");
		return -1;
	}

	ctx->switch_info.sensorA_awb_stats = malloc(sizeof(struct isp_awb_stats_s));
	ctx->switch_info.sensorB_awb_stats = malloc(sizeof(struct isp_awb_stats_s));
	if (!ctx->switch_info.sensorA_awb_stats || !ctx->switch_info.sensorB_awb_stats) {
		ISP_ERR("sensorA_awb_stats or sensorB_awb_stats is NULL!!!\n");
		return -1;
	}
	memset(ctx->switch_info.sensorA_awb_stats, 0, sizeof(struct isp_awb_stats_s));
	memset(ctx->switch_info.sensorB_awb_stats, 0, sizeof(struct isp_awb_stats_s));

	return 0;
}

HW_S32 isp_free_awb_fe_stats(struct isp_lib_context *ctx)
{
	if (!ctx) {
		ISP_ERR("ctx is NULL, it will not to set ae comp!!!\n");
		return -1;
	}

	if (!ctx->switch_info.sensorA_awb_stats && !ctx->switch_info.sensorB_awb_stats) {
		ISP_ERR("sensorA_awb_stats or sensorB_awb_stats is NULL!!!\n");
		return -1;
	}

	free(ctx->switch_info.sensorA_awb_stats);
	free(ctx->switch_info.sensorB_awb_stats);
	if (!ctx->switch_info.sensorA_awb_stats || !ctx->switch_info.sensorB_awb_stats) {
		ISP_ERR("sensorA_awb_stats or sensorB_awb_stats free failed!!!\n");
		return -1;
	}

	return 0;
}

//#define PRINT_STATS
HW_S32 isp_get_awb_fe_stats_avg(int dev_id, struct isp_awb_stats_s *awb_stats)
{
	struct hw_isp_device *isp = NULL;
	HW_U32 rgain = 256, bgain = 256;
	HW_U16 row, col;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL) {
		ISP_ERR("isp_gen is NULL!!!\n");
		return -1;
	}

	if (!awb_stats) {
		ISP_ERR("awb_stats is NULL!\n");
	}

	rgain = div_round(isp_gen->stats_ctx.wb_gain_saved.r_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);
	bgain = div_round(isp_gen->stats_ctx.wb_gain_saved.b_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);
#ifdef PRINT_STATS
	printf("---------------------------- R ----------------------------\n");
#endif
	for (row = 0;row < ISP_AWB_ROW; row++) {
		for (col = 0; col < ISP_AWB_COL; col++) {
			awb_stats->awb_avg_r[row][col] = isp_gen->stats_ctx.stats.awb_stats.awb_avg_r[row][col] * rgain >> 8;
			awb_stats->awb_avg_g[row][col] = isp_gen->stats_ctx.stats.awb_stats.awb_avg_g[row][col];
			awb_stats->awb_avg_b[row][col] = isp_gen->stats_ctx.stats.awb_stats.awb_avg_b[row][col] * bgain >> 8;
#ifdef PRINT_STATS
			printf("%3d ", awb_stats->awb_avg_r[row][col]);
#endif
		}
#ifdef PRINT_STATS
		printf("\n");
#endif
	}

#ifdef PRINT_STATS
	printf("---------------------------- G ----------------------------\n");
	for (row = 0;row < ISP_AWB_ROW; row++) {
		for (col = 0; col < ISP_AWB_COL; col++) {
			printf("%3d ", awb_stats->awb_avg_g[row][col]);
		}
		printf("\n");
	}

	printf("---------------------------- b ----------------------------\n");
	for (row = 0;row < ISP_AWB_ROW; row++) {
		for (col = 0; col < ISP_AWB_COL; col++) {
			printf("%3d ", awb_stats->awb_avg_b[row][col]);
		}
		printf("\n");
	}
	printf("----------------------------------------------------------\n");
#endif

	return 0;
}

HW_S32 isp_sensor_mipi_switch_get_awb_comp(struct isp_lib_context *isp_gen, HW_U32 *r_offset_avg, HW_U32 *b_offset_avg, HW_U32 precision)
{
	struct isp_awb_stats_s *a_awb_stats, *b_awb_stats;
	HW_U32 r_offset_tmp = 0, b_offset_tmp = 0;
	HW_U32 r_offset_sum = 0, b_offset_sum = 0;
	HW_U32 rgain = 256, bgain = 256;
	HW_U16 row, col;

	if (!r_offset_avg || !b_offset_avg) {
		ISP_ERR("r_offset_avg or  b_offset_avg is NULL!!!\n");
		return -1;
	}

	if (!isp_gen) {
		ISP_ERR("isp_gen is NULL!!!\n");
		return -1;
	}

	if (!isp_gen->switch_info.sensorA_awb_stats || !isp_gen->switch_info.sensorB_awb_stats) {
		ISP_ERR("awb_stats is NULL!!!\n");
		return -1;
	}

	a_awb_stats = isp_gen->switch_info.sensorA_awb_stats;
	b_awb_stats = isp_gen->switch_info.sensorB_awb_stats;
	rgain = div_round(isp_gen->stats_ctx.wb_gain_saved.r_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);
	bgain = div_round(isp_gen->stats_ctx.wb_gain_saved.b_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);
	isp_gen->switch_info.vaild_block_sum = 0;
	for (row = 0;row < ISP_AWB_ROW; row++) {
		for (col = 0; col < ISP_AWB_COL; col++) {
			/* block should be grey and no most different with last info */
			if (isGrayBlock(a_awb_stats->awb_avg_r[row][col], a_awb_stats->awb_avg_g[row][col], a_awb_stats->awb_avg_b[row][col],
					MIPI_SWITCH_AWB_RG_MIN, MIPI_SWITCH_AWB_RG_MAX, MIPI_SWITCH_AWB_BG_MIN, MIPI_SWITCH_AWB_BG_MAX, precision) &&
				isGrayBlock(b_awb_stats->awb_avg_r[row][col], b_awb_stats->awb_avg_g[row][col], b_awb_stats->awb_avg_b[row][col],
					MIPI_SWITCH_AWB_RG_MIN, MIPI_SWITCH_AWB_RG_MAX, MIPI_SWITCH_AWB_BG_MIN, MIPI_SWITCH_AWB_BG_MAX, precision) &&
				(abs(a_awb_stats->awb_avg_g[row][col] - b_awb_stats->awb_avg_g[row][col]) < MIPI_SWITCH_AWB_STATS_DIFF)) {
				/* get stats before awb */
				a_awb_stats->awb_avg_r[row][col] = div_round(a_awb_stats->awb_avg_r[row][col] << 8, rgain);
				a_awb_stats->awb_avg_b[row][col] = div_round(a_awb_stats->awb_avg_b[row][col] << 8, bgain);
				b_awb_stats->awb_avg_r[row][col] = div_round(b_awb_stats->awb_avg_r[row][col] << 8, rgain);
				b_awb_stats->awb_avg_b[row][col] = div_round(b_awb_stats->awb_avg_b[row][col] << 8, bgain);
				/* calc awb offset  */
				r_offset_tmp = (a_awb_stats->awb_avg_r[row][col] * b_awb_stats->awb_avg_g[row][col] * precision) / (a_awb_stats->awb_avg_g[row][col] * b_awb_stats->awb_avg_r[row][col]);
				b_offset_tmp = (a_awb_stats->awb_avg_b[row][col] * b_awb_stats->awb_avg_g[row][col] * precision) / (a_awb_stats->awb_avg_g[row][col] * b_awb_stats->awb_avg_b[row][col]);
				r_offset_sum += r_offset_tmp;
				b_offset_sum += b_offset_tmp;
				isp_gen->switch_info.vaild_block_sum++;
			}
		}
	}
	if (isp_gen->switch_info.vaild_block_sum < MIPI_SWITCH_AWB_VAILD_BLOCK_THRD) {
		ISP_WARN("vaild_block_sum = %d, it will not update awb offset\n", isp_gen->switch_info.vaild_block_sum);
		return -1;
	} else {
		*r_offset_avg = r_offset_sum / isp_gen->switch_info.vaild_block_sum;
		*b_offset_avg = b_offset_sum / isp_gen->switch_info.vaild_block_sum;
		ISP_PRINT("vaild_block_sum = %d, r_offset_sum = %d, b_offset_sum = %d, r_offset_avg = %d, b_bgain = %d\n",
			isp_gen->switch_info.vaild_block_sum, r_offset_sum, b_offset_sum, *r_offset_avg, *b_offset_avg);
	}

	return 0;
}

HW_S32 isp_sensor_mipi_switch_parser_ae_comp(struct sensor_mipi_switch_entity *sensor_mipi_switch_info, int precision, int default_comp)
{
	if (!sensor_mipi_switch_info) {
		ISP_ERR("sensor_mipi_switch_info is NULL!!!\n");
		return -1;
	}

	if (sensor_mipi_switch_info->comp_ratio <= 0) {
		sensor_mipi_switch_info->comp_ratio = precision;
		sensor_mipi_switch_info->exp_comp = default_comp;
		sensor_mipi_switch_info->gain_comp = default_comp;
		ISP_WARN("It will use default: comp_ratio = %d, exp_comp = %d, gain_comp = %d\n",
			sensor_mipi_switch_info->comp_ratio, sensor_mipi_switch_info->exp_comp, sensor_mipi_switch_info->gain_comp);
		return -1;
	}

	return 0;
}

HW_S32 isp_sensor_mipi_switch_set_ae_comp(struct isp_lib_context *ctx, struct sensor_mipi_switch_entity *sensor_mipi_switch_info)
{
	HW_U32 ev_sensor_exp_line;
	HW_U32 ev_analog_gain;
	HW_U32 comp_ratio;

	if (!ctx) {
		ISP_ERR("ctx is NULL, it will not to set ae comp!!!\n");
		return -1;
	}

	if (!sensor_mipi_switch_info) {
		ISP_ERR("sensor_mipi_switch_info is NULL!!!\n");
		return -1;
	}

	if (sensor_mipi_switch_info->exp_comp <= 0 || sensor_mipi_switch_info->gain_comp <= 0 || sensor_mipi_switch_info->comp_ratio <= 0) {
		ISP_ERR("Invaild exp_comp = %d, gain_comp = %d, comp_ratio = %d\n",
			sensor_mipi_switch_info->exp_comp, sensor_mipi_switch_info->gain_comp, sensor_mipi_switch_info->comp_ratio);
		sensor_mipi_switch_info->exp_comp = MIPI_SWITCH_DEFAULT_PRECISION;
		sensor_mipi_switch_info->gain_comp = MIPI_SWITCH_DEFAULT_PRECISION;
		sensor_mipi_switch_info->comp_ratio = MIPI_SWITCH_DEFAULT_PRECISION;
	}

	comp_ratio = sensor_mipi_switch_info->comp_ratio;
	ev_sensor_exp_line = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sensor_exp_line;
	ev_analog_gain = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_analog_gain;
	ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sensor_exp_line = ev_sensor_exp_line * sensor_mipi_switch_info->exp_comp / comp_ratio;
	ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_analog_gain = ev_analog_gain * sensor_mipi_switch_info->gain_comp / comp_ratio;

	return 0;
}

HW_S32 isp_sensor_mipi_switch_set_awb_comp(struct isp_lib_context *ctx, HW_U32 rgain_comp, HW_U32 bgain_comp, HW_U32 precision, HW_U8 ctrl_type)
{
	HW_S32 ret;

	if (ctx == NULL) {
		ISP_ERR("ctx is NULL!!!\n");
		return -1;
	}

	if (precision <= 0) {
		ISP_ERR("precision is %d!!!\n", precision);
		return -1;
	}

	if (ctrl_type == MIPI_SWITCH_AWB_COMP_RESET) {
		if (IS_BETWEEN(rgain_comp, MIPI_SWITCH_AWB_OFFSET_MIN, MIPI_SWITCH_AWB_OFFSET_MAX) &&
			IS_BETWEEN(bgain_comp, MIPI_SWITCH_AWB_OFFSET_MIN, MIPI_SWITCH_AWB_OFFSET_MAX)) {
			ISP_PRINT("########### reset_awb_offset ###########\n");
			ISP_PRINT("[orign] result: rgain = %d, bgain = %d, rfavor = %d, bfavor = %d, rgain_comp_last = %d, bgain_comp_last = %d\n",
			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain, ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain,
			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor, ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor,
			rgain_comp, bgain_comp);

			/* reset comp */
			if (ctx->switch_info.vaild_block_sum >= MIPI_SWITCH_AWB_VAILD_BLOCK_THRD) {
				ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain *
					MIPI_SWITCH_DEFAULT_PRECISION / ctx->switch_info.rgain_comp;
				ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain *
					MIPI_SWITCH_DEFAULT_PRECISION / ctx->switch_info.bgain_comp;
				ISP_PRINT("[RESET_CURR] result: rgain = %d, bgain = %d, rfavor = %d, bfavor = %d, rgain_comp_last = %d, bgain_comp_last = %d\n",
				ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain, ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain,
				ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor, ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor,
				ctx->switch_info.rgain_comp, ctx->switch_info.bgain_comp);
			}
			/* reset last_comp */
			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain *
				MIPI_SWITCH_DEFAULT_PRECISION / rgain_comp;
			ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain *
				MIPI_SWITCH_DEFAULT_PRECISION / bgain_comp;
//			/* reset awb favor */
//			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor = ctx->switch_info.orign_rgain_favor;
//			ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor = ctx->switch_info.orign_bgain_favor;
			ISP_PRINT("[RESET_LAST] result: rgain = %d, bgain = %d, rfavor = %d, bfavor = %d\n",
			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain, ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain,
			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor, ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor);
		} else {
			       ISP_WARN("rgain_comp = %d, bgain_comp = %d, it will not reset awb offset!\n", rgain_comp, bgain_comp);
		}
		return 0;
	}

	if (ctrl_type == MIPI_SWITCH_AWB_COMP_UPDATE) {
		if (IS_BETWEEN(rgain_comp, MIPI_SWITCH_AWB_OFFSET_MIN, MIPI_SWITCH_AWB_OFFSET_MAX) &&
			IS_BETWEEN(bgain_comp, MIPI_SWITCH_AWB_OFFSET_MIN, MIPI_SWITCH_AWB_OFFSET_MAX)) {
			ISP_PRINT("########### update_awb_offset ###########\n");
			ISP_PRINT("[orign] result: rgain = %d, bgain = %d, rfavor = %d, bfavor = %d, target_rgain_comp = %d, target_bgain_comp = %d\n",
			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain, ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain,
			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor, ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor, rgain_comp, bgain_comp);

			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain * rgain_comp / precision;
			ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain * bgain_comp / precision;
			/* update favor */
//			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor = ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor * rgain_comp / precision;
//			ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor = ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor * bgain_comp / precision;

			ISP_PRINT("[update] result: rgain = %d, bgain = %d, rfavor = %d, bfavor = %d\n",
			ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain, ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain,
			ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor, ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor);
		} else {
			ISP_WARN("rgain_comp = %d, bgain_comp = %d, it will not set awb offset!\n", rgain_comp, bgain_comp);
		}
	}

	return 0;
}

HW_S32 isp_sensor_mipi_switch_comp_process(int dev_id, struct isp_lib_context *ctx, struct sensor_mipi_switch_entity *sensor_mipi_switch_info)
{
	struct hw_isp_device *isp = NULL;
	struct isp_h3a_coor_win awb_roi_attr;
	static HW_S32 tdnf_level_save[HW_ISP_DEVICE_NUM] = {0};
	int ret = 0;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (!ctx) {
		ISP_ERR("ctx is NULL, it will not to do isp_sensor_mipi_switch_comp_process\n");
		return -1;
	}

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	if (!ctx->switch_info.mipi_switch_video) {
		ISP_ERR("switch_video is NULL!\n");
		return -1;
	}

	if (!sensor_mipi_switch_info) {
		ISP_ERR("sensor_mipi_switch_info is NULL!!!\n");
		return -1;
	}

	if (sensor_mipi_switch_info->mipi_switch_status == SWITCH_B) {
		if (ctx->switch_info.frame_cnt == MIPI_SWITCH_FRAME_CROP_WIN) {
			/* save origin favor */
			ctx->switch_info.orign_rgain_favor = ctx->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor;
			ctx->switch_info.orign_bgain_favor = ctx->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor;
			/* modify awb win for get A's stats */
			awb_roi_attr.x1 = -304;
			awb_roi_attr.y1 = -304;
			awb_roi_attr.x2 = 400;
			awb_roi_attr.y2 = 400;
			__isp_set_awb_win(&awb_roi_attr, &ctx->stat.pic_size, ISP_WIN_MODE_MATRIX, WIN_FULL, &ctx->module_cfg.awb_cfg.awb_reg_win);
			tdnf_level_save[ctx->isp_id] = ctx->tune.tdf_level;
			ctx->tune.tdf_level = 0;
			ISP_PRINT("ISP%d tdf_level = %d, tdnf_level_save = %d\n", ctx->isp_id, ctx->tune.tdf_level, tdnf_level_save[ctx->isp_id]);
			ISP_PRINT("zoom_dis_d3d...\n");
		}

		if (ctx->switch_info.frame_cnt == MIPI_SWITCH_FRAME_DO_SWITCH) {
			/* get A stats */
			isp_malloc_awb_fe_stats(ctx);
			isp_get_awb_fe_stats_avg(dev_id, ctx->switch_info.sensorA_awb_stats);
			/* reset awb win for switch B */
			awb_roi_attr.x1 = H3A_PIC_OFFSET;
			awb_roi_attr.y1 = H3A_PIC_OFFSET;
			awb_roi_attr.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
			awb_roi_attr.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
			__isp_set_awb_win(&awb_roi_attr, &ctx->stat.pic_size, ISP_WIN_MODE_MATRIX, WIN_FULL, &ctx->module_cfg.awb_cfg.awb_reg_win);
			/* set ae comp */
			/* update AE comp offset */
			isp_sensor_mipi_switch_parser_ae_comp(sensor_mipi_switch_info, MIPI_SWITCH_DEFAULT_PRECISION, (MIPI_SWITCH_DEFAULT_PRECISION + MIPI_SWITCH_AE_COMP_OFFSET));
			isp_sensor_mipi_switch_set_ae_comp(ctx, sensor_mipi_switch_info);
			ISP_PRINT("do_drop_frame_first\n");
			video_set_tdm_drop_frame_num(ctx->switch_info.mipi_switch_video, ctx->switch_info.mipi_switch_info.drop_frame_num);
			/* do switch */
			isp_sensor_mipi_switch(isp, sensor_mipi_switch_info);
			/* update last_comp */
			isp_sensor_mipi_switch_set_awb_comp(ctx, ctx->switch_info.rgain_comp_last, ctx->switch_info.bgain_comp_last,
					MIPI_SWITCH_DEFAULT_PRECISION, MIPI_SWITCH_AWB_COMP_UPDATE);
		}

		if (ctx->switch_info.frame_cnt == MIPI_SWITCH_FRAME_SET_OFFSET) {
			 ctx->tune.tdf_level = tdnf_level_save[ctx->isp_id];
			ISP_PRINT("ISP%d tdf_level = %d\n", ctx->isp_id, ctx->tune.tdf_level);
			ISP_PRINT("zoom_en_d3d...\n");
			/* get B stats */
			isp_get_awb_fe_stats_avg(dev_id, ctx->switch_info.sensorB_awb_stats);
			/* calc awb offset */
			ret = isp_sensor_mipi_switch_get_awb_comp(ctx, &ctx->switch_info.rgain_comp, &ctx->switch_info.bgain_comp, MIPI_SWITCH_DEFAULT_PRECISION);
			if (ret < 0) {
				ISP_WARN("It will not to update awb offset!!!\n");
			} else {
				/* check AWB comp and set it */
				isp_sensor_mipi_switch_set_awb_comp(ctx, ctx->switch_info.rgain_comp, ctx->switch_info.bgain_comp,
					MIPI_SWITCH_DEFAULT_PRECISION, MIPI_SWITCH_AWB_COMP_UPDATE);
			}
			isp_free_awb_fe_stats(ctx);
			ctx->awb_settings.white_balance_lock = false;
			ctx->switch_info.mipi_switch_enable = 0;
		}
	} else if (sensor_mipi_switch_info->mipi_switch_status == SWITCH_A) {
		ctx->awb_settings.white_balance_lock = true;
		isp_sensor_mipi_switch_parser_ae_comp(sensor_mipi_switch_info, MIPI_SWITCH_DEFAULT_PRECISION, (MIPI_SWITCH_DEFAULT_PRECISION - MIPI_SWITCH_AE_COMP_OFFSET));
		isp_sensor_mipi_switch_set_ae_comp(ctx, sensor_mipi_switch_info);
		isp_sensor_mipi_switch_set_awb_comp(ctx, ctx->switch_info.rgain_comp_last, ctx->switch_info.bgain_comp_last, MIPI_SWITCH_DEFAULT_PRECISION, MIPI_SWITCH_AWB_COMP_RESET);
		/* update last_comp */
		ctx->switch_info.rgain_comp_last = ctx->switch_info.rgain_comp;
		ctx->switch_info.bgain_comp_last = ctx->switch_info.bgain_comp;
		video_set_tdm_drop_frame_num(ctx->switch_info.mipi_switch_video, ctx->switch_info.mipi_switch_info.drop_frame_num);
		isp_sensor_mipi_switch(isp, sensor_mipi_switch_info);
		ctx->switch_info.mipi_switch_enable = 0;
	}

	return 0;
}

HW_S32 isp_sensor_mipi_switch_comp(int dev_id, struct isp_video_device *video, struct sensor_mipi_switch_entity *sensor_mipi_switch_info)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *ctx;
	const int precision = 10000;
	int ret = 0;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL) {
		ISP_ERR("isp%d get isp ctx failed!\n", dev_id);
		return -1;
	}

	if (!video) {
		ISP_ERR("video is NULL!!!\n");
		return -1;
	}

	if (!sensor_mipi_switch_info) {
		ISP_ERR("sensor_mipi_switch_info is NULL!!!\n");
		return -1;
	}

	ctx->switch_info.mipi_switch_info.switch_ctrl = sensor_mipi_switch_info->switch_ctrl;
	ctx->switch_info.mipi_switch_info.mipi_switch_status = sensor_mipi_switch_info->mipi_switch_status;
	ctx->switch_info.mipi_switch_info.comp_ratio = sensor_mipi_switch_info->comp_ratio;
	ctx->switch_info.mipi_switch_info.exp_comp = sensor_mipi_switch_info->exp_comp;
	ctx->switch_info.mipi_switch_info.gain_comp = sensor_mipi_switch_info->gain_comp;
	ctx->switch_info.mipi_switch_info.drop_frame_num = sensor_mipi_switch_info->drop_frame_num;
	if (!ctx->isp_ir_flag) {	// color mode will set awb + ae offset
		ctx->switch_info.mipi_switch_video = video;
		ctx->switch_info.frame_cnt = 0;
		ctx->switch_info.mipi_switch_enable = 1;
	} else {			// ir mode only set ae offset
		if (ctx->switch_info.mipi_switch_info.mipi_switch_status == SWITCH_B) {
			isp_sensor_mipi_switch_parser_ae_comp(sensor_mipi_switch_info, MIPI_SWITCH_DEFAULT_PRECISION, (MIPI_SWITCH_DEFAULT_PRECISION + MIPI_SWITCH_AE_COMP_OFFSET));
			isp_sensor_mipi_switch_set_ae_comp(ctx, sensor_mipi_switch_info);
		} else if (ctx->switch_info.mipi_switch_info.mipi_switch_status == SWITCH_A) {
			isp_sensor_mipi_switch_parser_ae_comp(sensor_mipi_switch_info, MIPI_SWITCH_DEFAULT_PRECISION, (MIPI_SWITCH_DEFAULT_PRECISION - MIPI_SWITCH_AE_COMP_OFFSET));
			isp_sensor_mipi_switch_set_ae_comp(ctx, sensor_mipi_switch_info);
		}
		ret = isp_sensor_mipi_switch(isp, &ctx->switch_info.mipi_switch_info);
	}

	return ret;
}

HW_S32 isp_sensor_mipi_switch_ctrl(int dev_id, struct isp_video_device *switch_video, struct sensor_mipi_switch_entity *sensor_mipi_switch_info)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *ctx;
	unsigned int isp_3a_lock;
	int ret = 0;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (!switch_video) {
		ISP_ERR("switch_video is NULL!!!\n");
		return -1;
	}

	if (!sensor_mipi_switch_info) {
		ISP_ERR("sensor_mipi_switch_info is NULL!!!\n");
		return -1;
	}

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL) {
		ISP_ERR("isp%d get isp ctx failed!\n", dev_id);
		return -1;
	}

	if (sensor_mipi_switch_info->switch_ctrl == SET_SWITCH) {
		ctx->switch_info.mipi_switch_info.time_stamp = 0;
		isp_sensor_mipi_switch_comp(dev_id, switch_video, sensor_mipi_switch_info);
	} else if (sensor_mipi_switch_info->switch_ctrl == GET_SWITCH) {
		memcpy(sensor_mipi_switch_info, &ctx->switch_info.mipi_switch_info, sizeof(struct sensor_mipi_switch_entity));
		sensor_mipi_switch_info->switch_ctrl = GET_SWITCH;
		isp_sensor_mipi_switch(isp, sensor_mipi_switch_info);
	}

	return 0;
}

#if ISP_AI_SCENE_CONF
HW_S32 isp_set_scene(int dev_id, scene_mode_t scene_mode)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *isp_gen;
	int i, mode, count = (media_params.isp_sync_mode >> 16) & 0xff;

	if (count > 1) {
			mode = media_params.isp_sync_mode & 0xffff;
			for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
				if (mode & (0x1 << i)) {
					if (media_params.isp_use_cnt[i] == 0) {
						ISP_ERR("isp%d is not used\n", i);
						return -1;
					}
					isp = media_params.isp_dev[i];
					if (!isp) {
						ISP_ERR("isp%d device is NULL!\n", i);
						return -1;
					}
					isp_gen = isp_dev_get_ctx(isp);
					isp_ctx_set_scene(isp_gen, scene_mode);
				}
			}
	} else {
		isp = media_params.isp_dev[dev_id];
		if (!isp) {
			ISP_ERR("isp%d device is NULL!\n", dev_id);
			return -1;
		}
		isp_gen = isp_dev_get_ctx(isp);
		isp_ctx_set_scene(isp_gen, scene_mode);
	}

	return 0;
}
#endif

/*******************get isp version*********************/
HW_S32 isp_get_version(char* version)
{
	sprintf(version, "ISP%d", ISP_VERSION);

	ISP_PRINT("ISP Version: ISP%d\n", ISP_VERSION);

	return 0;
}

/*******************set isp model*********************/
int isp_set_aetable(int dev_id, int enable)
{
	struct hw_isp_device *isp = NULL;
	struct v4l2_event event;
	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);

	memset(&event, 0, sizeof(event));
	event.id = V4L2_CID_SCENE_MODE;
	event.u.ctrl.value = enable;

	if(isp->ops->ctrl_process) {
        isp->ops->ctrl_process(isp, &event);
		isp_ctx_config_update(isp_gen);
	} else {
		ISP_ERR("set isp ae table fail!!!\n");
		return -1;
	}

	return 0;
}

