/*
 ******************************************************************************
 *
 * isp_manage.c
 *
 * Hawkview ISP - isp_manage.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/06/06	VIDEO INPUT
 *
 *****************************************************************************
 */

#include "../include/isp_comm.h"
#include "../include/isp_module_cfg.h"
#include "../include/isp_manage.h"
#include "../include/isp_debug.h"
#include "../include/isp_base.h"
#include "../isp_math/isp_math_util.h"
#include "isp_otp_golden.h"

#if ISP_AI_SCENE_CONF
#include "../include/ai_scene_param.h"
#endif
unsigned int isp_lib_log_param = 0;//0xffffffff;
/* ldci_video option */
#define LDCI_VIDEO_CHN 4

void isp_get_saved_regs(struct isp_lib_context *isp_gen)
{
	FUNCTION_LOG;
	//isp_gen->stat.min_rgb_saved = bsp_isp_get_saved_cfa_min_rgb();
	//isp_gen->stat.c_noise_saved = bsp_isp_get_saved_cnr_noise();
	FUNCTION_LOG;
}

void isp_rolloff_set_params_helper(isp_rolloff_entity_context_t *isp_rolloff_cxt, rolloff_param_type_t cmd_type)
{
	if (!isp_rolloff_cxt->rolloff_entity || !isp_rolloff_cxt->ops || !isp_rolloff_cxt->rolloff_param)
		return;

	isp_rolloff_cxt->rolloff_param->type = cmd_type;
	isp_rolloff_cxt->ops->isp_rolloff_set_params(isp_rolloff_cxt->rolloff_entity,
			isp_rolloff_cxt->rolloff_param, &isp_rolloff_cxt->rolloff_result);
}

void isp_afs_set_params_helper(isp_afs_entity_context_t *isp_afs_cxt, afs_param_type_t cmd_type)
{
	if (!isp_afs_cxt->afs_entity || !isp_afs_cxt->ops || !isp_afs_cxt->afs_param)
		return;

	isp_afs_cxt->afs_param->type = cmd_type;
	isp_afs_cxt->ops->isp_afs_set_params(isp_afs_cxt->afs_entity, isp_afs_cxt->afs_param, &isp_afs_cxt->afs_result);
}

void isp_iso_set_params_helper(isp_iso_entity_context_t *isp_iso_cxt, iso_param_type_t cmd_type)
{
	if (!isp_iso_cxt->iso_entity || !isp_iso_cxt->ops || !isp_iso_cxt->iso_param)
		return;

	isp_iso_cxt->iso_param->type = cmd_type;
	isp_iso_cxt->ops->isp_iso_set_params(isp_iso_cxt->iso_entity, isp_iso_cxt->iso_param, &isp_iso_cxt->iso_result);
}

void isp_md_set_params_helper(isp_md_entity_context_t *isp_md_cxt, md_param_type_t cmd_type)
{
	if (!isp_md_cxt->md_entity || !isp_md_cxt->ops || !isp_md_cxt->md_param)
		return;

	isp_md_cxt->md_param->type = cmd_type;
	isp_md_cxt->ops->isp_md_set_params(isp_md_cxt->md_entity, isp_md_cxt->md_param, &isp_md_cxt->md_result);
}

void isp_af_set_params_helper(isp_af_entity_context_t *isp_af_cxt, af_param_type_t cmd_type)
{
	if (!isp_af_cxt->af_entity || !isp_af_cxt->ops || !isp_af_cxt->af_param)
		return;

	isp_af_cxt->af_param->type = cmd_type;
	isp_af_cxt->ops->isp_af_set_params(isp_af_cxt->af_entity, isp_af_cxt->af_param, &isp_af_cxt->af_result);
}

void isp_awb_set_params_helper(isp_awb_entity_context_t *isp_awb_cxt, awb_param_type_t cmd_type)
{
	if (!isp_awb_cxt->awb_entity || !isp_awb_cxt->ops || !isp_awb_cxt->awb_param)
		return;

	isp_awb_cxt->awb_param->type = cmd_type;
	isp_awb_cxt->ops->isp_awb_set_params(isp_awb_cxt->awb_entity, isp_awb_cxt->awb_param, &isp_awb_cxt->awb_result);
}

void isp_ae_set_params_helper(isp_ae_entity_context_t *isp_ae_cxt, ae_param_type_t cmd_type)
{
	if (!isp_ae_cxt->ae_entity || !isp_ae_cxt->ops || !isp_ae_cxt->ae_param)
		return;

	isp_ae_cxt->ae_param->type = cmd_type;
	isp_ae_cxt->ops->isp_ae_set_params(isp_ae_cxt->ae_entity, isp_ae_cxt->ae_param, &isp_ae_cxt->ae_result);
}

void isp_gtm_set_params_helper(isp_gtm_entity_context_t *isp_gtm_cxt, gtm_param_type_t cmd_type)
{
	if (!isp_gtm_cxt->gtm_entity || !isp_gtm_cxt->ops || !isp_gtm_cxt->gtm_param)
		return;

	isp_gtm_cxt->gtm_param->type = cmd_type;
	isp_gtm_cxt->ops->isp_gtm_set_params(isp_gtm_cxt->gtm_entity, isp_gtm_cxt->gtm_param, &isp_gtm_cxt->gtm_result);
}

void isp_pltm_set_params_helper(isp_pltm_entity_context_t *isp_pltm_cxt, pltm_param_type_t cmd_type)
{
	if (!isp_pltm_cxt->pltm_entity || !isp_pltm_cxt->ops || !isp_pltm_cxt->pltm_param)
		return;

	isp_pltm_cxt->pltm_param->type = cmd_type;
	isp_pltm_cxt->ops->isp_pltm_set_params(isp_pltm_cxt->pltm_entity, isp_pltm_cxt->pltm_param, &isp_pltm_cxt->pltm_result);
}

void isp_ae_update_touch_cfg(struct isp_lib_context *isp_gen)
{
	static enum auto_focus_status af_status[5];
	static unsigned char cnt = 1;
	static unsigned char last_cnt = 0;
	unsigned char focus_change = 1;
	HW_U32 ae_gain = isp_gen->ae_entity_ctx.ae_result.ae_gain;
	unsigned char isp_ae_change_flags = 0;
	struct isp_d3d_k_stats_s *d3d_k_stats = &isp_gen->stats_ctx.stats.d3d_k_stats;
	HW_S16 af_stable_max = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_max;
	HW_S16 af_stable_min = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_min;
	HW_U32 i, j, mov_cnt = 0, mov = 0;
	unsigned char isp_mv_change_flags = 0;

	if (isp_gen->isp_ini_cfg.isp_test_settings.af_en == 1 && isp_gen->ae_settings.exp_metering_mode == AE_METERING_MODE_SPOT) {
		af_status[cnt] = isp_gen->af_entity_ctx.af_result.af_status_output;
		if (cnt == 0)
			last_cnt = 5 - 1;
		else
			last_cnt = cnt - 1;
		if (af_status[last_cnt] == AUTO_FOCUS_STATUS_REACHED && af_status[cnt] == AUTO_FOCUS_STATUS_BUSY)
			focus_change = 1;
		else
			focus_change = 0;
		cnt++;
		if (cnt == 5)
			cnt = 0;
		if (!focus_change)
			return;
	} else if (isp_gen->isp_ini_cfg.isp_test_settings.af_en == 0 && isp_gen->ae_settings.exp_metering_mode == AE_METERING_MODE_SPOT) {
		if (af_stable_max == 0 || af_stable_min == 0) {
			af_stable_max = 256 + 15;
			af_stable_min = 256 - 15;
		}
		if (ae_gain > af_stable_max || ae_gain < af_stable_min)
			isp_ae_change_flags = 1;
		else
			isp_ae_change_flags = 0;

		for (i = 0; i < ISP_D3D_K_ROW; i++) {
			for (j = 0; j < ISP_D3D_K_COL; j++) {
				mov_cnt += d3d_k_stats->k_stat[i][j];
			}
		}
		mov = 255 - (mov_cnt / (ISP_D3D_K_ROW * ISP_D3D_K_COL));
		if (mov > (isp_gen->isp_ini_cfg.isp_3a_settings.af_scene_motion_th * 3))
			isp_mv_change_flags = 1;
		else
			isp_mv_change_flags = 0;

		if (isp_mv_change_flags && !isp_ae_change_flags)
			ISP_PRINT("RESET TOUCH AE\n");
		else
			return;
	}

	if (isp_gen->ae_settings.exp_metering_mode == AE_METERING_MODE_SPOT) {
		isp_gen->ae_settings.exp_metering_mode = AE_METERING_MODE_MATRIX;
		isp_gen->ae_settings.ae_coor.x1 = H3A_PIC_OFFSET;
		isp_gen->ae_settings.ae_coor.y1 = H3A_PIC_OFFSET;
		isp_gen->ae_settings.ae_coor.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
		isp_gen->ae_settings.ae_coor.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
		isp_gen->ae_entity_ctx.ae_param->ae_setting = isp_gen->ae_settings;
		isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_BUILD_TOUCH_WEIGHT);
		//isp_gen->isp_3a_change_flags |= ISP_SET_AE_METERING_MODE;
		if (!isp_gen->ae_settings.exp_compensation) {
			isp_gen->pltm_entity_ctx.pltm_param->pltm_enable = 1;
			isp_gen->gtm_entity_ctx.gtm_param->gtm_enable = 1;
		}
	}
}

void isp_af_update_touch_cfg(struct isp_lib_context *isp_gen)
{
	static enum auto_focus_status af_status[5];
	static unsigned char cnt = 1;
	static unsigned char last_cnt = 0;
	unsigned char focus_change = 1;

	if (isp_gen->isp_ini_cfg.isp_test_settings.af_en == 1 && isp_gen->af_settings.af_mode == AUTO_FOCUS_TOUCH)  {
		af_status[cnt] = isp_gen->af_entity_ctx.af_result.af_status_output;
		if (cnt == 0)
			last_cnt = 5 - 1;
		else
			last_cnt = cnt - 1;
		if (af_status[last_cnt] == AUTO_FOCUS_STATUS_REACHED && af_status[cnt] == AUTO_FOCUS_STATUS_BUSY)
			focus_change = 1;
		else
			focus_change = 0;
		cnt++;
		if (cnt == 5)
			cnt = 0;
		if (!focus_change)
			return;
		else
			ISP_PRINT("RESET TOUCH AF\n");

	}

	if (isp_gen->af_settings.af_mode == AUTO_FOCUS_TOUCH) {
		isp_gen->af_settings.af_metering_mode = AUTO_FOCUS_METERING_CENTER_WEIGHTED;
		isp_gen->af_settings.af_mode = AUTO_FOCUS_CONTINUEOUS;
		isp_gen->af_settings.af_coor.x1 = H3A_PIC_OFFSET;
		isp_gen->af_settings.af_coor.y1 = H3A_PIC_OFFSET;
		isp_gen->af_settings.af_coor.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
		isp_gen->af_settings.af_coor.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
		isp_gen->af_entity_ctx.af_param->af_settings = isp_gen->af_settings;
		isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_BUILD_TOUCH_WEIGHT);
		//isp_gen->isp_3a_change_flags |= ISP_SET_AF_METERING_MODE;
	}
}

void __isp_iso_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->iso_entity_ctx.iso_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_gen->iso_entity_ctx.iso_param->iso_frame_id = isp_gen->iso_frame_cnt;
}

void __isp_iso_run(struct isp_lib_context *isp_gen)
{
	isp_iso_entity_context_t *isp_iso_cxt = &isp_gen->iso_entity_ctx;

	isp_iso_cxt->ops->isp_iso_run(isp_iso_cxt->iso_entity, &isp_iso_cxt->iso_result);
}

void __isp_rolloff_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->rolloff_entity_ctx.rolloff_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_gen->rolloff_entity_ctx.rolloff_param->rolloff_frame_id = isp_gen->rolloff_frame_cnt;
	//rolloff_sensor_info.
	isp_gen->rolloff_entity_ctx.rolloff_param->rolloff_sensor_info = isp_gen->sensor_info;
	//rolloff lsc_cfg.
	isp_gen->rolloff_entity_ctx.rolloff_param->rolloff_ctrl = isp_gen->alsc_settings;
}

void __isp_rolloff_run(struct isp_lib_context *isp_gen)
{
	unsigned short *lens_tbl = (unsigned short *)isp_gen->module_cfg.lens_table;
	int i;
	isp_rolloff_entity_context_t *isp_rolloff_cxt = &isp_gen->rolloff_entity_ctx;

	isp_rolloff_cxt->ops->isp_rolloff_run(isp_rolloff_cxt->rolloff_entity,
		&isp_rolloff_cxt->rolloff_stats, &isp_rolloff_cxt->rolloff_result);
}

void __isp_afs_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->afs_entity_ctx.afs_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_gen->afs_entity_ctx.afs_param->afs_frame_id = isp_gen->af_frame_cnt;
	isp_gen->afs_entity_ctx.afs_param->afs_sensor_info = isp_gen->sensor_info;
}

void __isp_afs_run(struct isp_lib_context *isp_gen)
{
	isp_afs_entity_context_t *isp_afs_cxt = &isp_gen->afs_entity_ctx;
	int ret = 0;

	if (isp_gen->ops->afs_done) {
		ret = isp_gen->ops->afs_done(isp_gen, &isp_afs_cxt->afs_result);
	}

	if (!ret) {
		isp_afs_cxt->ops->isp_afs_run(isp_afs_cxt->afs_entity, &isp_afs_cxt->afs_stats, &isp_afs_cxt->afs_result);
	}

	isp_gen->ae_settings.flicker_type = isp_afs_cxt->afs_result.flicker_type_output;
}

void __isp_md_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->md_entity_ctx.md_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_gen->md_entity_ctx.md_param->md_frame_id = isp_gen->md_frame_cnt;
	isp_gen->sensor_info.is_af_busy = (isp_gen->af_entity_ctx.af_result.af_status_output ==	AUTO_FOCUS_STATUS_BUSY) ? 1 : 0;
	isp_gen->md_entity_ctx.md_param->md_sensor_info = isp_gen->sensor_info;
}

void __isp_md_run(struct isp_lib_context *isp_gen)
{
	isp_md_entity_context_t *isp_md_cxt = &isp_gen->md_entity_ctx;
	int ret = 0;

	if (isp_gen->ops->md_done) {
		ret = isp_gen->ops->md_done(isp_gen, &isp_md_cxt->md_result);
	}
	if (!ret) {
		isp_md_cxt->ops->isp_md_run(isp_md_cxt->md_entity,
				&isp_md_cxt->md_stats, &isp_md_cxt->md_result);
	}
	isp_gen->sensor_info.motion_flag = isp_md_cxt->md_result.motion_flag;
}

void __isp_af_set_params(struct isp_lib_context *isp_gen)
{
	struct isp_d3d_k_stats_s *d3d_k_stats = &isp_gen->stats_ctx.stats.d3d_k_stats;
	HW_U32 i, j, mov_cnt = 0;
	isp_gen->af_entity_ctx.af_param->af_frame_id = isp_gen->af_frame_cnt;
	isp_gen->af_entity_ctx.af_param->focus_absolute = isp_gen->af_settings.focus_absolute;
	isp_gen->af_entity_ctx.af_param->focus_relative = isp_gen->af_settings.focus_relative;
	isp_gen->af_entity_ctx.af_param->af_run_mode = isp_gen->af_settings.af_mode;
	isp_gen->af_entity_ctx.af_param->af_metering_mode = isp_gen->af_settings.af_metering_mode;
	isp_gen->af_entity_ctx.af_param->af_range = isp_gen->af_settings.af_range;
	isp_gen->af_entity_ctx.af_param->focus_lock = isp_gen->af_settings.focus_lock;
	isp_gen->af_entity_ctx.af_param->sensor_info =	isp_gen->sensor_info;

	for (i = 0; i < ISP_D3D_K_ROW; i++) {
		for (j = 0; j < ISP_D3D_K_COL; j++) {
			mov_cnt += d3d_k_stats->k_stat[i][j];
		}
	}
	isp_gen->af_entity_ctx.af_param->mov = 255 - (mov_cnt / (ISP_D3D_K_ROW * ISP_D3D_K_COL));
	isp_af_update_touch_cfg(isp_gen);
}

void __isp_af_run(struct isp_lib_context *isp_gen)
{
	isp_af_entity_context_t *isp_af_cxt = &isp_gen->af_entity_ctx;
	int ret = 0;

	if (isp_gen->ops->af_done) {
		ret = isp_gen->ops->af_done(isp_gen, &isp_af_cxt->af_result);
	}

	if (!ret) {
		isp_af_cxt->ops->isp_af_run(isp_af_cxt->af_entity,
				&isp_af_cxt->af_stats, &isp_af_cxt->af_result);
	}

	isp_gen->module_cfg.af_cfg.af_sap_lim = isp_af_cxt->af_result.af_sap_lim_output;
}

void __isp_awb_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->awb_entity_ctx.awb_param->awb_frame_id = isp_gen->awb_frame_cnt;
	isp_gen->awb_entity_ctx.awb_param->awb_ctrl = isp_gen->awb_settings;
	//sensor info.
	isp_gen->awb_entity_ctx.awb_param->awb_sensor_info = isp_gen->sensor_info;
}

#if 0
static void __awb_comp(struct isp_lib_context *isp_gen, awb_stats_t *stats, awb_result_t *result) {
	HW_U32 i = 0, j = 0, n = 0, avg_r, avg_g, avg_b, satu, max_rgb, min_rgb;
	HW_U32 rgain = 256, bgain = 256;
	HW_U32 rg_sum_0 = 0, bg_sum_0 = 0, rg_sum_1 = 0, bg_sum_1 = 0, cnt_0 = 0, cnt_1 = 0;
	HW_U16 rg_avg_0, bg_avg_0, rg_avg_1, bg_avg_1, rgain_comp = 0, bgain_comp = 0;
	static HW_U16 rgain_comp_cur = 256, bgain_comp_cur = 256;

	if (isp_gen->awb_frame_cnt < 30 && isp_gen->isp_id != 0)
		return;

	rgain = div_round(isp_gen->stats_ctx.wb_gain_saved.r_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);
	bgain = div_round(isp_gen->stats_ctx.wb_gain_saved.b_gain << 8, isp_gen->stats_ctx.wb_gain_saved.gr_gain);

	for (n = 0; n < ISP_AWB_ROW*ISP_AWB_COL; n++) {
		i = n / ISP_AWB_COL;
		j = n % ISP_AWB_COL;

		avg_r = stats->awb_stats->awb_avg_r[i][j] * rgain >> 8;
		avg_g = stats->awb_stats->awb_avg_g[i][j];
		avg_b = stats->awb_stats->awb_avg_b[i][j] * bgain >> 8;
		max_rgb = max(avg_r, max(avg_g, avg_b));
		min_rgb = min(avg_r, min(avg_g, avg_b));
		if (max_rgb)
			satu = (max_rgb - min_rgb) * 100 / max_rgb;
		else
			satu = 0;

		if (max_rgb > 40 && max_rgb < 100 && satu < 10) {
			if (j < ISP_AWB_COL/2) {
				rg_sum_0 += div_round(avg_r << 8, avg_g);
				bg_sum_0 += div_round(avg_b << 8, avg_g);
				cnt_0++;
			} else {
				rg_sum_1 += div_round(avg_r << 8, avg_g);
				bg_sum_1 += div_round(avg_b << 8, avg_g);
				cnt_1++;
			}
		}
	}

	if (cnt_0 >= 50 && cnt_1 >= 50) {
		rg_avg_0 = rg_sum_0 / cnt_0;
		bg_avg_0 = bg_sum_0 / cnt_0;
		rg_avg_1 = rg_sum_1 / cnt_1;
		bg_avg_1 = bg_sum_1 / cnt_1;

		rgain_comp = div_round(rg_avg_0 << 8, rg_avg_1);
		bgain_comp = div_round(bg_avg_0 << 8, bg_avg_1);

		if (rgain_comp_cur > rgain_comp)
			rgain_comp_cur -= min(rgain_comp_cur - rgain_comp, 2);
		else
			rgain_comp_cur += min(rgain_comp - rgain_comp_cur, 2);

		if (bgain_comp_cur > bgain_comp)
			bgain_comp_cur -= min(bgain_comp_cur - bgain_comp, 2);
		else
			bgain_comp_cur += min(bgain_comp - bgain_comp_cur, 2);
	}

	result->wb_gain_output.r_gain = result->wb_gain_output.r_gain * rgain_comp_cur >> 8;
	result->wb_gain_output.b_gain = result->wb_gain_output.b_gain * bgain_comp_cur >> 8;

	ISP_LIB_LOG(ISP_LOG_AWB, "isp%d: cnt: %d, %d ; comp: %d %d -> %d %d\n", isp_gen->isp_id, cnt_0, cnt_1, rgain_comp_cur, bgain_comp_cur, rgain_comp, bgain_comp);
}
#endif

void __isp_awb_run(struct isp_lib_context *isp_gen)
{
	isp_awb_entity_context_t *isp_awb_cxt = &isp_gen->awb_entity_ctx;
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;
	struct isp_module_config *mod_cfg = &isp_gen->module_cfg;
	int ret = 0;

	if (isp_gen->ops->awb_done) {
		ret = isp_gen->ops->awb_done(isp_gen, &isp_awb_cxt->awb_result);
	}

	if (!ret) {
		isp_awb_cxt->ops->isp_awb_run(isp_awb_cxt->awb_entity, &isp_awb_cxt->awb_stats, &isp_awb_cxt->awb_result);
	} else {
		if (!isp_awb_cxt->awb_result.wb_gain_output.r_gain && !isp_awb_cxt->awb_result.wb_gain_output.gr_gain
			&& !isp_awb_cxt->awb_result.wb_gain_output.gb_gain && !isp_awb_cxt->awb_result.wb_gain_output.b_gain) {
			//Forced to do once awb
			isp_awb_cxt->ops->isp_awb_run(isp_awb_cxt->awb_entity, &isp_awb_cxt->awb_stats, &isp_awb_cxt->awb_result);
		} else {
			isp_awb_cxt->awb_param->awb_rest_en = 1;
		}

		//awb dynamic compensation, use for two camera stitching camera
		//__awb_comp(isp_gen, &isp_awb_cxt->awb_stats, &isp_awb_cxt->awb_result);
	}

	if (isp_gen->isp_ini_cfg.isp_3a_settings.awb_ext_light_num == 1) {
		if (isp_gen->ops->awb_limit) {
			isp_gen->ops->awb_limit(isp_gen, &isp_awb_cxt->awb_result);
		}
	}

	if (param->isp_test_settings.nrp_en && mod_cfg->nrp_cfg.gamma_en &&
			!mod_cfg->nrp_cfg.inv_gamma_en && mod_cfg->mode_cfg.wb_sel) {
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.r_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.r_gain / 256.0, 1/1.3) * 256.0);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.gr_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gr_gain / 256.0, 1/1.3) * 256.0);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.gb_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gb_gain / 256.0, 1/1.3) * 256.0);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.b_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.b_gain / 256.0, 1/1.3) * 256.0);
	} else if (isp_gen->ai_isp_en) {
		if (isp_gen->ai_isp_en == 1) {
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.r_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.r_gain / 256.0, 1/2.2) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gr_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gr_gain / 256.0, 1/2.2) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gb_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gb_gain / 256.0, 1/2.2) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.b_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.b_gain / 256.0, 1/2.2) * 256.0);
		} else if (isp_gen->ai_isp_en == 2) {
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.r_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.r_gain / 256.0, 1/1.69) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gr_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gr_gain / 256.0, 1/1.69) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gb_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gb_gain / 256.0, 1/1.69) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.b_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.b_gain / 256.0, 1/1.69) * 256.0);
		} else if (isp_gen->ai_isp_en == 3) {
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.r_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.r_gain / 256.0, 1/1.3) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gr_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gr_gain / 256.0, 1/1.3) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.gb_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.gb_gain / 256.0, 1/1.3) * 256.0);
			isp_gen->module_cfg.wb_gain_cfg.wb_gain.b_gain = (HW_U16)(powf((float)isp_awb_cxt->awb_result.wb_gain_output.b_gain / 256.0, 1/1.3) * 256.0);
		}
	} else {
		isp_gen->module_cfg.wb_gain_cfg.wb_gain = isp_awb_cxt->awb_result.wb_gain_output;
	}

	isp_gen->stats_ctx.wb_gain_saved = isp_awb_cxt->awb_result.wb_gain_output;
}

void __isp_ae_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->ae_entity_ctx.ae_param->ae_frame_id = isp_gen->ae_frame_cnt;
	isp_gen->ae_entity_ctx.ae_param->ae_isp_id = isp_gen->isp_id;
	isp_gen->ae_entity_ctx.ae_param->ae_setting = isp_gen->ae_settings;
	//ae_sensor_info.
	isp_gen->ae_entity_ctx.ae_param->ae_sensor_info = isp_gen->sensor_info;
	isp_gen->ae_entity_ctx.ae_param->ae_target_comp = 0;
	isp_ae_update_touch_cfg(isp_gen);
}

#if ISP_AI_SCENE_CONF
void __isp_set_scene_param(struct isp_lib_context *isp_gen)
{
	if (isp_gen->ai_scene.scene_change_flag) {
		isp_gen->ai_scene.scene_frame_cnt++;
		if (isp_gen->ai_scene.cur_scene == BLUE_SKY) {
#ifdef AI_SCENE_SMOOTH
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			if (isp_gen->ai_scene.scene_frame_cnt <= blue_sky_tune_settings.isp_ai_smooth_frames) {
				isp_gen->tune.saturation_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.saturation_level, blue_sky_tune_settings.saturation_level);
				isp_gen->tune.hue_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.hue_level, blue_sky_tune_settings.hue_level);
				isp_gen->tune.contrast_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.contrast_level, blue_sky_tune_settings.contrast_level);
				isp_gen->tune.sharpness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.sharpness_level, blue_sky_tune_settings.sharpness_level);
				isp_gen->tune.brightness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.brightness_level, blue_sky_tune_settings.brightness_level);
				isp_gen->tune.denoise_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.denoise_level, blue_sky_tune_settings.denoise_level);
				isp_gen->tune.tdf_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, blue_sky_tune_settings.isp_ai_smooth_frames, last_tune_settings.tdf_level, blue_sky_tune_settings.tdf_level);
			}
			if (isp_gen->ai_scene.scene_frame_cnt == blue_sky_tune_settings.isp_ai_smooth_frames) {
				last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
				last_tune_settings.hue_level = isp_gen->tune.hue_level;
				last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
				last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
				last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
				last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
				last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
				// last_tune_settings.ae_table_night_length = blue_sky_tune_settings.ae_table_night_length;
				// memcpy(&last_tune_settings.ae_ai_night_table[0], &blue_sky_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_gen->ai_scene.scene_change_flag = 0;
				isp_gen->ai_scene.scene_frame_cnt = 0;
			}
#else
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			isp_gen->tune.saturation_level = blue_sky_tune_settings.saturation_level;
			isp_gen->tune.hue_level = blue_sky_tune_settings.hue_level;
			isp_gen->tune.contrast_level = blue_sky_tune_settings.contrast_level;
			isp_gen->tune.sharpness_level = blue_sky_tune_settings.sharpness_level;
			isp_gen->tune.brightness_level = blue_sky_tune_settings.brightness_level;
			isp_gen->tune.denoise_level = blue_sky_tune_settings.denoise_level;
			isp_gen->tune.tdf_level = blue_sky_tune_settings.tdf_level;
			isp_gen->ai_scene.scene_change_flag = 0;
			isp_gen->ai_scene.scene_frame_cnt = 0;
#endif
		}

		if (isp_gen->ai_scene.cur_scene == NIGHT_VIEW) {
#ifdef AI_SCENE_SMOOTH
			if (isp_gen->ai_scene.scene_frame_cnt == 1) {
				isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_CAPTURE;
				isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length = night_view_tune_settings.ae_table_night_length;
				memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[0],
					&night_view_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);
			}
			if (isp_gen->ai_scene.scene_frame_cnt <= night_view_tune_settings.isp_ai_smooth_frames) {
				isp_gen->tune.saturation_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.saturation_level, night_view_tune_settings.saturation_level);
				isp_gen->tune.hue_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.hue_level, night_view_tune_settings.hue_level);
				isp_gen->tune.contrast_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.contrast_level, night_view_tune_settings.contrast_level);
				isp_gen->tune.sharpness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.sharpness_level, night_view_tune_settings.sharpness_level);
				isp_gen->tune.brightness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.brightness_level, night_view_tune_settings.brightness_level);
				isp_gen->tune.denoise_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.denoise_level, night_view_tune_settings.denoise_level);
				isp_gen->tune.tdf_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, night_view_tune_settings.isp_ai_smooth_frames, last_tune_settings.tdf_level, night_view_tune_settings.tdf_level);
			}
			if (isp_gen->ai_scene.scene_frame_cnt == night_view_tune_settings.isp_ai_smooth_frames) {
				last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
				last_tune_settings.hue_level = isp_gen->tune.hue_level;
				last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
				last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
				last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
				last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
				last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
				last_tune_settings.ae_table_night_length =
					night_view_tune_settings.ae_table_night_length;
				memcpy(&last_tune_settings.ae_ai_night_table[0],
					&night_view_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_gen->ai_scene.scene_change_flag = 0;
				isp_gen->ai_scene.scene_frame_cnt = 0;
			}
#else
			isp_gen->tune.saturation_level = night_view_tune_settings.saturation_level;
			isp_gen->tune.hue_level = night_view_tune_settings.hue_level;
			isp_gen->tune.contrast_level = night_view_tune_settings.contrast_level;
			isp_gen->tune.sharpness_level = night_view_tune_settings.sharpness_level;
			isp_gen->tune.brightness_level = night_view_tune_settings.brightness_level;
			isp_gen->tune.denoise_level = night_view_tune_settings.denoise_level;
			isp_gen->tune.tdf_level = night_view_tune_settings.tdf_level;

			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_CAPTURE;
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length =
				night_view_tune_settings.ae_table_night_length;
			memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[0],
				&night_view_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
			isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);

			isp_gen->ai_scene.scene_change_flag = 0;
			isp_gen->ai_scene.scene_frame_cnt = 0;
#endif
		}

		if (isp_gen->ai_scene.cur_scene == PORTRAIT) {
#ifdef AI_SCENE_SMOOTH
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			if (isp_gen->ai_scene.scene_frame_cnt <= portrait_tune_settings.isp_ai_smooth_frames) {
				isp_gen->tune.saturation_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.saturation_level, portrait_tune_settings.saturation_level);
				isp_gen->tune.hue_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.hue_level, portrait_tune_settings.hue_level);
				isp_gen->tune.contrast_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.contrast_level, portrait_tune_settings.contrast_level);
				isp_gen->tune.sharpness_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.sharpness_level, portrait_tune_settings.sharpness_level);
				isp_gen->tune.brightness_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.brightness_level, portrait_tune_settings.brightness_level);
				isp_gen->tune.denoise_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.denoise_level, portrait_tune_settings.denoise_level);
				isp_gen->tune.tdf_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, portrait_tune_settings.isp_ai_smooth_frames, last_tune_settings.tdf_level, portrait_tune_settings.tdf_level);
			}
			if(isp_gen->ai_scene.scene_frame_cnt == portrait_tune_settings.isp_ai_smooth_frames) {
				last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
				last_tune_settings.hue_level = isp_gen->tune.hue_level;
				last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
				last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
				last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
				last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
				last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
				last_tune_settings.ae_table_night_length = portrait_tune_settings.ae_table_night_length;
				memcpy(&last_tune_settings.ae_ai_night_table[0], &portrait_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_gen->ai_scene.scene_change_flag = 0;
				isp_gen->ai_scene.scene_frame_cnt = 0;
			}
#else
			isp_gen->tune.saturation_level = portrait_tune_settings.saturation_level;
			isp_gen->tune.hue_level = portrait_tune_settings.hue_level;
			isp_gen->tune.contrast_level = portrait_tune_settings.contrast_level;
			isp_gen->tune.sharpness_level = portrait_tune_settings.sharpness_level;
			isp_gen->tune.brightness_level = portrait_tune_settings.brightness_level;
			isp_gen->tune.denoise_level = portrait_tune_settings.denoise_level;
			isp_gen->tune.tdf_level = portrait_tune_settings.tdf_level;
			isp_gen->ai_scene.scene_change_flag = 0;
			isp_gen->ai_scene.scene_frame_cnt = 0;
#endif
		}

		if (isp_gen->ai_scene.cur_scene == NORMAL) {
#ifdef AI_SCENE_SMOOTH
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			if (isp_gen->scene_frame_cnt == 1) {
				if (memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0],
					&normal_tune_settings.ae_ai_night_table[0], 42*sizeof(int))) {
					isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);
				}
			}
			if (isp_gen->scene_frame_cnt <= normal_tune_settings.isp_ai_smooth_frames) {
				isp_gen->tune.saturation_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.saturation_level, normal_tune_settings.saturation_level);
				isp_gen->tune.hue_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.hue_level, normal_tune_settings.hue_level);
				isp_gen->tune.contrast_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.contrast_level, normal_tune_settings.contrast_level);
				isp_gen->tune.sharpness_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.sharpness_level, normal_tune_settings.sharpness_level);
				isp_gen->tune.brightness_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.brightness_level, normal_tune_settings.brightness_level);
				isp_gen->tune.denoise_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.denoise_level, normal_tune_settings.denoise_level);
				isp_gen->tune.tdf_level =
					ValueInterp(isp_gen->scene_frame_cnt, 0, normal_tune_settings.isp_ai_smooth_frames, last_tune_settings.tdf_level, normal_tune_settings.tdf_level);
			}
			if (isp_gen->ai_scene.scene_frame_cnt == normal_tune_settings.isp_ai_smooth_frames) {
				last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
				last_tune_settings.hue_level = isp_gen->tune.hue_level;
				last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
				last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
				last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
				last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
				last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
				last_tune_settings.ae_table_night_length = normal_tune_settings.ae_table_night_length;
				memcpy(&last_tune_settings.ae_ai_night_table[0], &normal_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_gen->ai_scene.scene_change_flag = 0;
				isp_gen->ai_scene.scene_frame_cnt = 0;
			}
#else
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			isp_gen->tune.saturation_level = normal_tune_settings.saturation_level;
			isp_gen->tune.hue_level = normal_tune_settings.hue_level;
			isp_gen->tune.contrast_level = normal_tune_settings.contrast_level;
			isp_gen->tune.sharpness_level = normal_tune_settings.sharpness_level;
			isp_gen->tune.brightness_level = normal_tune_settings.brightness_level;
			isp_gen->tune.denoise_level = normal_tune_settings.denoise_level;
			isp_gen->tune.tdf_level = normal_tune_settings.tdf_level;
			memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0],
				&normal_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
			isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);
			isp_gen->ai_scene.scene_change_flag = 0;
			isp_gen->ai_scene.scene_frame_cnt = 0;
#endif
		}

		if (isp_gen->ai_scene.cur_scene == GREEN_PLANTS) {
#ifdef AI_SCENE_SMOOTH
			// isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			if (isp_gen->ai_scene.scene_frame_cnt <= green_plants_tune_settings.isp_ai_smooth_frames) {
				isp_gen->tune.saturation_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.saturation_level, green_plants_tune_settings.saturation_level);
				isp_gen->tune.hue_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.hue_level, green_plants_tune_settings.hue_level);
				isp_gen->tune.contrast_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.contrast_level, green_plants_tune_settings.contrast_level);
				isp_gen->tune.sharpness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.sharpness_level, green_plants_tune_settings.sharpness_level);
				isp_gen->tune.brightness_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.brightness_level, green_plants_tune_settings.brightness_level);
				isp_gen->tune.denoise_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.denoise_level, green_plants_tune_settings.denoise_level);
				isp_gen->tune.tdf_level =
					ValueInterp(isp_gen->ai_scene.scene_frame_cnt, 0, green_plants_tune_settings.isp_ai_smooth_frames, last_tune_settings.tdf_level, green_plants_tune_settings.tdf_level);
			}
			if (isp_gen->ai_scene.scene_frame_cnt == green_plants_tune_settings.isp_ai_smooth_frames) {
				last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
				last_tune_settings.hue_level = isp_gen->tune.hue_level;
				last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
				last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
				last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
				last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
				last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
				last_tune_settings.ae_table_night_length = green_plants_tune_settings.ae_table_night_length;
				memcpy(&last_tune_settings.ae_ai_night_table[0],
					&green_plants_tune_settings.ae_ai_night_table[0], 42*sizeof(int));
				isp_gen->ai_scene.scene_change_flag = 0;
				isp_gen->ai_scene.scene_frame_cnt = 0;
			}
#else
			isp_gen->ae_entity_ctx.ae_param->ae_setting.scene_mode = SCENE_MODE_PREVIEW;
			isp_gen->tune.saturation_level = green_plants_tune_settings.saturation_level;
			isp_gen->tune.hue_level = green_plants_tune_settings.hue_level;
			isp_gen->tune.contrast_level = normal_tune_settings.contrast_level;
			isp_gen->tune.sharpness_level = green_plants_tune_settings.sharpness_level;
			isp_gen->tune.brightness_level = green_plants_tune_settings.brightness_level;
			isp_gen->tune.denoise_level = green_plants_tune_settings.denoise_level;
			isp_gen->tune.tdf_level = green_plants_tune_settings.tdf_level;
			isp_gen->ai_scene.scene_change_flag = 0;
			isp_gen->ai_scene.scene_frame_cnt = 0;
#endif
		}

		ISP_PRINT("#### cur_scene_frame_cnt:%d saturation_level:%d hue_level:%d contrast_level:%d "
			"sharpness_level:%d denoise_level:%d brightness_level:%d tdf_level:%d scene_mode:%d \n",
			isp_gen->ai_scene.scene_frame_cnt, isp_gen->tune.saturation_level,isp_gen->tune.hue_level,
			isp_gen->tune.contrast_level,isp_gen->tune.sharpness_level, isp_gen->tune.denoise_level,
			isp_gen->tune.brightness_level, isp_gen->tune.tdf_level, isp_gen->ae_settings.scene_mode);
	}
}
#endif

void __isp_ae_update_sensor_info(struct isp_lib_context *isp_gen)
{
	isp_sensor_info_t *sensor_info = &isp_gen->sensor_info;
	ae_result_t *result = &isp_gen->ae_entity_ctx.ae_result;
	HW_U32 total_vts = isp_gen->sensor_info.exp_line / 16;

	sensor_info->exp_line = result->sensor_set.ev_set_curr.ev_sensor_exp_line;
	sensor_info->ang_gain = result->sensor_set.ev_set_curr.ev_analog_gain;
	sensor_info->dig_gain = result->sensor_set.ev_set_curr.ev_digital_gain;
	sensor_info->total_gain = result->sensor_set.ev_set_curr.ev_total_gain;

	sensor_info->ae_tbl_idx = result->sensor_set.ev_set_curr.ev_idx;
	sensor_info->ae_tbl_idx_max = result->sensor_set.ev_idx_max;

	sensor_info->is_ae_done = (result->ae_status == AE_STATUS_DONE) ? 1 : 0;
	sensor_info->backlight = result->backlight;
	sensor_info->ae_gain = result->ae_gain;
	sensor_info->ae_lv = result->sensor_set.ev_set_curr.ev_lv;

	if (sensor_info->vts && sensor_info->hts && sensor_info->pclk) {
		total_vts = sensor_info->vts > total_vts ? sensor_info->vts : total_vts;
		sensor_info->fps = sensor_info->pclk / (total_vts * sensor_info->hts);
	} else {
		sensor_info->fps = 30;
	}
	sensor_info->fps = clamp(sensor_info->fps, 1, 1000);
	sensor_info->frame_time = 1000 / sensor_info->fps; /*ms*/
}

void __isp_ae_run(struct isp_lib_context *isp_gen)
{
	isp_ae_entity_context_t *isp_ae_cxt = &isp_gen->ae_entity_ctx;
	int ret = 0;

	if (isp_gen->ops->ae_done) {
		ret = isp_gen->ops->ae_done(isp_gen, &isp_ae_cxt->ae_result);
	}

	if (isp_gen->ae_settings.ae_face_disappear_flag &&
		(!isp_gen->ae_settings.ae_face_disappear_job_done)) {
		if (--isp_gen->ae_settings.ae_face_delay_cnt < 1) {
			/* reset disappear_flag and delay_cnt */
			isp_gen->ae_settings.ae_face_disappear_job_done = 1;
			isp_gen->ae_settings.ae_face_disappear_flag = 0;
			isp_gen->ae_settings.ae_face_for_detect_flag = 1;
			isp_gen->ae_settings.ae_face_appear_cnt = 0;
			isp_gen->ae_settings.ae_face_delay_cnt = isp_gen->ae_settings.face_cfg.face_ae_delay_cnt;
			/* reset exp_metering_mode to AE_METERING_MODE_MATRIX */
			isp_gen->ae_settings.exp_metering_mode = AE_METERING_MODE_MATRIX;
			isp_gen->ae_settings.exposure_lock = false;
			isp_gen->isp_3a_change_flags |= ISP_SET_AE_METERING_MODE;
			ISP_PRINT("[done] ae_face_for_detect_flag = %d, ae_face_disappear_flag = %d, entity->ae_face_delay_cnt = %d, ae_face_disappear_job_done = %d\n",
			isp_gen->ae_settings.ae_face_for_detect_flag,
			isp_gen->ae_settings.ae_face_disappear_flag,
			isp_gen->ae_settings.ae_face_delay_cnt,
			isp_gen->ae_settings.ae_face_disappear_job_done);
			return;
		}
		/* output save target */
		isp_gen->ae_entity_ctx.ae_result.ae_target = isp_gen->ae_settings.ae_face_last_target;
		return;
	} else {
		if (!ret) {
			isp_ae_cxt->ops->isp_ae_run(isp_ae_cxt->ae_entity,
					&isp_ae_cxt->ae_stats, &isp_ae_cxt->ae_result);
		} else {
			if (isp_gen->ae_frame_cnt == 0)
				isp_ae_cxt->ops->isp_ae_run(isp_ae_cxt->ae_entity,
						&isp_ae_cxt->ae_stats, &isp_ae_cxt->ae_result);
		}
	}

	__isp_ae_update_sensor_info(isp_gen);
	config_gamma(isp_gen);
	config_dig_gain(isp_gen, isp_ae_cxt->ae_result.sensor_set.ev_set_curr.ev_digital_gain,
		isp_ae_cxt->ae_result.sensor_set_short.ev_set_curr.ev_digital_gain);
	config_wdr(isp_gen, 0);
}

void __isp_gtm_set_params(struct isp_lib_context *isp_gen)
{
	isp_gen->gtm_entity_ctx.gtm_param->gtm_frame_id = isp_gen->gtm_frame_cnt;
	isp_gen->gtm_entity_ctx.gtm_param->contrast = isp_gen->adjust.contrast;
	isp_gen->gtm_entity_ctx.gtm_param->brightness = isp_gen->adjust.brightness;
	isp_gen->gtm_entity_ctx.gtm_param->BrightPixellValue =	isp_gen->ae_entity_ctx.ae_result.BrightPixellValue;
	isp_gen->gtm_entity_ctx.gtm_param->DarkPixelValue = isp_gen->ae_entity_ctx.ae_result.DarkPixelValue;
	isp_gen->gtm_entity_ctx.gtm_param->wdr_mode = isp_gen->sensor_info.wdr_mode;
	//gtm_ini_cfg.
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.hist_sel = isp_gen->isp_ini_cfg.isp_tunning_settings.gtm_hist_sel;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gtm_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gtm_type;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gamma_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gamma_type;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.AutoAlphaEn	= isp_gen->isp_ini_cfg.isp_tunning_settings.auto_alpha_en;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.hist_pix_cnt = isp_gen->isp_ini_cfg.isp_tunning_settings.hist_pix_cnt; //[0, 256]
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.dark_minval = isp_gen->isp_ini_cfg.isp_tunning_settings.dark_minval; //[0, 128]
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.bright_minval = isp_gen->isp_ini_cfg.isp_tunning_settings.bright_minval; //[0, 128]
	if (isp_gen->isp_ini_cfg.isp_test_settings.awb_en)
		isp_gen->gtm_entity_ctx.gtm_param->ldci_work_en = isp_gen->awb_entity_ctx.awb_param->awb_rest_en;
	else
		isp_gen->gtm_entity_ctx.gtm_param->ldci_work_en = 1;
	isp_gen->gtm_entity_ctx.gtm_param->current_colorspace = isp_gen->sensor_info.color_space;
	isp_gen->gtm_entity_ctx.gtm_param->ldci_video_chn = LDCI_VIDEO_CHN + isp_gen->isp_id;
	isp_gen->gtm_entity_ctx.gtm_param->ldci_entity_id = isp_gen->isp_id;
	memcpy(&isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.plum_var[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.plum_var[0], GTM_LUM_IDX_NUM*GTM_VAR_IDX_NUM*sizeof(HW_S16));
	memcpy(&isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gtm_cfg[0], &isp_gen->ae_settings.ae_hist_eq_cfg[0], ISP_GTM_HEQ_MAX*sizeof(HW_S16));
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_INI_DATA);
}

void __isp_gtm_run(struct isp_lib_context *isp_gen)
{
	isp_gtm_entity_context_t *isp_gtm_cxt = &isp_gen->gtm_entity_ctx;

	if (isp_gen->ops->gtm_done) {
		isp_gen->ops->gtm_done(isp_gen, &isp_gtm_cxt->gtm_result);
	}

	isp_gtm_cxt->ops->isp_gtm_run(isp_gtm_cxt->gtm_entity,
				&isp_gtm_cxt->gtm_stats,
				&isp_gtm_cxt->gtm_result);
}

void __isp_pltm_set_params(struct isp_lib_context *isp_gen)
{
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;
	isp_gen->pltm_entity_ctx.pltm_param->pltm_frame_id = isp_gen->ae_frame_cnt;

	//pltm sensor_info.
	isp_gen->pltm_entity_ctx.pltm_param->sensor_info = isp_gen->sensor_info;
	//pltm_ini_cfg.
	isp_gen->pltm_entity_ctx.pltm_param->pltm_max_stage = isp_gen->module_cfg.pltm_cfg.max_stage + 1;
	isp_gen->pltm_entity_ctx.pltm_param->hist0_sel = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist0_sel;
	isp_gen->pltm_entity_ctx.pltm_param->hist1_sel = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist1_sel;
	isp_gen->pltm_entity_ctx.pltm_param->ae_lum_idx = isp_gen->iso_entity_ctx.iso_result.lum_idx;
	isp_gen->pltm_entity_ctx.pltm_param->wdr_stitch_en = isp_gen->isp_ini_cfg.isp_test_settings.wdr_stitch_en;
	isp_gen->pltm_entity_ctx.pltm_param->wdr_ratio_real = isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.real;
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param->pltm_ini.pltm_cfg[0],
			&param->isp_tunning_settings.pltm_cfg[0], ISP_PLTM_MAX * sizeof(HW_S32));
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param->pltm_ini.pltm_dynamic_cfg[0],
		&isp_gen->ae_settings.pltm_dynamic_cfg[0], ISP_PLTM_DYNAMIC_MAX*sizeof(HW_S16));
}

void __isp_pltm_run(struct isp_lib_context *isp_gen)
{
	pltm_result_t *result = &isp_gen->pltm_entity_ctx.pltm_result;
	struct isp_pltm_config *pltm_cfg = &isp_gen->module_cfg.pltm_cfg;
	isp_pltm_entity_context_t *isp_pltm_ctx = &isp_gen->pltm_entity_ctx;
	int ret = 0, dcc_ratio;

	if (isp_gen->ops->pltm_done) {
		ret = isp_gen->ops->pltm_done(isp_gen, &isp_pltm_ctx->pltm_result);
	}
	if (!ret) {
		isp_pltm_ctx->ops->isp_pltm_run(isp_pltm_ctx->pltm_entity,
			&isp_pltm_ctx->pltm_stats, &isp_pltm_ctx->pltm_result);
	}

	pltm_cfg->strength = result->pltm_auto_stren;
	if (isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_HDR_RATIO]) {
		if (result->pltm_hdr_ratio)
			pltm_cfg->dcc_lw_ratio = min(255, 2 * (isp_pltm_ctx->pltm_param->pltm_ini.pltm_dynamic_cfg[ISP_PLTM_DYNAMIC_DCC_LW_RT] << 16) / result->pltm_hdr_ratio);
		else
			pltm_cfg->dcc_lw_ratio = 255;
	} else {
		pltm_cfg->dcc_lw_ratio = isp_pltm_ctx->pltm_param->pltm_ini.pltm_dynamic_cfg[ISP_PLTM_DYNAMIC_DCC_LW_RT];
	}
	if (pltm_cfg->dcc_hi_th != pltm_cfg->dcc_lw_th)
		pltm_cfg->dcc_slp = ((256 - pltm_cfg->dcc_lw_ratio) << 8) / (pltm_cfg->dcc_hi_th - pltm_cfg->dcc_lw_th);
	else
		pltm_cfg->dcc_slp = 132;

	ISP_LIB_LOG(ISP_LOG_PLTM, "dcc_lw_ratio = %d, hdr dcc_lw_ratio = %d\n", isp_pltm_ctx->pltm_param->pltm_ini.pltm_dynamic_cfg[ISP_PLTM_DYNAMIC_DCC_LW_RT], pltm_cfg->dcc_lw_ratio);
}

void __isp_ctx_cfg_lib(struct isp_lib_context *isp_gen)
{
	//struct isp_param_config *param = &isp_gen->isp_ini_cfg;

	// isp_lib_context def settings
	isp_gen->adjust.contrast   = 0;
	isp_gen->adjust.brightness = 0;
	isp_gen->adjust.defog_value = 0;

	isp_gen->tune.contrast_level = 256;
	isp_gen->tune.brightness_level = 256;
	isp_gen->tune.sharpness_level = 256;
	isp_gen->tune.saturation_level = 256;
	isp_gen->tune.tdf_level = 256;
	isp_gen->tune.denoise_level = 256;
	isp_gen->tune.pltmwdr_level = 0;
	isp_gen->tune.effect = ISP_COLORFX_NONE;
	isp_gen->VencVe2IspParam.d2d_level = 256;
	isp_gen->VencVe2IspParam.d3d_level = 256;
	isp_gen->VencVe2IspParam.mMovingLevelInfo.is_overflow = 0;
	isp_gen->npu_nr_cfg.roi_num = 0;

	isp_gen->awb_settings.wb_gain_manual.r_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.gr_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.gb_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.b_gain = 256;

#if ISP_AI_SCENE_CONF
	// for ai scene param
	isp_gen->ai_scene.last_tune_settings.contrast_level = isp_gen->tune.contrast_level;
	isp_gen->ai_scene.last_tune_settings.saturation_level = isp_gen->tune.saturation_level;
	isp_gen->ai_scene.last_tune_settings.brightness_level = isp_gen->tune.brightness_level;
	isp_gen->ai_scene.last_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
	isp_gen->ai_scene.last_tune_settings.tdf_level = isp_gen->tune.tdf_level;
	isp_gen->ai_scene.last_tune_settings.denoise_level = isp_gen->tune.denoise_level;
	isp_gen->ai_scene.last_tune_settings.hue_level = isp_gen->tune.hue_level;
	normal_tune_settings.contrast_level = isp_gen->tune.contrast_level;
	normal_tune_settings.saturation_level = isp_gen->tune.saturation_level;
	normal_tune_settings.brightness_level = isp_gen->tune.brightness_level;
	normal_tune_settings.sharpness_level = isp_gen->tune.sharpness_level;
	normal_tune_settings.tdf_level = isp_gen->tune.tdf_level;
	normal_tune_settings.denoise_level = isp_gen->tune.denoise_level;
	normal_tune_settings.hue_level = isp_gen->tune.hue_level;
	isp_gen->ai_scene.scene_frame_cnt = 0;
	isp_gen->ai_scene.scene_change_flag = 0;
	isp_gen->ai_scene.cur_scene = NORMAL;
	isp_gen->ae_settings.scene_mode = SCENE_MODE_PREVIEW;
#endif

	// exp settings
	isp_gen->ae_settings.exp_mode = EXP_AUTO;
	isp_gen->ae_settings.exp_metering_mode = AE_METERING_MODE_MATRIX;
	isp_gen->ae_settings.flash_mode = FLASH_MODE_NONE;
	isp_gen->ae_settings.flash_switch_flag = false;
	isp_gen->ae_settings.flash_open = 0;
	isp_gen->ae_settings.flicker_mode = FREQUENCY_AUTO;
	isp_gen->ae_settings.iso_mode = ISO_AUTO;
	isp_gen->ae_settings.exposure_lock = false;
	isp_gen->ae_settings.exp_compensation = 0;
	isp_gen->ae_settings.ae_face_for_detect_flag = 1;

	// af settings
	isp_gen->af_settings.af_mode = AUTO_FOCUS_CONTINUEOUS;
	isp_gen->af_settings.af_metering_mode = AUTO_FOCUS_METERING_CENTER_WEIGHTED;
	isp_gen->af_settings.focus_lock = false;
	isp_gen->af_settings.af_range = AUTO_FOCUS_RANGE_AUTO;

	// awb settings
	isp_gen->awb_settings.wb_mode = WB_AUTO;
	isp_gen->awb_settings.white_balance_lock = false;

	isp_gen->awb_settings.awb_coor.x1 = H3A_PIC_OFFSET;
	isp_gen->awb_settings.awb_coor.y1 = H3A_PIC_OFFSET;
	isp_gen->awb_settings.awb_coor.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
	isp_gen->awb_settings.awb_coor.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
	isp_gen->ae_settings.ae_coor.x1 = H3A_PIC_OFFSET;
	isp_gen->ae_settings.ae_coor.y1 = H3A_PIC_OFFSET;
	isp_gen->ae_settings.ae_coor.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
	isp_gen->ae_settings.ae_coor.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
	memset(&(isp_gen->ae_settings.face_cfg), 0, sizeof(struct ae_face_cfg));

	isp_gen->af_settings.af_coor.x1 = H3A_PIC_OFFSET;
	isp_gen->af_settings.af_coor.y1 = H3A_PIC_OFFSET;
	isp_gen->af_settings.af_coor.x2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;
	isp_gen->af_settings.af_coor.y2 = H3A_PIC_SIZE + H3A_PIC_OFFSET;

	isp_gen->stat.min_rgb_saved = 1023;
	isp_gen->stat.c_noise_saved = 20;

	isp_gen->stats_ctx.wb_gain_saved.r_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.gr_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.gb_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.b_gain = 256;

	//alsc settings
	isp_gen->alsc_settings.correct_freqs = 6;
	isp_gen->alsc_settings.correct_start = isp_gen->alsc_settings.correct_freqs * 5;
	isp_gen->alsc_settings.gain_pool_size = 16;
	isp_gen->alsc_settings.table_switch = 2;
	isp_gen->alsc_settings.pre_lowBright = 35;
	isp_gen->alsc_settings.pre_complex = 141;
	isp_gen->alsc_settings.pre_highSatu = 161;
	isp_gen->alsc_settings.rgb_semiflat = 6;
	isp_gen->alsc_settings.rch_semiflat = 0;

	//sensor temp
	isp_gen->temp_info.enable = 1;  //default is enable

	//sensor flip
	isp_gen->flip_info.enable = 1;
	isp_gen->flip_info.hflip = 0;
	isp_gen->flip_info.vflip = 0;
	//sensor_mipi_switch
	isp_gen->switch_info.mipi_switch_enable = 0;
	isp_gen->switch_info.bgain_comp = 10000;
	isp_gen->switch_info.rgain_comp = 10000;
	isp_gen->switch_info.bgain_comp_last = 10000;
	isp_gen->switch_info.rgain_comp_last = 10000;
	isp_gen->switch_info.sensorA_awb_stats = NULL;
	isp_gen->switch_info.sensorB_awb_stats = NULL;

	//wdr setting
	isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.tmp = 16<<10;
	isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.real = isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.tmp;
	isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.isp_hardware = isp_gen->ae_entity_ctx.ae_result.ae_wdr_ratio.tmp;

	// 1 -> gamma 2.2
	// 2 -> gamma 1.69
	// 3 -> gamma 1.3
	//isp_gen->ai_isp_en = 0;
}

#define ISP_CTX_MODULE_EN(en_bit, ISP_FEATURES) \
{ \
	if (en_bit) {\
		mod_cfg->module_enable_flag |= ISP_FEATURES;\
	} else {\
		mod_cfg->module_enable_flag &= ~ISP_FEATURES;\
	}\
}

int __isp_ctx_apply_enable(struct isp_lib_context *isp_gen)
{
	struct isp_module_config *mod_cfg = &isp_gen->module_cfg;
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;


	if (isp_gen->sensor_info.wdr_mode == 0)
		param->isp_test_settings.wdr_stitch_en = 0;

	ISP_CTX_MODULE_EN(param->isp_test_settings.sharp_en       , ISP_FEATURES_SHARP);
	ISP_CTX_MODULE_EN(param->isp_test_settings.denoise_en     , ISP_FEATURES_D2D);
	ISP_CTX_MODULE_EN(param->isp_test_settings.drc_en         , ISP_FEATURES_RGB_DRC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.lsc_en         , ISP_FEATURES_LSC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.msc_en         , ISP_FEATURES_MSC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.gamma_en       , ISP_FEATURES_GAMMA);
	ISP_CTX_MODULE_EN(param->isp_test_settings.ccm_en         , ISP_FEATURES_RGB2RGB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.blc_en         , ISP_FEATURES_BLC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.wb_en          , ISP_FEATURES_WB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.otf_dpc_en     , ISP_FEATURES_DPC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cfa_en         , ISP_FEATURES_CFA);
	ISP_CTX_MODULE_EN(param->isp_test_settings.tdf_en         , ISP_FEATURES_D3D);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cnr_en         , ISP_FEATURES_CNR);
	ISP_CTX_MODULE_EN(param->isp_test_settings.lca_en         , ISP_FEATURES_LCA);
	ISP_CTX_MODULE_EN(param->isp_test_settings.gca_en         , ISP_FEATURES_GCA);
 	ISP_CTX_MODULE_EN(param->isp_test_settings.dig_gain_en    , ISP_FEATURES_DG);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cem_en         , ISP_FEATURES_CEM);
	ISP_CTX_MODULE_EN(param->isp_test_settings.pltm_en        , ISP_FEATURES_PLTM);
	ISP_CTX_MODULE_EN(param->isp_test_settings.wdr_stitch_en  , ISP_FEATURES_WDR_STITCH);
	ISP_CTX_MODULE_EN(param->isp_test_settings.wdr_split_en   , ISP_FEATURES_WDR_SPLIT);
 	ISP_CTX_MODULE_EN(param->isp_test_settings.ctc_en         , ISP_FEATURES_CTC);

	// Colorspace open in hardware as default.
	ISP_CTX_MODULE_EN(1, ISP_FEATURES_RGB2YUV);
	ISP_CTX_MODULE_EN(1, ISP_FEATURES_WDR);
	ISP_CTX_MODULE_EN(1, ISP_FEATURES_NRP);
	ISP_CTX_MODULE_EN(1, ISP_FEATURES_NR);
	ISP_CTX_MODULE_EN(1, ISP_FEATURES_VIN);

	ISP_CTX_MODULE_EN(param->isp_test_settings.afs_en        , ISP_FEATURES_AFS);
	ISP_CTX_MODULE_EN(param->isp_test_settings.ae_en         , ISP_FEATURES_AE);
	ISP_CTX_MODULE_EN(param->isp_test_settings.awb_en        , ISP_FEATURES_AWB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.af_en         , ISP_FEATURES_AF);
	ISP_CTX_MODULE_EN(param->isp_test_settings.hist_en       , ISP_FEATURES_HIST);

	mod_cfg->module_enable_flag |= ISP_FEATURES_MODE;

	isp_lib_log_param = param->isp_test_settings.isp_log_param;

	return 0;
}
#if 0
static void __build_sharp_anti_tbl(HW_U16 *gm_in, HW_U16 *inv_gm_out)
{
	int i, j, max, min;
	int tmp = 0;

	//anti gamma
	inv_gm_out[0] = 0;
	for(i = 1; i < 64; i++) {
		for(j = tmp; j < 64; j++) {
			min = gm_in[j];
			max = gm_in[j+1];
			if((i*64) >= min && (i*64) <= max) {
				inv_gm_out[i] = (j*64 + 64*(i*64-min)/(max-min));
				tmp = j;
				break;
			}
			inv_gm_out[i] = 4095;
		}
	}
	inv_gm_out[64] = 4095;
}
#endif
static void __isp_ctx_cfg_mod(struct isp_lib_context *isp_gen)
{
	//after __isp_ctx_cfg_lib
	struct isp_module_config *mod_cfg = &isp_gen->module_cfg;
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;
	int i = 0, j = 0;
	//HW_U8 isp_tdnf_k_mg_bk[ISP_REG1_TBL_LENGTH] = {0, 8, 16, 24, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128};
	//HW_U8 isp_tdnf_r_mg_bk[ISP_REG1_TBL_LENGTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 5, 9, 19, 37, 71, 255};
	HW_U8 isp_tdnf_df_bd_tbl[ISP_TDNF_TBL_SIZE] = {
		255,   255,   255,   255,   255,   255,   255,   255,
		255,   255,   255,   255,   255,   255,   255,   255,
		255,   254,   254,   254,   254,   254,   254,   253,
		253,   253,   253,   253,   252,   252,   252,   252,
		252,   251,   251,   251,   250,   250,   250,   250,
		249,   249,   249,   248,   248,   248,   247,   247,
		247,   246,   246,   245,   245,   245,   244,   244,
		243,   243,   242,   242,   241,   241,   240,   240,
		240,   239,   238,   238,   237,   237,   236,   236,
		235,   235,   234,   234,   233,   232,   232,   231,
		231,   230,   229,   229,   228,   227,   227,   226,
		225,   225,   224,   223,   222,   222,   221,   220,
		220,   219,   218,   217,   216,   216,   215,   214,
		213,   212,   212,   211,   210,   209,   208,   207,
		207,   206,   205,   204,   203,   202,   201,   200,
		199,   198,   197,   196,   195,   194,   193,   192,
		192,   190,   189,   188,   187,   186,   185,   184,
		183,   182,   181,   180,   179,   178,   177,   176,
		175,   173,   172,   171,   170,   169,   168,   166,
		165,   164,   163,   162,   160,   159,   158,   157,
		156,   154,   153,   152,   150,   149,   148,   147,
		145,   144,   143,   141,   140,   139,   137,   136,
		135,   133,   132,   130,   129,   128,   126,   125,
		123,   122,   120,   119,   117,   116,   114,   113,
		112,   110,   108,   107,   105,   104,   102,   101,
		99,    98,    96,    95,    93,    91,    90,    88,
		87,    85,    83,    82,    80,    78,    77,    75,
		73,    72,    70,    68,    66,    65,    63,    61,
		60,    58,    56,    54,    52,    51,    49,    47,
		45,    43,    42,    40,    38,    36,    34,    32,
		31,    29,    27,    25,    23,    21,    19,    17,
		15,    13,    11,     9,     7,     5,     3,     1
	};

	//WDR
	if (isp_gen->sensor_info.wdr_mode == ISP_2FCMD_WDR_MODE || isp_gen->sensor_info.wdr_mode == ISP_3FCMD_WDR_MODE) { /* one ch data input */
		mod_cfg->mode_cfg.wdr_mode = WDR_1FCH;
		isp_gen->ae_settings.ae_mode = AE_NORM;
		mod_cfg->mode_cfg.dg_mode = DG_AFTER_WDR;
	} else if (isp_gen->sensor_info.wdr_mode == ISP_2FDOL_WDR_MODE || isp_gen->sensor_info.wdr_mode == ISP_2FCMD1FDOL_WDR_MODE
			|| isp_gen->sensor_info.wdr_mode == ISP_1FDOL2FCMD_WDR_MODE) { /* two ch data input */
		mod_cfg->mode_cfg.wdr_mode = WDR_2FCH;
		isp_gen->ae_settings.ae_mode = clamp(param->isp_3a_settings.wdr_comm_cfg[WDR_COMM_MODE], AE_WDR, AE_WDR_AUTO_DG);
		if (isp_gen->ae_settings.ae_mode == AE_WDR_DG || isp_gen->ae_settings.ae_mode == AE_WDR_AUTO_DG) {
			mod_cfg->mode_cfg.dg_mode = DG_BEFORE_WDR;
		} else {
			mod_cfg->mode_cfg.dg_mode = DG_AFTER_WDR;
		}
	} else if (isp_gen->sensor_info.wdr_mode == ISP_3FDOL_WDR_MODE) { /* three ch data input */
		mod_cfg->mode_cfg.wdr_mode = WDR_3FCH;
		isp_gen->ae_settings.ae_mode = AE_WDR;
		if (isp_gen->ae_settings.ae_mode == AE_WDR_DG || isp_gen->ae_settings.ae_mode == AE_WDR_AUTO_DG) {
			mod_cfg->mode_cfg.dg_mode = DG_BEFORE_WDR;
		} else {
			mod_cfg->mode_cfg.dg_mode = DG_AFTER_WDR;
		}
	} else {
		mod_cfg->mode_cfg.wdr_mode = WDR_1FCH;
		isp_gen->ae_settings.ae_mode = AE_NORM;
		mod_cfg->mode_cfg.dg_mode = DG_AFTER_WDR;
	}

	config_wdr_split(isp_gen);
	config_fe_channel(isp_gen);
	config_wdr(isp_gen, 1);

	isp_gen->databit.st_pltm = 12;/*10/12*/
	isp_gen->databit.st_cnr = isp_gen->databit.st_pltm;
	isp_gen->databit.st_drc = isp_gen->databit.st_pltm;
	isp_gen->databit.st_csc = 8;

	//awb
	if (param->isp_test_settings.wb_en == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.r_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.gr_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.gb_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.b_gain == 0) {
		mod_cfg->wb_gain_cfg.wb_gain.r_gain  = 468;
		mod_cfg->wb_gain_cfg.wb_gain.gr_gain = 256;
		mod_cfg->wb_gain_cfg.wb_gain.gb_gain = 256;
		mod_cfg->wb_gain_cfg.wb_gain.b_gain  = 482;
	}

	mod_cfg->awb_cfg.awb_r_sat_lim = AWB_SAT_DEF_LIM;
	mod_cfg->awb_cfg.awb_g_sat_lim = AWB_SAT_DEF_LIM;
	mod_cfg->awb_cfg.awb_b_sat_lim = AWB_SAT_DEF_LIM;
	isp_gen->awb_settings.wb_stat_combine_mode = WB_STAT_NONE;

	//af
	mod_cfg->af_cfg.af_sap_lim = ISP_AF_DIR_TH;

	//ae
	isp_gen->ae_entity_ctx.ae_param->ae_save_delay_en = 1;
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.wdr_cfg[0], &param->isp_3a_settings.wdr_comm_cfg[0], ISP_WDR_COMM_CFG_MAX*sizeof(HW_U16));

	mod_cfg->mode_cfg.input_fmt = isp_gen->sensor_info.input_seq;
	mod_cfg->mode_cfg.rgb_cfg_bit = clamp(isp_gen->databit.st_pltm, 10, 12);
	mod_cfg->mode_cfg.hist0_sel = param->isp_3a_settings.ae_hist0_sel;
	mod_cfg->mode_cfg.hist1_sel = param->isp_3a_settings.ae_hist1_sel;
	mod_cfg->mode_cfg.ae_sel = param->isp_3a_settings.ae_stat_sel;
	mod_cfg->mode_cfg.awb_sel = param->isp_3a_settings.awb_stat_sel;

	//shift bit
	if (mod_cfg->mode_cfg.ae_sel == 0) {
		mod_cfg->ae_cfg.ae_reg_win.shift_bit0 = isp_gen->databit.st_femsc[0] - 12;
		mod_cfg->ae_cfg.ae_reg_win.shift_bit2 = isp_gen->databit.st_femsc[1] - 12;
		if (mod_cfg->mode_cfg.wdr_mode == WDR_2FCH) {
			if (isp_gen->ae_settings.ae_mode < AE_WDR_AUTO)
				mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH; //stat from long exp frame
			else
				mod_cfg->mode_cfg.ae_mode = AE_STAT_2CH;
		} else if (mod_cfg->mode_cfg.wdr_mode == WDR_3FCH) {
			if (isp_gen->ae_settings.ae_mode < AE_WDR_AUTO)
				mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH; //stat from long exp frame
			else
				mod_cfg->mode_cfg.ae_mode = AE_STAT_3CH;
		} else
			mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH;
	} else if (mod_cfg->mode_cfg.ae_sel == 1) {
		mod_cfg->ae_cfg.ae_reg_win.shift_bit0 = isp_gen->databit.st_msc - 12;
		mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH;
	} else if (mod_cfg->mode_cfg.ae_sel == 2) {
		mod_cfg->ae_cfg.ae_reg_win.shift_bit0 = 12 - isp_gen->databit.st_cnr;
		mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH;
	} else if (mod_cfg->mode_cfg.ae_sel == 3) {
		mod_cfg->ae_cfg.ae_reg_win.shift_bit0 = 12 - isp_gen->databit.st_drc;
		mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH;
	} else {
		mod_cfg->ae_cfg.ae_reg_win.shift_bit0 = 12 - isp_gen->databit.st_csc;
		mod_cfg->mode_cfg.ae_mode = AE_STAT_1CH;
	}
	isp_gen->ae_settings.ae_stat_mode = mod_cfg->mode_cfg.ae_mode;

	if (mod_cfg->mode_cfg.awb_sel == 0)
		mod_cfg->awb_cfg.awb_reg_win.shift_bit0 = isp_gen->databit.st_msc - 12;
	else
		mod_cfg->awb_cfg.awb_reg_win.shift_bit0 = 12 - isp_gen->databit.st_pltm;

	if (mod_cfg->mode_cfg.hist0_sel == 0)
		mod_cfg->hist_cfg.hist0_reg_win.shift_bit0 = isp_gen->databit.st_msc - 8;
	else if (mod_cfg->mode_cfg.hist0_sel == 1)
		mod_cfg->hist_cfg.hist0_reg_win.shift_bit0 = isp_gen->databit.st_cnr - 8;
	else if (mod_cfg->mode_cfg.hist0_sel == 2)
		mod_cfg->hist_cfg.hist0_reg_win.shift_bit0 = isp_gen->databit.st_drc - 8;
	else
		mod_cfg->hist_cfg.hist0_reg_win.shift_bit0 = isp_gen->databit.st_csc - 8;

	if (mod_cfg->mode_cfg.hist1_sel == 0)
		mod_cfg->hist_cfg.hist1_reg_win.shift_bit0 = isp_gen->databit.st_msc - 8;
	else if (mod_cfg->mode_cfg.hist1_sel == 1)
		mod_cfg->hist_cfg.hist1_reg_win.shift_bit0 = isp_gen->databit.st_cnr - 8;
	else if (mod_cfg->mode_cfg.hist1_sel == 2)
		mod_cfg->hist_cfg.hist1_reg_win.shift_bit0 = isp_gen->databit.st_drc - 8;
	else
		mod_cfg->hist_cfg.hist1_reg_win.shift_bit0 = isp_gen->databit.st_csc - 8;

	mod_cfg->output_speed = 1;

	config_dig_gain(isp_gen, 1024, 1024);

	//dpc
	mod_cfg->otf_cfg.remove_mode = param->isp_tunning_settings.dpc_remove_mode;
	mod_cfg->otf_cfg.sup_twinkle_en = param->isp_tunning_settings.dpc_sup_twinkle_en;

	//nrp
	mod_cfg->nrp_cfg.gm_neg_ratio = 0;
	if (param->isp_test_settings.nrp_en) {
		if (isp_gen->sensor_info.wdr_mode) {
			//ISP_WARN("close nrp when input data is wdr\n");
			mod_cfg->mode_cfg.gain_sel = 1;
			mod_cfg->mode_cfg.wb_sel = 1;
			mod_cfg->nrp_cfg.gamma_en = 0;
			mod_cfg->nrp_cfg.inv_gamma_en = 0;
			mod_cfg->nrp_cfg.blc_en = 0;
			mod_cfg->nrp_cfg.inv_blc_en = 0;
		} else {
			mod_cfg->mode_cfg.gain_sel = 0;
			mod_cfg->mode_cfg.wb_sel = 0;
			mod_cfg->nrp_cfg.gamma_en = 1;
			mod_cfg->nrp_cfg.inv_gamma_en = 0;
			if (((!mod_cfg->mode_cfg.gain_sel && (param->isp_test_settings.dig_gain_en || param->isp_test_settings.lsc_en || param->isp_test_settings.msc_en)) ||
				(!mod_cfg->mode_cfg.wb_sel && param->isp_test_settings.wb_en) || mod_cfg->nrp_cfg.gamma_en) && param->isp_test_settings.blc_en) {
				mod_cfg->nrp_cfg.blc_en = 1;
				mod_cfg->nrp_cfg.inv_blc_en = 1;
			} else {
				mod_cfg->nrp_cfg.blc_en = 0;
				mod_cfg->nrp_cfg.inv_blc_en = 0;
			}
		}
	} else {
		mod_cfg->mode_cfg.gain_sel = 1;
		mod_cfg->mode_cfg.wb_sel = 1;
		mod_cfg->nrp_cfg.gamma_en = 0;
		mod_cfg->nrp_cfg.inv_gamma_en = 0;
		mod_cfg->nrp_cfg.blc_en = 0;
		mod_cfg->nrp_cfg.inv_blc_en = 0;
	}

	if (isp_gen->ai_isp_en) {
		mod_cfg->mode_cfg.gain_sel = 0;
		mod_cfg->mode_cfg.wb_sel = 0;
		mod_cfg->nrp_cfg.gamma_en = 0;
		mod_cfg->nrp_cfg.inv_gamma_en = 0;
		if (param->isp_test_settings.blc_en) {
			mod_cfg->nrp_cfg.blc_en = 1;
			mod_cfg->nrp_cfg.inv_blc_en = 1;
		} else {
			mod_cfg->nrp_cfg.blc_en = 0;
			mod_cfg->nrp_cfg.inv_blc_en = 0;
		}
	}

	//nr
	mod_cfg->nr_cfg.d2d_msc_cmp_en = isp_gen->isp_ini_cfg.isp_test_settings.msc_en;
	mod_cfg->nr_cfg.d3d_msc_cmp_en = isp_gen->isp_ini_cfg.isp_test_settings.msc_en;
	if (isp_gen->sensor_info.width_overlayer)
		mod_cfg->nr_cfg.msc_blw = isp_gen->sensor_info.width_overlayer / 22;
	else
		mod_cfg->nr_cfg.msc_blw = isp_gen->sensor_info.sensor_width / 22;
	mod_cfg->nr_cfg.msc_blh = isp_gen->sensor_info.sensor_height / 22;
	mod_cfg->nr_cfg.msc_blw_dlt = 4096 / mod_cfg->nr_cfg.msc_blw;
	mod_cfg->nr_cfg.msc_blh_dlt = 4096 / mod_cfg->nr_cfg.msc_blh;

	//denoise
	mod_cfg->bdnf_cfg.lyr_core_ratio[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR0_CORE_RATIO];
	mod_cfg->bdnf_cfg.lyr_core_ratio[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR1_CORE_RATIO];
	mod_cfg->bdnf_cfg.lyr_core_ratio[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR2_CORE_RATIO];
	mod_cfg->bdnf_cfg.lyr_core_ratio[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR3_CORE_RATIO];
	mod_cfg->bdnf_cfg.lyr_side_ratio[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR0_SIDE_RATIO];
	mod_cfg->bdnf_cfg.lyr_side_ratio[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR1_SIDE_RATIO];
	mod_cfg->bdnf_cfg.lyr_side_ratio[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR2_SIDE_RATIO];
	mod_cfg->bdnf_cfg.lyr_side_ratio[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR3_SIDE_RATIO];
	mod_cfg->bdnf_cfg.lyr_dnr_cratio[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR0_DNR_CRATIO];
	mod_cfg->bdnf_cfg.lyr_dnr_cratio[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR1_DNR_CRATIO];
	mod_cfg->bdnf_cfg.lyr_dnr_cratio[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR2_DNR_CRATIO];
	mod_cfg->bdnf_cfg.lyr_dnr_cratio[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR3_DNR_CRATIO];
	mod_cfg->bdnf_cfg.lyr_gauss_type[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR0_GAUSS_TYPE];
	mod_cfg->bdnf_cfg.lyr_gauss_type[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR1_GAUSS_TYPE];
	mod_cfg->bdnf_cfg.lyr_gauss_type[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR2_GAUSS_TYPE];
	mod_cfg->bdnf_cfg.lyr_gauss_type[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR3_GAUSS_TYPE];
	mod_cfg->bdnf_cfg.lyr_gauss_wght[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR0_GAUSS_WGTH];
	mod_cfg->bdnf_cfg.lyr_gauss_wght[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR1_GAUSS_WGTH];
	mod_cfg->bdnf_cfg.lyr_gauss_wght[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR2_GAUSS_WGTH];
	mod_cfg->bdnf_cfg.lyr_gauss_wght[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_LYR3_GAUSS_WGTH];
	mod_cfg->bdnf_cfg.cnt_ratio[0] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_CNT_RATIO0];
	mod_cfg->bdnf_cfg.cnt_ratio[1] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_CNT_RATIO1];
	mod_cfg->bdnf_cfg.cnt_ratio[2] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_CNT_RATIO2];
	mod_cfg->bdnf_cfg.cnt_ratio[3] = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_CNT_RATIO3];
	mod_cfg->bdnf_cfg.wdr_lm_hi_slp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_LM_HI_SLP];
	mod_cfg->bdnf_cfg.wdr_ms_hi_slp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_MS_HI_SLP];
	mod_cfg->bdnf_cfg.wdr_lm_lw_slp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_LM_LW_SLP];
	mod_cfg->bdnf_cfg.wdr_ms_lw_slp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_MS_LW_SLP];
	mod_cfg->bdnf_cfg.wdr_lm_max_clp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_LM_MAX_CLP];
	mod_cfg->bdnf_cfg.wdr_ms_max_clp = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_WDR_MS_MAX_CLP];
	mod_cfg->nr_cfg.d2d_msc_cmp_ratio = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_MSC_CMP_RATIO];
	mod_cfg->bdnf_cfg.dtc_mg_mode = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_DTC_MG_MODE];
	mod_cfg->bdnf_cfg.dtc_mg_clip = param->isp_tunning_settings.denoise_comm_cfg[ISP_DENOISE_DTC_MG_CLIP];

	//tdf
	if (param->isp_test_settings.tdf_en)
		mod_cfg->tdf_cfg.bypass_mode = 0;
	else
		mod_cfg->tdf_cfg.bypass_mode = 1;
	mod_cfg->tdf_cfg.flt_out_sel = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_FILT_OUT_SEL];
	if (isp_gen->sensor_info.width_overlayer)
		mod_cfg->tdf_cfg.kb_wnum = isp_gen->sensor_info.width_overlayer / 9;
	else
		mod_cfg->tdf_cfg.kb_wnum = isp_gen->sensor_info.sensor_width / 9;
	mod_cfg->tdf_cfg.kb_hnum = isp_gen->sensor_info.sensor_height / 9;
	if (isp_gen->sensor_info.width_overlayer)
		mod_cfg->tdf_cfg.kst_wlen = isp_gen->sensor_info.width_overlayer / 32;
	else
		mod_cfg->tdf_cfg.kst_wlen = isp_gen->sensor_info.sensor_width / 32;
	mod_cfg->tdf_cfg.kst_hlen = isp_gen->sensor_info.sensor_height / 24;
	mod_cfg->tdf_cfg.kst_pnum = mod_cfg->tdf_cfg.kst_wlen * mod_cfg->tdf_cfg.kst_hlen;
	mod_cfg->tdf_cfg.kst_wphs = ((mod_cfg->tdf_cfg.kst_wlen >> 1) << 12) / mod_cfg->tdf_cfg.kst_wlen;
	mod_cfg->tdf_cfg.kst_wstp = (1 << 12) / mod_cfg->tdf_cfg.kst_wlen;
	mod_cfg->tdf_cfg.kst_hphs = ((mod_cfg->tdf_cfg.kst_hlen >> 1) << 12) / mod_cfg->tdf_cfg.kst_hlen;
	mod_cfg->tdf_cfg.kst_hstp = (1 << 12) / mod_cfg->tdf_cfg.kst_hlen;
	mod_cfg->tdf_cfg.diff_intra_amp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_DIFF_INTRA_AMP];
	mod_cfg->tdf_cfg.diff_inter_amp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_DIFF_INTER_AMP];
	mod_cfg->tdf_cfg.thr_intra_amp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_THR_INTRA_AMP];
	mod_cfg->tdf_cfg.thr_inter_amp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_THR_INTER_AMP];
	mod_cfg->tdf_cfg.diff_min_clp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_DIFF_MIN_CLP];
	mod_cfg->tdf_cfg.wdr_lm_hi_slp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_LM_HI_SLP];
	mod_cfg->tdf_cfg.wdr_ms_hi_slp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_MS_HI_SLP];
	mod_cfg->tdf_cfg.wdr_lm_lw_slp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_LM_LW_SLP];
	mod_cfg->tdf_cfg.wdr_ms_lw_slp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_MS_LW_SLP];
	mod_cfg->tdf_cfg.wdr_lm_max_clp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_LM_MAX_CLP];
	mod_cfg->tdf_cfg.wdr_ms_max_clp = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_WDR_MS_MAX_CLP];
	mod_cfg->nr_cfg.d3d_msc_cmp_ratio = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MSC_CMP_RATIO];
	mod_cfg->tdf_cfg.stl_stg_cth[0] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_0];
	mod_cfg->tdf_cfg.stl_stg_cth[1] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_1];
	mod_cfg->tdf_cfg.stl_stg_cth[2] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_2];
	mod_cfg->tdf_cfg.stl_stg_cth[3] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_3];
	mod_cfg->tdf_cfg.stl_stg_cth[4] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_4];
	mod_cfg->tdf_cfg.stl_stg_cth[5] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_5];
	mod_cfg->tdf_cfg.stl_stg_cth[6] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_6];
	mod_cfg->tdf_cfg.stl_stg_cth[7] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_CTH_7];
	mod_cfg->tdf_cfg.stl_stg_kth[0] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_0];
	mod_cfg->tdf_cfg.stl_stg_kth[1] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_1];
	mod_cfg->tdf_cfg.stl_stg_kth[2] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_2];
	mod_cfg->tdf_cfg.stl_stg_kth[3] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_3];
	mod_cfg->tdf_cfg.stl_stg_kth[4] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_4];
	mod_cfg->tdf_cfg.stl_stg_kth[5] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_5];
	mod_cfg->tdf_cfg.stl_stg_kth[6] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_6];
	mod_cfg->tdf_cfg.stl_stg_kth[7] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_STL_STG_KTH_7];
	mod_cfg->tdf_cfg.mot_stg_cth[0] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_0];
	mod_cfg->tdf_cfg.mot_stg_cth[1] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_1];
	mod_cfg->tdf_cfg.mot_stg_cth[2] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_2];
	mod_cfg->tdf_cfg.mot_stg_cth[3] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_3];
	mod_cfg->tdf_cfg.mot_stg_cth[4] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_4];
	mod_cfg->tdf_cfg.mot_stg_cth[5] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_5];
	mod_cfg->tdf_cfg.mot_stg_cth[6] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_6];
	mod_cfg->tdf_cfg.mot_stg_cth[7] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_CTH_7];
	mod_cfg->tdf_cfg.mot_stg_kth[0] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_0];
	mod_cfg->tdf_cfg.mot_stg_kth[1] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_1];
	mod_cfg->tdf_cfg.mot_stg_kth[2] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_2];
	mod_cfg->tdf_cfg.mot_stg_kth[3] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_3];
	mod_cfg->tdf_cfg.mot_stg_kth[4] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_4];
	mod_cfg->tdf_cfg.mot_stg_kth[5] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_5];
	mod_cfg->tdf_cfg.mot_stg_kth[6] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_6];
	mod_cfg->tdf_cfg.mot_stg_kth[7] = param->isp_tunning_settings.tdf_comm_cfg[ISP_TDF_MOT_STG_KTH_7];
	HW_U8 *tdnf_table = (HW_U8 *)mod_cfg->tdnf_table;
	for (i = 0; i < ISP_TDNF_TBL_SIZE; i++) {
		tdnf_table[8*i + 0] = isp_tdnf_df_bd_tbl[i];
		//tdnf_table[8*i + 1] = isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_kdelta[i];
	}
	memcpy(&mod_cfg->tdf_cfg.df_shp_bk[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_df_shape[0], ISP_REG1_TBL_LENGTH);
	memcpy(&mod_cfg->tdf_cfg.r_amp_bk[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_ratio_amp[0], ISP_REG1_TBL_LENGTH);
	memcpy(&mod_cfg->tdf_cfg.k_dlt_bk[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_k_dlt_bk[0], ISP_REG1_TBL_LENGTH);
	memcpy(&mod_cfg->tdf_cfg.ct_rt_bk[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_ct_rt_bk[0], ISP_REG1_TBL_LENGTH);
	memcpy(&mod_cfg->tdf_cfg.lay0_d2d0_rt_br[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_lay0_d2d0_rt_br[0], ISP_REG1_TBL_LENGTH);
	memcpy(&mod_cfg->tdf_cfg.lay1_d2d0_rt_br[0], &isp_gen->isp_ini_cfg.isp_tunning_settings.isp_tdnf_lay1_d2d0_rt_br[0], ISP_REG1_TBL_LENGTH);
	//memcpy(&mod_cfg->tdf_cfg.k_mg_bk[0], &isp_tdnf_k_mg_bk[0], ISP_REG1_TBL_LENGTH);
	//memcpy(&mod_cfg->tdf_cfg.r_mg_bk[0], &isp_tdnf_r_mg_bk[0], ISP_REG1_TBL_LENGTH);

	//CFA
	mod_cfg->mode_cfg.dmsc_mode = DMSC_NORMAL;
	mod_cfg->cfa_cfg.cfa_grad_th = param->isp_tunning_settings.grad_th;
	mod_cfg->cfa_cfg.cfa_dir_v_th = param->isp_tunning_settings.dir_v_th;
	mod_cfg->cfa_cfg.cfa_dir_h_th = param->isp_tunning_settings.dir_h_th;
	mod_cfg->cfa_cfg.res_smth_high = param->isp_tunning_settings.res_smth_high;
	mod_cfg->cfa_cfg.res_smth_low = param->isp_tunning_settings.res_smth_low;
	//mod_cfg->cfa_cfg.afc_cnr_stren = 0x0;
	//mod_cfg->cfa_cfg.afc_rgb_stren = 0x0;
	//mod_cfg->cfa_cfg.afc_bayer_stren = 0x0;
	mod_cfg->cfa_cfg.res_high_th = param->isp_tunning_settings.res_high_th;
	mod_cfg->cfa_cfg.res_low_th = param->isp_tunning_settings.res_low_th;
	mod_cfg->cfa_cfg.res_dir_a = param->isp_tunning_settings.res_dir_a;
	mod_cfg->cfa_cfg.res_dir_d = param->isp_tunning_settings.res_dir_d;
	mod_cfg->cfa_cfg.res_dir_v = param->isp_tunning_settings.res_dir_v;
	mod_cfg->cfa_cfg.res_dir_h = param->isp_tunning_settings.res_dir_h;
	if (param->isp_test_settings.defog_en == 1) {
		//mod_cfg->cfa_cfg.min_rgb    = 1023;
	} else {
		//mod_cfg->cfa_cfg.min_rgb    = 0;
	}

	//ccm
	mod_cfg->rgb2rgb_cfg.color_matrix = param->isp_tunning_settings.color_matrix_ini[0];

	//ctc todo
	mod_cfg->ctc_cfg.ctc_th_max = param->isp_tunning_settings.ctc_th_max;//316;
	mod_cfg->ctc_cfg.ctc_th_min = param->isp_tunning_settings.ctc_th_min;//60;
	mod_cfg->ctc_cfg.ctc_th_slope = param->isp_tunning_settings.ctc_th_slope;//262;
	mod_cfg->ctc_cfg.ctc_dir_wt = param->isp_tunning_settings.ctc_dir_wt;//64;
	mod_cfg->ctc_cfg.ctc_dir_th = param->isp_tunning_settings.ctc_dir_th;//80;

	//pltm
	if (isp_gen->module_cfg.wdr_cfg.wdr_stitch_mode == ISP_WDR_STITCH_2F_LM)
		mod_cfg->pltm_cfg.max_stage = 3 - 1; /*2f-wdr*/
	else if (isp_gen->module_cfg.wdr_cfg.wdr_stitch_mode == ISP_WDR_STITCH_3F_LMS)
		mod_cfg->pltm_cfg.max_stage = 4 - 1; /*3f-wdr*/
	else
#if (ISP_VERSION == 603)
		mod_cfg->pltm_cfg.max_stage = 1 - 1;/*linear*/
#else
		mod_cfg->pltm_cfg.max_stage = 3 - 1;/*linear*/
#endif

	mod_cfg->pltm_cfg.strength = 256;
	if (isp_gen->sensor_info.width_overlayer)
		mod_cfg->pltm_cfg.blk_width = isp_gen->sensor_info.width_overlayer / 32;
	else
		mod_cfg->pltm_cfg.blk_width = isp_gen->sensor_info.sensor_width / 32;
	mod_cfg->pltm_cfg.blk_height = isp_gen->sensor_info.sensor_height / 24;
	mod_cfg->pltm_cfg.blk_num = mod_cfg->pltm_cfg.blk_width * mod_cfg->pltm_cfg.blk_height;
	mod_cfg->pltm_cfg.wi_phase = ((mod_cfg->pltm_cfg.blk_width >> 1) << 12) / mod_cfg->pltm_cfg.blk_width;
	mod_cfg->pltm_cfg.wi_step = (1 << 12) / mod_cfg->pltm_cfg.blk_width;
	mod_cfg->pltm_cfg.hi_phase = ((mod_cfg->pltm_cfg.blk_height >> 1) << 12) / mod_cfg->pltm_cfg.blk_height;
	mod_cfg->pltm_cfg.hi_step = (1 << 12) / mod_cfg->pltm_cfg.blk_height;
	mod_cfg->pltm_cfg.nrm_ratio = 2048;
	mod_cfg->pltm_cfg.gtm_en = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_GTM_EN];
	mod_cfg->pltm_cfg.lft_en = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_LTF_EN];
	mod_cfg->pltm_cfg.dsc_en = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DSC_EN];
	mod_cfg->pltm_cfg.dcc_en = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DCC_EN];
	mod_cfg->pltm_cfg.dsc_shf_bit = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DSC_SHF_BIT];
	mod_cfg->pltm_cfg.dsc_lw_th = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DSC_LW_TH];
	mod_cfg->pltm_cfg.dsc_hi_th = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DSC_HI_TH];
	mod_cfg->pltm_cfg.dsc_lw_ratio = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DSC_LW_RT];
	mod_cfg->pltm_cfg.dcc_shf_bit = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DCC_SHF_BIT];
	mod_cfg->pltm_cfg.dcc_lw_th = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DCC_LW_TH];
	mod_cfg->pltm_cfg.dcc_hi_th = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_DCC_HI_TH];
	mod_cfg->pltm_cfg.mgc_int_smth[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_INT_SMTH0];
	mod_cfg->pltm_cfg.mgc_int_smth[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_INT_SMTH1];
	mod_cfg->pltm_cfg.mgc_int_smth[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_INT_SMTH2];
	mod_cfg->pltm_cfg.mgc_int_smth[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_INT_SMTH3];
	mod_cfg->pltm_cfg.mgc_lum_adpt[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_LUM_ADPT0];
	mod_cfg->pltm_cfg.mgc_lum_adpt[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_LUM_ADPT1];
	mod_cfg->pltm_cfg.mgc_lum_adpt[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_LUM_ADPT2];
	mod_cfg->pltm_cfg.mgc_lum_adpt[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_MGC_LUM_ADPT3];
	mod_cfg->pltm_cfg.sts_ceil_slp[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_CEIL_SLP0];
	mod_cfg->pltm_cfg.sts_ceil_slp[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_CEIL_SLP1];
	mod_cfg->pltm_cfg.sts_ceil_slp[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_CEIL_SLP2];
	mod_cfg->pltm_cfg.sts_ceil_slp[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_CEIL_SLP3];
	mod_cfg->pltm_cfg.sts_floor_slp[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_FLOOR_SLP0];
	mod_cfg->pltm_cfg.sts_floor_slp[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_FLOOR_SLP1];
	mod_cfg->pltm_cfg.sts_floor_slp[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_FLOOR_SLP2];
	mod_cfg->pltm_cfg.sts_floor_slp[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_FLOOR_SLP3];
	mod_cfg->pltm_cfg.sts_gd_ratio[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_GD_RT0];
	mod_cfg->pltm_cfg.sts_gd_ratio[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_GD_RT1];
	mod_cfg->pltm_cfg.sts_gd_ratio[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_GD_RT2];
	mod_cfg->pltm_cfg.sts_gd_ratio[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_STS_GD_RT3];
	mod_cfg->pltm_cfg.adjk_crct_ratio[0] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_ADJK_CRCT_RT0];
	mod_cfg->pltm_cfg.adjk_crct_ratio[1] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_ADJK_CRCT_RT1];
	mod_cfg->pltm_cfg.adjk_crct_ratio[2] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_ADJK_CRCT_RT2];
	mod_cfg->pltm_cfg.adjk_crct_ratio[3] = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_ADJK_CRCT_RT3];
	for (i = 0; i < PLTM_MAX_STAGE; i++) {
		mod_cfg->pltm_cfg.sts_flt_stren[i][0] = 384;//384
		mod_cfg->pltm_cfg.sts_flt_stren[i][1] = 256;//256
		mod_cfg->pltm_cfg.sts_flt_stren[i][2] = 64;//128

		mod_cfg->pltm_cfg.adj1_asym_ratio[i] = 0;
	}
	if (isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_MODE] != 1) {
		for (i = 0; i < 128; i++) {
			mod_cfg->pltm_cfg.pltm_table[ISP_PLTM_TM_TBL_SIZE + 4*i + 0] = param->isp_tunning_settings.isp_pltm_lum_map_cv[0][i];
			mod_cfg->pltm_cfg.pltm_table[ISP_PLTM_TM_TBL_SIZE + 4*i + 1] = param->isp_tunning_settings.isp_pltm_lum_map_cv[1][i];
			mod_cfg->pltm_cfg.pltm_table[ISP_PLTM_TM_TBL_SIZE + 4*i + 2] = param->isp_tunning_settings.isp_pltm_lum_map_cv[2][i];
			mod_cfg->pltm_cfg.pltm_table[ISP_PLTM_TM_TBL_SIZE + 4*i + 3] = param->isp_tunning_settings.isp_pltm_lum_map_cv[3][i];
		}
	}
	for (i = 0; i < 15; i++) {
		for (j = 0; j < PLTM_MAX_STAGE; j++) {
			mod_cfg->pltm_cfg.stat_gd_cv[i*PLTM_MAX_STAGE+j] = param->isp_tunning_settings.isp_pltm_stat_gd_cv[j][i];
		}
	}
	for (i = 0; i < ISP_REG_TBL_LENGTH; i++) {
		for (j = 0; j < PLTM_MAX_STAGE; j++) {
			mod_cfg->pltm_cfg.adj_k_df_cv[i*PLTM_MAX_STAGE+j] = param->isp_tunning_settings.isp_pltm_df_cv[j][i];
		}
	}

	// gca
	//mod_cfg->gca_cfg.gca_ct_w = (isp_gen->stat.pic_size.width - 1) / 2;
	//mod_cfg->gca_cfg.gca_ct_h = (isp_gen->stat.pic_size.height - 1) / 2;
	mod_cfg->gca_cfg.gca_ct_w = param->isp_tunning_settings.gca_cfg[ISP_GCA_CT_W];
	mod_cfg->gca_cfg.gca_ct_h = param->isp_tunning_settings.gca_cfg[ISP_GCA_CT_H];

	mod_cfg->gca_cfg.gca_r_para0 = param->isp_tunning_settings.gca_cfg[ISP_GCA_R_PARA0];
	mod_cfg->gca_cfg.gca_r_para1 = param->isp_tunning_settings.gca_cfg[ISP_GCA_R_PARA1];
	mod_cfg->gca_cfg.gca_r_para2 = param->isp_tunning_settings.gca_cfg[ISP_GCA_R_PARA2];
	mod_cfg->gca_cfg.gca_b_para0 = param->isp_tunning_settings.gca_cfg[ISP_GCA_B_PARA0];
	mod_cfg->gca_cfg.gca_b_para1 = param->isp_tunning_settings.gca_cfg[ISP_GCA_B_PARA1];
	mod_cfg->gca_cfg.gca_b_para2 = param->isp_tunning_settings.gca_cfg[ISP_GCA_B_PARA2];
	mod_cfg->gca_cfg.gca_int_cns = param->isp_tunning_settings.gca_cfg[ISP_GCA_INT_CNS];

	// lca some param update in iso
	for(i = 0; i < 9; i++) {
		for(j = 0; j < 4; j++) {
			if((4*i+j) >= ISP_REG_TBL_LENGTH)
				break;
			mod_cfg->lca_cfg.lca_pf_satu_lut[4*i+j] = param->isp_tunning_settings.lca_pf_satu_lut[4*i+j];
		}
	}

	for(i = 0; i < 9; i++) {
		for(j = 0; j < 4; j++) {
			if((4*i+j) >= ISP_REG_TBL_LENGTH)
				break;
			mod_cfg->lca_cfg.lca_gf_satu_lut[4*i+j] = param->isp_tunning_settings.lca_gf_satu_lut[4*i+j];
		}
	}

	// lsc
	if (param->isp_test_settings.lsc_en) {
		mod_cfg->mode_cfg.rsc_mode = param->isp_tunning_settings.lsc_mode;
	}

	// msc
	if (param->isp_test_settings.msc_en) {
		mod_cfg->mode_cfg.msc_mode = param->isp_tunning_settings.msc_mode;
		for (i = 0; i < ISP_MSC_TBL_LUT_SIZE; i++) {
			mod_cfg->msc_cfg.msc_blw_lut[i] = clamp(param->isp_tunning_settings.msc_blw_lut[i], 2, 1023);
		}
		for (i = 0; i < ISP_MSC_TBL_LUT_SIZE; i++) {
			mod_cfg->msc_cfg.msc_blh_lut[i] = clamp(param->isp_tunning_settings.msc_blh_lut[i], 2, 1023);
		}
		if (param->isp_tunning_settings.msc_mode >= 4) {
			mod_cfg->msc_cfg.msc_blw_dlt_lut[0] = clamp(4096/max(mod_cfg->msc_cfg.msc_blw_lut[0], 1), 0, 4095);
			mod_cfg->msc_cfg.msc_blw_dlt_lut[ISP_MSC_TBL_LUT_DLT_SIZE -1] = clamp(4096/max(mod_cfg->msc_cfg.msc_blw_lut[ISP_MSC_TBL_LUT_SIZE-1], 1), 4, 2048);
			for (i = 1; i < ISP_MSC_TBL_LUT_DLT_SIZE - 1; i++) {
				mod_cfg->msc_cfg.msc_blw_dlt_lut[i] = clamp(8192/max(mod_cfg->msc_cfg.msc_blw_lut[i] + mod_cfg->msc_cfg.msc_blw_lut[i-1], 1), 4, 2048);
			}
			mod_cfg->msc_cfg.msc_blh_dlt_lut[0] = clamp(4096/max(mod_cfg->msc_cfg.msc_blh_lut[0], 1), 4, 2048);
			mod_cfg->msc_cfg.msc_blh_dlt_lut[ISP_MSC_TBL_LUT_DLT_SIZE -1] = clamp(4096/max(mod_cfg->msc_cfg.msc_blh_lut[ISP_MSC_TBL_LUT_SIZE-1], 1), 4, 2048);
			for(i = 1; i < ISP_MSC_TBL_LUT_DLT_SIZE - 1; i++) {
				mod_cfg->msc_cfg.msc_blh_dlt_lut[i] = clamp(8192/max(mod_cfg->msc_cfg.msc_blh_lut[i] + mod_cfg->msc_cfg.msc_blh_lut[i-1], 1), 4, 2048);
			}
		} else {
			mod_cfg->msc_cfg.msc_blw_dlt_lut[0] = clamp(4096/max(mod_cfg->msc_cfg.msc_blw_lut[0], 1), 4, 2048);
			mod_cfg->msc_cfg.msc_blw_dlt_lut[8] = clamp(4096/max(mod_cfg->msc_cfg.msc_blw_lut[7], 1), 4, 2048);
			for (i = 1; i < 8; i++) {
				mod_cfg->msc_cfg.msc_blw_dlt_lut[i] = clamp(8192/max(mod_cfg->msc_cfg.msc_blw_lut[i] + mod_cfg->msc_cfg.msc_blw_lut[i-1], 1), 4, 2048);
			}
			mod_cfg->msc_cfg.msc_blh_dlt_lut[0] = clamp(4096/max(mod_cfg->msc_cfg.msc_blh_lut[0], 1), 4, 2048);
			mod_cfg->msc_cfg.msc_blh_dlt_lut[8] = clamp(4096/max(mod_cfg->msc_cfg.msc_blh_lut[7], 1), 4, 2048);
			for(i = 1; i < 8; i++) {
				mod_cfg->msc_cfg.msc_blh_dlt_lut[i] = clamp(8192/max(mod_cfg->msc_cfg.msc_blh_lut[i] + mod_cfg->msc_cfg.msc_blh_lut[i-1], 1), 4, 2048);
			}
		}
	}

	// update otp infomation
	if (isp_gen->sensor_otp.otp_enable) {
		if (isp_gen->sensor_otp.otp_info && isp_gen->sensor_otp.pmsc_table &&
			isp_gen->sensor_otp.pwb_table && isp_gen->sensor_otp.paf_table) {
			/* msc */
			if (isp_gen->stitch_mode == STITCH_2IN1_LINNER) {
				if (isp_gen->isp_id % 2 == 0) {
					for(i = 0; i < OTP_MSC_SIZE; i++) {
						isp_gen->sensor_otp.otp_info->msc_golden_ratio[i] = (float)max(isp_gen->sensor_otp.pmsc_table[i], 1) / max(msc_golden[i], 1);
					}
					isp_gen->sensor_otp.otp_info->msc_r_ratio = (float)max(isp_gen->sensor_otp.pmsc_table[0] + isp_gen->sensor_otp.pmsc_table[15] + isp_gen->sensor_otp.pmsc_table[15*16] + isp_gen->sensor_otp.pmsc_table[16*16 - 1], 1) /
						max(msc_golden[0] + msc_golden[15] + msc_golden[15*16] + msc_golden[16*16 - 1], 1);
				} else {
					for(i = 0; i < OTP_MSC_SIZE; i++) {
						isp_gen->sensor_otp.otp_info->msc_golden_ratio[i] = (float)max(isp_gen->sensor_otp.pmsc_table[i], 1) / max(msc_golden[OTP_MSC_SIZE + i], 1);
					}
					isp_gen->sensor_otp.otp_info->msc_r_ratio = (float)max(isp_gen->sensor_otp.pmsc_table[0] + isp_gen->sensor_otp.pmsc_table[15] + isp_gen->sensor_otp.pmsc_table[15*16] + isp_gen->sensor_otp.pmsc_table[16*16 - 1], 1) /
						max(msc_golden[OTP_MSC_SIZE + 0] + msc_golden[OTP_MSC_SIZE + 15] + msc_golden[OTP_MSC_SIZE + 15*16] + msc_golden[OTP_MSC_SIZE + 16*16 - 1], 1);
				}
			} else {
				for(i = 0; i < OTP_MSC_SIZE; i++) {
					isp_gen->sensor_otp.otp_info->msc_golden_ratio[i] = (float)max(isp_gen->sensor_otp.pmsc_table[i], 1) / max(msc_golden[i], 1);
				}
				isp_gen->sensor_otp.otp_info->msc_r_ratio = (float)max(isp_gen->sensor_otp.pmsc_table[0] + isp_gen->sensor_otp.pmsc_table[15] + isp_gen->sensor_otp.pmsc_table[15*16] + isp_gen->sensor_otp.pmsc_table[16*16 - 1], 1) /
					max(msc_golden[0] + msc_golden[15] + msc_golden[15*16] + msc_golden[16*16 - 1], 1);
			}
			for (i = 0; i < OTP_MSC_SIZE / 3; i++) {
				if (isp_gen->sensor_otp.otp_info->msc_r_ratio > 1.0) {
					isp_gen->sensor_otp.otp_info->msc_golden_flag[i] = 1;
				} else {
					isp_gen->sensor_otp.otp_info->msc_golden_flag[i] = 0;
				}
			}
			//printf("isp%d ratio\n", isp_gen->isp_id);
			//for(i = 0; i < OTP_MSC_SIZE; i++) {
			//	if (i % 16 == 0)
			//		printf("\n");
			//	printf("%.4f, ", isp_gen->sensor_otp.otp_info->msc_golden_ratio[i]);
			//}
			//printf("\n");

			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[0] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[1] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[2] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[3] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[4] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[5] = 0;// more less more green

			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[0] = 0;// more less more red
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[1] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[2] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[3] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[4] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[5] = 0;
#if 0
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[0] = 5;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[1] = 5;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[2] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[3] = 10;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[4] = 11;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio[5] = 11;// more less more green

			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[0] = -5;// more less more red
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[1] = -5;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[2] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[3] = 0;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[4] = -6;
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[5] = -8;
#endif
#if 0
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[0] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[0];
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[1] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[1];
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[2] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[2];
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[3] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[3];
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[4] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[4];
			isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[5] = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[5];

			ISP_LIB_LOG(ISP_LOG_LSC, "%f, %f, %f, %f, %f, %f. \n",
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[0],
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[1],
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[2],
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[3],
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[4],
				isp_gen->sensor_otp.otp_info->msc_adjust_ratio_less[5]);
#endif

			/* wb */
			for(i = 0; i < ISP_RAW_CH_MAX; i++) {
				ISP_LIB_LOG(ISP_LOG_AWB, "awb corner :%5d, golden: %d. \n", isp_gen->sensor_otp.pwb_table[i], isp_gen->sensor_otp.pwb_table[i+4]);
			}
			isp_gen->sensor_otp.otp_info->wb_golden_ratio[0] = (float)max(isp_gen->sensor_otp.pwb_table[0], 1) / (float)max(isp_gen->sensor_otp.pwb_table[0+4], 1);
			isp_gen->sensor_otp.otp_info->wb_golden_ratio[1] = (float)max(isp_gen->sensor_otp.pwb_table[1] + isp_gen->sensor_otp.pwb_table[2], 1)
				/ (float)max(isp_gen->sensor_otp.pwb_table[1+4] + isp_gen->sensor_otp.pwb_table[2+4], 1);
			isp_gen->sensor_otp.otp_info->wb_golden_ratio[2] = (float)max(isp_gen->sensor_otp.pwb_table[3], 1) / (float)max(isp_gen->sensor_otp.pwb_table[3+4], 1);

			ISP_LIB_LOG(ISP_LOG_AWB, "awb otp ratio r :%5f, g: %5f, b: %5f. \n", isp_gen->sensor_otp.otp_info->wb_golden_ratio[0], isp_gen->sensor_otp.otp_info->wb_golden_ratio[1], isp_gen->sensor_otp.otp_info->wb_golden_ratio[2]);

			for(i = 0; i < param->isp_3a_settings.awb_light_num; i++) {
				ISP_LIB_LOG(ISP_LOG_AWB, "Before awb otp adjust %5d %5d %5d \n",param->isp_3a_settings.awb_light_info[10*i + 0], param->isp_3a_settings.awb_light_info[10*i + 1], param->isp_3a_settings.awb_light_info[10*i + 2]);
				param->isp_3a_settings.awb_light_info[10*i + 0] = (HW_S32)(param->isp_3a_settings.awb_light_info[10*i + 0]*isp_gen->sensor_otp.otp_info->wb_golden_ratio[0]);
				param->isp_3a_settings.awb_light_info[10*i + 1] = (HW_S32)(param->isp_3a_settings.awb_light_info[10*i + 1]*isp_gen->sensor_otp.otp_info->wb_golden_ratio[1]);
				param->isp_3a_settings.awb_light_info[10*i + 2] = (HW_S32)(param->isp_3a_settings.awb_light_info[10*i + 2]*isp_gen->sensor_otp.otp_info->wb_golden_ratio[2]);
				ISP_LIB_LOG(ISP_LOG_AWB, "Before awb otp adjust %5d %5d %5d \n",param->isp_3a_settings.awb_light_info[10*i + 0], param->isp_3a_settings.awb_light_info[10*i + 1], param->isp_3a_settings.awb_light_info[10*i + 2]);
			}

			/* af */
			if (param->isp_test_settings.af_en) {
				if ((!isp_gen->sensor_otp.paf_table[0] && !isp_gen->sensor_otp.paf_table[1]) || (isp_gen->sensor_otp.paf_table[1] > isp_gen->sensor_otp.paf_table[0]) ||
					(isp_gen->sensor_otp.paf_table[0] > 1023) || (isp_gen->sensor_otp.paf_table[1] > 1023) ||
					(!af_golden[0] && !af_golden[1]) || (af_golden[1] > af_golden[0]) || (af_golden[0] > 1023) || (af_golden[1] > 1023)) {
					ISP_PRINT("AF_OTP data failed, disable AF_OTP. (gd_max = %d, gd_min = %d, otp_max = %d, otp_min = %d)\n",
						af_golden[0], af_golden[1], isp_gen->sensor_otp.paf_table[0], isp_gen->sensor_otp.paf_table[1]);
					isp_gen->sensor_otp.otp_info->af_max_code_offset = 0;
					isp_gen->sensor_otp.otp_info->af_min_code_offset = 0;
				} else {
					isp_gen->sensor_otp.otp_info->af_max_code_offset = (int)isp_gen->sensor_otp.paf_table[0] - af_golden[0];
					isp_gen->sensor_otp.otp_info->af_min_code_offset = (int)isp_gen->sensor_otp.paf_table[1] - af_golden[1];
					ISP_LIB_LOG(ISP_LOG_AF, "otp af max offset = %d, af min offset = %d. \n", isp_gen->sensor_otp.otp_info->af_max_code_offset, isp_gen->sensor_otp.otp_info->af_min_code_offset);
				}
			} else {
				isp_gen->sensor_otp.otp_info->af_max_code_offset = 0;
				isp_gen->sensor_otp.otp_info->af_min_code_offset = 0;
			}
		} else {
			ISP_ERR("otp enable, buf otp pointer error, disable OTP.(otp_info=%p, pmsc_table=%p, pwb_table=%p, paf_table%p)\n", isp_gen->sensor_otp.otp_info,
				isp_gen->sensor_otp.pmsc_table, isp_gen->sensor_otp.pwb_table, isp_gen->sensor_otp.paf_table);
			isp_gen->sensor_otp.otp_enable = 0;
		}
	}

	/*sharp*/
	mod_cfg->sharp_cfg.ss_shp_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_SS_SHP_RATIO];
	mod_cfg->sharp_cfg.ls_shp_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_LS_SHP_RATIO];
	mod_cfg->sharp_cfg.ss_dir_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_SS_DIR_RATIO];
	mod_cfg->sharp_cfg.ls_dir_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_LS_DIR_RATIO];
	mod_cfg->sharp_cfg.ss_crc_stren = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_SS_CRC_STREN];
	mod_cfg->sharp_cfg.ss_crc_min = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_SS_CRC_MIN];
	mod_cfg->sharp_cfg.wht_sat_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_WHT_SAT_RATIO];
	mod_cfg->sharp_cfg.blk_sat_ratio = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_BLK_SAT_RATIO];
	mod_cfg->sharp_cfg.wht_slp_bt = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_WHT_SLP_BT];
	mod_cfg->sharp_cfg.blk_slp_bt = param->isp_tunning_settings.sharp_comm_cfg[ISP_SHARP_BLK_SLP_BT];
	memcpy(&mod_cfg->sharp_cfg.sharp_ss_value[0], &param->isp_tunning_settings.isp_sharp_ss_value[0],
		ISP_REG_TBL_LENGTH * sizeof(HW_U16));
	memcpy(&mod_cfg->sharp_cfg.sharp_ls_value[0], &param->isp_tunning_settings.isp_sharp_ls_value[0],
		ISP_REG_TBL_LENGTH * sizeof(HW_U16));
	memcpy(&mod_cfg->sharp_cfg.sharp_hsv[0], &param->isp_tunning_settings.isp_sharp_hsv[0],
		46 * sizeof(HW_U16));

#ifdef USE_ENCPP
	/*encpp sharp*/
	isp_gen->encpp_static_sharp_cfg.ss_shp_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_SS_SHP_RATIO];
	isp_gen->encpp_static_sharp_cfg.ls_shp_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_LS_SHP_RATIO];
	isp_gen->encpp_static_sharp_cfg.ss_dir_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_SS_DIR_RATIO];
	isp_gen->encpp_static_sharp_cfg.ls_dir_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_LS_DIR_RATIO];
	isp_gen->encpp_static_sharp_cfg.ss_crc_stren = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_SS_CRC_STREN];
	isp_gen->encpp_static_sharp_cfg.ss_crc_min = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_SS_CRC_MIN];
	isp_gen->encpp_static_sharp_cfg.wht_sat_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_WHT_SAT_RATIO];
	isp_gen->encpp_static_sharp_cfg.blk_sat_ratio = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_BLK_SAT_RATIO];
	isp_gen->encpp_static_sharp_cfg.wht_slp_bt = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_WHT_SLP_BT];
	isp_gen->encpp_static_sharp_cfg.blk_slp_bt = param->isp_tunning_settings.encpp_sharp_comm_cfg[ENCPP_SHARP_BLK_SLP_BT];
	memcpy(&isp_gen->encpp_static_sharp_cfg.sharp_ss_value[0], &param->isp_tunning_settings.encpp_sharp_ss_value[0],
		ISP_REG_TBL_LENGTH * sizeof(HW_U16));
	memcpy(&isp_gen->encpp_static_sharp_cfg.sharp_ls_value[0], &param->isp_tunning_settings.encpp_sharp_ls_value[0],
		ISP_REG_TBL_LENGTH * sizeof(HW_U16));
	memcpy(&isp_gen->encpp_static_sharp_cfg.sharp_hsv[0], &param->isp_tunning_settings.encpp_sharp_hsv[0],
		46 * sizeof(HW_U16));
#endif

	config_band_step(isp_gen);

	//drc
	mod_cfg->drc_cfg.drc_scale = 0;

	//gamma table
	if (param->isp_test_settings.gamma_en) {
		config_gamma(isp_gen);
	} else {
		for (i = 0; i < 1024; i++) {
			mod_cfg->gamma_cfg.gamma_tbl[i] = i * 4;
			mod_cfg->gamma_cfg.gamma_tbl[i + 1024] = i * 4;
			mod_cfg->gamma_cfg.gamma_tbl[i + 2048] = i * 4;
		}
	}

	if (param->isp_test_settings.lsc_en) {
		config_lens_table(isp_gen, 512);
		config_lens_center(isp_gen);
	}
	if (param->isp_test_settings.msc_en) {
		config_msc_table(isp_gen, 512, 256);
	}

	config_nr_msc_table(isp_gen, 1);

	memcpy(&mod_cfg->cem_cfg.cem_table[0], &param->isp_tunning_settings.isp_cem_table[0], ISP_CEM_MEM_SIZE);
}

int __isp_alloc_reg_tbl(struct isp_lib_context *isp_gen)
{
	unsigned int isp_default_reg[ISP_LOAD_REG_SIZE >> 2] = {
#if (ISP_VERSION >= 600)
		0x00000005, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000001, 0x140162f0, 0x005f0f0c, 0x00000013,
		0x00000000, 0x00000080, 0x00000000, 0x14010000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000080, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000080, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000080, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x000c00c7, 0x00000053, 0x0fa0002a, 0x00000000,
		0x00000000, 0x14011a80, 0x00000000, 0x00000000,
		0x04380780, 0x04380780, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000030, 0x00100110,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
#if (ISP_VERSION == 606)
		0x1e0c1e1e, 0x00240100, 0x241e0c1e, 0x000c0100,
#elif (ISP_VERSION == 603)
		0x1e0c1e1e, 0x00240040, 0x241e0c1e, 0x000c0040,
#elif (ISP_VERSION == 602)
		0x1e0c1e1e, 0x00240130, 0x241e0c1e, 0x000c0130,
#elif (ISP_VERSION == 600) && (ISP_SMALL_VERSION == 200)
		0x1a0c1a1a, 0x00240130, 0x241a0c1a, 0x000c0130,
#else
		0x120c1212, 0x002400c0, 0x24120c12, 0x000c00c0,
#endif
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x000180c0, 0x80008000, 0x00000000, 0x00000000,
		0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
		0x04000400, 0x04000400, 0x00000000, 0x00000000,
		0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
		0x04000400, 0x04000400, 0x00000000, 0x00000000,
		0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
		0x04000400, 0x04000400, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x07033371, 0x0fffff0f, 0x00000000, 0x00000000,
		0xffff0013, 0x0f000200, 0x00400015, 0x0f000333,
		0x04000040, 0x08000010, 0x08000002, 0x04000004,
		0xffff0013, 0x0f000200, 0x00400015, 0x0f000333,
		0x04000040, 0x08000010, 0x08000002, 0x04000004,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0xa4a0c040, 0x08004000, 0x0aaaee0c, 0x00000000,
		0x0136003c, 0x00000106, 0x00005040, 0x00000000,
		0x03bf021b, 0x0023fb0e, 0x0008ad60, 0x0000000a,
		0x00000000, 0x57310000, 0x002f0053, 0x00000000,
		0x80008000, 0x0f000200, 0x0f000200, 0x00130013,
		0x00800080, 0x00000000, 0x01040609, 0x202000c3,
		0x00100010, 0x01000100, 0x00100010, 0x01000100,
		0xc627a010, 0x00000000, 0x00000000, 0x00000000,
		0x4000401c, 0x40040100, 0x40003024, 0x40040100,
		0x40002018, 0x40040100, 0x4000181c, 0x40040100,
		0x0400008a, 0x40000100, 0x0020400f, 0x00000000,
		0x00d50078, 0x0a8c3c2d, 0x08000044, 0x07d2005b,
		0x50185050, 0x20401010, 0x00000001, 0x00000000,
		0x02002020, 0x02002020, 0x00800080, 0x00000000,
		0x010100c0, 0x011001e0, 0x011801e8, 0x012001f0,
		0x012c01f8, 0x013801fa, 0x013c01fc, 0x013f01ff,
		0x3c244058, 0x00005678, 0x20408010, 0x20408005,
		0x2a002300, 0x00000160, 0x1fc01fc0, 0x1fc01fc0,
		0x01000100, 0x01000100, 0x00000fff, 0x00000000,
		0x04000400, 0x04000400, 0x00000000, 0x00000000,
		0x304383c0, 0x304383c0, 0x304383c0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x800f8000, 0x000003ff, 0x0a8c2d3c, 0x08000044,
		0x07d2005b, 0x00000000, 0x00000000, 0x00000000,
		0x00800080, 0x08010200, 0xff008020, 0x00000000,
		0x00800080, 0x08010200, 0xff008020, 0x00000000,
		0x00800080, 0x08010200, 0xff008020, 0x00000000,
		0x00800080, 0x08010200, 0xff008020, 0x00000000,
		0x00800000, 0x0024f000, 0x02000010, 0x0084c000,
		0xffffff74, 0x00001010, 0x0fa00fff, 0x01010101,
		0x03000200, 0x00600140, 0x01189200, 0x0008fb20,
		0x00000000, 0x00801000, 0x16010040, 0x40100210,
		0x00000000, 0x00000000, 0x00100100, 0x00000000,
		0x00000000, 0x00000000, 0x00100100, 0x00000000,
		0x00000000, 0x00200020, 0x00018600, 0x00000000,
		0x00000000, 0x00200020, 0x00018600, 0x00000000,
		0x00000000, 0x00200020, 0x00018600, 0x00000000,
		0x00000100, 0x00000000, 0x00000100, 0x00000000,
		0x00000100, 0x00000000, 0x00000000, 0x00000000,
		0x00100010, 0x00100010, 0x00000000, 0x00000000,
		0x00000008, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x02040107, 0x07680064, 0x01c206d6, 0x068701c2,
		0x004007b7, 0x02000200, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x003c0050, 0x00040400, 0x000012c0,
		0x00000000, 0x00000000, 0x01000100, 0x01000100,
		0x01080000, 0x008600f0, 0x00000000, 0x39ccf1ac,
		0x00029391, 0x00000000, 0x00000000, 0x00000035,
		0x0000027b, 0x0000009d, 0x00000088, 0x2fb00511,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x000000f0,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x003c0050, 0x0fff0004, 0x0fff0fff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000008, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x04380780, 0x00000004, 0x00000000,
		0x00000000, 0x04380780, 0x00000004, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00040200, 0x00100580, 0x00400880, 0x02001000,
		0x00000000, 0x01800480, 0x08000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00310057, 0x00310057, 0x00310057, 0x00310057,
		0x00310057, 0x00310057, 0x00310057, 0x00310057,
		0x00310057, 0x00310057, 0x00310057, 0x00000000,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x00310057, 0x00310057, 0x00310057, 0x00310057,
		0x00310057, 0x00310057, 0x00310057, 0x00310057,
		0x00310057, 0x00310057, 0x00310057, 0x00000000,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00000000, 0x00000000, 0x00000000,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00000000, 0x00000000, 0x00000000,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00000000, 0x00000000, 0x00000000,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00000000, 0x00000000, 0x00000000,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00100010, 0x00100010, 0x00100010,
		0x00100010, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x01010101, 0x01010101, 0x01010101, 0x01010101,
		0x01010101, 0x01010101, 0x01010101, 0x01010101,
		0x01010101, 0x00000000, 0x00000000, 0x00000000,
		0x01010101, 0x01010101, 0x01010101, 0x01010101,
		0x01010101, 0x01010101, 0x01010101, 0x01010101,
		0x01010101, 0x00000000, 0x00000000, 0x00000000,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
		0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0x00000000, 0x0f0f0f0f, 0x0f0f0f0f,
		0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f,
		0x0f0f0f0f, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0x00000000, 0x0ff3fcff, 0x0ff3fcff,
		0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff,
		0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff,
		0x0ff3fcff, 0x0ff3fcff, 0x000f000f, 0x000f000f,
		0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
		0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
		0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
		0x000f000f, 0x000f000f, 0x000f000f, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
#endif
	};

	/* REG LOAD*/
	isp_gen->load_reg_base = malloc(ISP_LOAD_DRAM_SIZE);
	if(isp_gen->load_reg_base == NULL) {
		ISP_ERR("load_reg_base alloc failed, no memory!");
		return -1;
	}
	memset(isp_gen->load_reg_base, 0, ISP_LOAD_REG_SIZE);
	memcpy(isp_gen->load_reg_base, &isp_default_reg[0], ISP_LOAD_REG_SIZE);

	isp_gen->module_cfg.isp_dev_id = isp_gen->isp_id;
	isp_gen->module_cfg.isp_platform_id = ISP_PLATFORM_SUN8IW21P1;

	isp_map_addr(&isp_gen->module_cfg, (unsigned long)isp_gen->load_reg_base);
#if (ISP_VERSION >= 600)
	/*FE TABLE*/
	isp_gen->module_cfg.fe_table = isp_gen->load_reg_base + ISP_LOAD_REG_SIZE;
	memset(isp_gen->module_cfg.fe_table, 0, ISP_FE_TABLE_SIZE);

	isp_gen->module_cfg.ch0_msc_table = isp_gen->module_cfg.fe_table + ISP_CH0_MSC_FE_MEM_OFS;
	isp_gen->module_cfg.ch1_msc_table = isp_gen->module_cfg.fe_table + ISP_CH1_MSC_FE_MEM_OFS;
	isp_gen->module_cfg.ch2_msc_table = isp_gen->module_cfg.fe_table + ISP_CH2_MSC_FE_MEM_OFS;

	/*BAYER TABLE*/
	isp_gen->module_cfg.bayer_table = isp_gen->module_cfg.fe_table + ISP_FE_TABLE_SIZE;
	memset(isp_gen->module_cfg.bayer_table, 0, ISP_BAYER_TABLE_SIZE);

	isp_gen->module_cfg.lens_table = isp_gen->module_cfg.bayer_table + ISP_LSC_MEM_OFS;
	isp_gen->module_cfg.msc_table = isp_gen->module_cfg.bayer_table + ISP_MSC_MEM_OFS;
	isp_gen->module_cfg.nr_msc_table = isp_gen->module_cfg.bayer_table + ISP_MSC_NR_MEM_OFS;
	isp_gen->module_cfg.tdnf_table = isp_gen->module_cfg.bayer_table + ISP_TDNF_DK_MEM_OFS;
	isp_gen->module_cfg.pltm_table = isp_gen->module_cfg.bayer_table + ISP_PLTM_TM_MEM_OFS;

	/*RGB TABLE*/
	isp_gen->module_cfg.rgb_table = isp_gen->module_cfg.bayer_table + ISP_BAYER_TABLE_SIZE;
	memset(isp_gen->module_cfg.rgb_table, 0, ISP_RGB_TABLE_SIZE);

	isp_gen->module_cfg.drc_table = isp_gen->module_cfg.rgb_table + ISP_DRC_MEM_OFS;
	isp_gen->module_cfg.gamma_table = isp_gen->module_cfg.rgb_table + ISP_GAMMA_MEM_OFS;

	/*YUV TABLE*/
	isp_gen->module_cfg.yuv_table = isp_gen->module_cfg.rgb_table + ISP_RGB_TABLE_SIZE;
	memset(isp_gen->module_cfg.yuv_table, 0, ISP_YUV_TABLE_SIZE);

	isp_gen->module_cfg.cem_table = isp_gen->module_cfg.yuv_table + ISP_CEM_MEM_OFS;
#endif
	return 0;
}

int __isp_dump_reg(struct isp_lib_context *isp_gen)
{
	int i, *reg;
	printf("dump ISP%d regs :\n", isp_gen->isp_id);
	for(i = 0; i < 0x40; i = i + 4)
	{
		reg = (int*)(isp_gen->load_reg_base + i);
		if(i % 0x10 == 0)
			printf("0x%08x:  ", i);
		printf("0x%08x, ", reg[0]);
		if(i % 0x10 == 0xc)
			printf("\n");
	}
	for(i = 0x40; i < 0x400; i = i + 4)
	{
		reg = (int*)(isp_gen->load_reg_base + i);

		if(i % 0x10 == 0)
			printf("0x%08x:  ", i);
		printf("0x%08x, ", reg[0]);
		if(i % 0x10 == 0xc)
			printf("\n");
	}

	return 0;
}

HW_S32 isp_ctx_stats_prepare(struct isp_lib_context *isp_gen, const void *buffer)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	isp_handle_stats(isp_gen, buffer);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}
HW_S32 isp_ctx_stats_prepare_sync(struct isp_lib_context *isp_gen, const void *buffer0, const void *buffer1)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	isp_handle_stats_sync(isp_gen, buffer0, buffer1);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}


HW_S32 isp_ctx_stats_req(struct isp_lib_context *isp_gen,
					struct isp_stats_context *stats_ctx)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	*stats_ctx = isp_gen->stats_ctx;
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}

HW_S32 __isp_ctx_update_iso_cfg(struct isp_lib_context *isp_gen)
{
	isp_iso_entity_context_t *isp_iso_cxt = &isp_gen->iso_entity_ctx;

	if (!isp_gen->iso_entity_ctx.iso_param) {
		return -1;
	}

	isp_gen->iso_entity_ctx.iso_param->isp_gen = isp_gen;

	isp_gen->iso_entity_ctx.iso_param->cnr_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->sharpness_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->brightness_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->contrast_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->cem_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->denoise_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->black_level_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->dpc_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->defog_value_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->pltm_dynamic_cfg_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->tdnr_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->ae_cfg_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->gtm_cfg_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->lca_cfg_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->wdr_cfg_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->cfa_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->shading_adjust = true;
	isp_gen->iso_entity_ctx.iso_param->encoder_adjust = true;
#if (ISP_VERSION >= 602)
	isp_gen->iso_entity_ctx.iso_param->awb_adjust = true;
#endif
	if((isp_gen->isp_ini_cfg.isp_test_settings.af_en == 1) || (isp_gen->isp_ini_cfg.isp_test_settings.isp_test_focus == 1))
		isp_gen->iso_entity_ctx.iso_param->af_cfg_adjust = true;
	else
		isp_gen->iso_entity_ctx.iso_param->af_cfg_adjust = false;

	isp_gen->iso_entity_ctx.iso_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;

	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_UPDATE_PARAMS);

	isp_iso_cxt->ops->isp_iso_run(isp_iso_cxt->iso_entity, &isp_iso_cxt->iso_result);

	return 0;
}

HW_S32 __isp_ctx_update_af_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->af_entity_ctx.af_param) {
		return -1;
	}

	//af_ini_cfg.
	isp_gen->af_entity_ctx.af_param->af_ini.af_use_otp = isp_gen->isp_ini_cfg.isp_3a_settings.af_use_otp;
	if (isp_gen->sensor_otp.otp_enable && isp_gen->sensor_otp.otp_info) {
		isp_gen->af_entity_ctx.af_param->af_ini.vcm_min_code = clamp(isp_gen->isp_ini_cfg.isp_3a_settings.vcm_min_code + isp_gen->sensor_otp.otp_info->af_min_code_offset, 0, 1023);
		isp_gen->af_entity_ctx.af_param->af_ini.vcm_max_code = clamp(isp_gen->isp_ini_cfg.isp_3a_settings.vcm_max_code + isp_gen->sensor_otp.otp_info->af_max_code_offset, 0, 1023);
	} else {
		isp_gen->af_entity_ctx.af_param->af_ini.vcm_min_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_min_code;
		isp_gen->af_entity_ctx.af_param->af_ini.vcm_max_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_max_code;
	}
	isp_gen->af_entity_ctx.af_param->af_ini.af_interval_time = isp_gen->isp_ini_cfg.isp_3a_settings.af_interval_time;
	isp_gen->af_entity_ctx.af_param->af_ini.af_speed_ind = isp_gen->isp_ini_cfg.isp_3a_settings.af_speed_ind;
	isp_gen->af_entity_ctx.af_param->af_ini.af_auto_fine_en = isp_gen->isp_ini_cfg.isp_3a_settings.af_auto_fine_en;
	isp_gen->af_entity_ctx.af_param->af_ini.af_single_fine_en = isp_gen->isp_ini_cfg.isp_3a_settings.af_single_fine_en;
	isp_gen->af_entity_ctx.af_param->af_ini.af_fine_step = isp_gen->isp_ini_cfg.isp_3a_settings.af_fine_step;
	isp_gen->af_entity_ctx.af_param->af_ini.af_move_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_cnt;
	isp_gen->af_entity_ctx.af_param->af_ini.af_still_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_cnt;
	isp_gen->af_entity_ctx.af_param->af_ini.af_move_monitor_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_monitor_cnt;
	isp_gen->af_entity_ctx.af_param->af_ini.af_still_monitor_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_monitor_cnt;
	isp_gen->af_entity_ctx.af_param->af_ini.af_stable_min = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_min;
	isp_gen->af_entity_ctx.af_param->af_ini.af_stable_max = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_max;
	isp_gen->af_entity_ctx.af_param->af_ini.af_low_light_lv = isp_gen->isp_ini_cfg.isp_3a_settings.af_low_light_lv;
	isp_gen->af_entity_ctx.af_param->af_ini.af_near_tolerance = isp_gen->isp_ini_cfg.isp_3a_settings.af_near_tolerance;
	isp_gen->af_entity_ctx.af_param->af_ini.af_far_tolerance = isp_gen->isp_ini_cfg.isp_3a_settings.af_far_tolerance;
	isp_gen->af_entity_ctx.af_param->af_ini.af_tolerance_off = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_off;
	isp_gen->af_entity_ctx.af_param->af_ini.af_peak_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_peak_th;
	isp_gen->af_entity_ctx.af_param->af_ini.af_dir_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_dir_th;
	isp_gen->af_entity_ctx.af_param->af_ini.af_change_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.af_change_ratio;
	isp_gen->af_entity_ctx.af_param->af_ini.af_move_minus = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_minus;
	isp_gen->af_entity_ctx.af_param->af_ini.af_still_minus = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_minus;
	isp_gen->af_entity_ctx.af_param->af_ini.af_scene_motion_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_scene_motion_th;
	isp_gen->af_entity_ctx.af_param->af_ini.af_tolerance_tbl_len = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_tbl_len;
	memcpy(&isp_gen->af_entity_ctx.af_param->af_ini.af_std_code_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.af_std_code_tbl[0], 20*sizeof(int));
	memcpy(&isp_gen->af_entity_ctx.af_param->af_ini.af_tolerance_value_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[0], 20*sizeof(int));
	isp_gen->af_entity_ctx.af_param->af_ini.af_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.af_reserve_0;
	isp_gen->af_entity_ctx.af_param->af_ini.af_touch_dist_ind = isp_gen->isp_ini_cfg.isp_3a_settings.af_reserve_1;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_INI_DATA);

	isp_gen->af_entity_ctx.af_param->vcm.vcm_max_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_max_code;
	isp_gen->af_entity_ctx.af_param->vcm.vcm_min_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_min_code;

	isp_gen->af_entity_ctx.af_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->af_entity_ctx.af_param->test_cfg.isp_test_focus = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_focus;
	isp_gen->af_entity_ctx.af_param->test_cfg.focus_start = isp_gen->isp_ini_cfg.isp_test_settings.focus_start;
	isp_gen->af_entity_ctx.af_param->test_cfg.focus_step = isp_gen->isp_ini_cfg.isp_test_settings.focus_step;
	isp_gen->af_entity_ctx.af_param->test_cfg.focus_end = isp_gen->isp_ini_cfg.isp_test_settings.focus_end;
	isp_gen->af_entity_ctx.af_param->test_cfg.focus_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.focus_change_interval;
	isp_gen->af_entity_ctx.af_param->test_cfg.af_en = isp_gen->isp_ini_cfg.isp_test_settings.af_en;

	return 0;
}
HW_S32 __isp_ctx_update_awb_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->awb_entity_ctx.awb_param) {
		return -1;
	}

	//awb_ini_cfg.
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_interval = isp_gen->isp_ini_cfg.isp_3a_settings.awb_interval;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_speed = isp_gen->isp_ini_cfg.isp_3a_settings.awb_speed;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_color_temper_low = isp_gen->isp_ini_cfg.isp_3a_settings.awb_color_temper_low;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_color_temper_high = isp_gen->isp_ini_cfg.isp_3a_settings.awb_color_temper_high;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_base_temper = isp_gen->isp_ini_cfg.isp_3a_settings.awb_base_temper;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_green_zone_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_green_zone_dist;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_blue_sky_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_blue_sky_dist;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_light_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_light_num;
	//isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_ext_light_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_ext_light_num;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_ext_light_num = 0;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_skin_color_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_skin_color_num;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_special_color_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_special_color_num;
#if (ISP_VERSION < 602)
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_rgain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.awb_rgain_favor;
	isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_bgain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.awb_bgain_favor;
#endif
	memcpy(&isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_light_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_light_info[0], 160*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_ext_light_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[0], 80*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_skin_color_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_skin_color_info[0], 40*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_special_color_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_special_color_info[0], 80*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param->awb_ini.awb_preset_gain[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_preset_gain[0], 22*sizeof(int));

	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_INI_DATA);

	//test cfg.
	isp_gen->awb_entity_ctx.awb_param->test_cfg.awb_en = isp_gen->isp_ini_cfg.isp_test_settings.awb_en;
	isp_gen->awb_entity_ctx.awb_param->test_cfg.isp_color_temp = isp_gen->isp_ini_cfg.isp_test_settings.isp_color_temp;
#if (ISP_VERSION >= 602)
	isp_gen->awb_entity_ctx.awb_param->test_cfg.isp_test_mode = clamp(isp_gen->isp_ini_cfg.isp_3a_settings.local_wb_coef, -1, 20);
#else
	if (isp_gen->stitch_mode == STITCH_NONE) {
		isp_gen->awb_entity_ctx.awb_param->test_cfg.isp_test_mode = 10;//isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	} else {
		isp_gen->awb_entity_ctx.awb_param->test_cfg.isp_test_mode = -1;
	}
#endif

	return 0;
}

HW_S32 __isp_ctx_update_ae_cfg(struct isp_lib_context *isp_gen)
{
	//struct isp_param_config *param = &isp_gen->isp_ini_cfg;

	if (!isp_gen->ae_entity_ctx.ae_param) {
		return -1;
	}

	//ae_sensor_info.
	isp_gen->ae_entity_ctx.ae_param->ae_sensor_info = isp_gen->sensor_info;
	//ae_ini_cfg.
	isp_gen->ae_entity_ctx.ae_param->ae_ini.define_ae_table = isp_gen->isp_ini_cfg.isp_3a_settings.define_ae_table;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_max_lv = isp_gen->isp_ini_cfg.isp_3a_settings.ae_max_lv;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview_length;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture_length;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video_length;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_hist_mod_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_ev_step = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ev_step;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_ConvDataIndex = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ConvDataIndex;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_blowout_pre_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_pre_en;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_blowout_attr = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_attr;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.ae_delay_frame;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.exp_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.exp_delay_frame;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.gain_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.gain_delay_frame;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.exp_comp_step = isp_gen->isp_ini_cfg.isp_3a_settings.exp_comp_step;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_touch_dist_ind = isp_gen->isp_ini_cfg.isp_3a_settings.ae_touch_dist_ind;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_iso2gain_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.ae_iso2gain_ratio;
	isp_gen->ae_settings.flicker_type = isp_gen->isp_ini_cfg.isp_tunning_settings.flicker_type;

	// wdr mode: 0 is default, use 256
	//isp_gen->isp_ini_cfg.isp_3a_settings.wdr_cfg[WDR_MODE] = 0;

	// gain_ratio: 0
	//isp_gen->ae_entity_ctx.ae_param->ae_ini.gain_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.gain_ratio;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.gain_ratio = 0;

	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_analog_gain_range[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_analog_gain_range[0], 2*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_digital_gain_range[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_digital_gain_range[0], 2*sizeof(int));
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_digital_gain_range[0] = 1024;
	isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_digital_gain_range[1] = 1024 * 63;

	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_fno_step[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_fno_step[0], 16*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_win_weight[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_win_weight[0], 64*sizeof(int));

	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_INI_DATA);

#if ISP_AI_SCENE_CONF
	// for ai scene param
	isp_gen->ai_scene.last_tune_settings.ae_table_night_length =
		isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length;
	memcpy(&isp_gen->ai_scene.last_tune_settings.ae_ai_night_table[0],
		&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0], 42*sizeof(int));
	normal_tune_settings.ae_table_night_length =
		isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length;
	memcpy(&normal_tune_settings.ae_ai_night_table[0],
		&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0], 42*sizeof(int));
	blue_sky_tune_settings.ae_table_night_length =
		isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length;
	memcpy(&blue_sky_tune_settings.ae_ai_night_table[0],
		&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0], 42*sizeof(int));
	green_plants_tune_settings.ae_table_night_length =
		isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length;
	memcpy(&green_plants_tune_settings.ae_ai_night_table[0],
		&isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0], 42*sizeof(int));
#endif

	//test cfg.
	isp_gen->ae_entity_ctx.ae_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.isp_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_gain;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.isp_exp_line = isp_gen->isp_ini_cfg.isp_test_settings.isp_exp_line;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.ae_forced = isp_gen->isp_ini_cfg.isp_test_settings.ae_forced;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.lum_forced = isp_gen->isp_ini_cfg.isp_test_settings.lum_forced;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.isp_test_exptime = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_exptime;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.exp_line_start = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_start;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.exp_line_step = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_step;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.exp_line_end = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_end;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.exp_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.exp_change_interval;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.isp_test_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_gain;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.gain_start = isp_gen->isp_ini_cfg.isp_test_settings.gain_start;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.gain_step = isp_gen->isp_ini_cfg.isp_test_settings.gain_step;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.gain_end = isp_gen->isp_ini_cfg.isp_test_settings.gain_end;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.gain_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.gain_change_interval;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.ae_en = isp_gen->isp_ini_cfg.isp_test_settings.ae_en;

	// Check ae hardware delay.
	// When check isp wdr/dg delay, we should set ae_stat_sel equals 2, because the stat date before the wdr dealt was not changed.
	// When check sensor exp/ag delay, we should set ae_stat_sel equals 0, because the stat date after the wdr dealt was not accurate.
	isp_gen->ae_entity_ctx.ae_param->test_cfg.ae_check_delay_en = 0;
	isp_gen->ae_entity_ctx.ae_param->test_cfg.ae_delay_type = ISP_AE_DELAY_WDR_AGAIN;

	isp_gen->ae_entity_ctx.ae_param->ae_setting = isp_gen->ae_settings;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);

	//get gain range
	isp_gen->tune.gains.ana_gain_min = isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_analog_gain_range[0];
	isp_gen->tune.gains.ana_gain_max = isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_analog_gain_range[1];
	isp_gen->tune.gains.dig_gain_min = isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_digital_gain_range[0]/4; /*Q10 -> Q8*/
	isp_gen->tune.gains.dig_gain_max = isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_digital_gain_range[1]/4;

	return 0;
}

HW_S32 __isp_ctx_update_afs_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->afs_entity_ctx.afs_param) {
		return -1;
	}

	isp_gen->afs_entity_ctx.afs_param->flicker_ratio = isp_gen->isp_ini_cfg.isp_tunning_settings.flicker_ratio;
	isp_gen->afs_entity_ctx.afs_param->flicker_type_ini = isp_gen->isp_ini_cfg.isp_tunning_settings.flicker_type;
	isp_gen->afs_entity_ctx.afs_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->afs_entity_ctx.afs_param->test_cfg.afs_en = isp_gen->isp_ini_cfg.isp_test_settings.afs_en;

	return 0;
}

HW_S32 __isp_ctx_update_md_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->md_entity_ctx.md_param) {
		return -1;
	}

	isp_gen->md_entity_ctx.md_param->af_scene_motion_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_scene_motion_th;
	isp_gen->md_entity_ctx.md_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;

	return 0;
}

HW_S32 __isp_ctx_update_rolloff_cfg(struct isp_lib_context *isp_gen)
{
	int m;

	if (!isp_gen->rolloff_entity_ctx.rolloff_param) {
		return -1;
	}

	//rolloff_sensor_info.
	isp_gen->rolloff_entity_ctx.rolloff_param->rolloff_sensor_info = isp_gen->sensor_info;

	for (m = 0; m < 11 * ROLLOFF_WIN_SIZE; m++)
		isp_gen->rolloff_entity_ctx.rolloff_param->rolloff_ini.Rgain[m]
			= isp_gen->isp_ini_cfg.isp_tunning_settings.msc_tbl[(m / ROLLOFF_WIN_SIZE) + 1][m % ROLLOFF_WIN_SIZE];

	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_INI_DATA);

	return 0;
}
HW_S32 __isp_ctx_update_gtm_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->gtm_entity_ctx.gtm_param) {
		return -1;
	}

	isp_gen->gtm_entity_ctx.gtm_param->gtm_enable = isp_gen->isp_ini_cfg.isp_test_settings.gtm_en;
	isp_gen->gtm_entity_ctx.gtm_param->gamma_tbl =	&isp_gen->module_cfg.gamma_cfg.gamma_tbl[0];
	isp_gen->gtm_entity_ctx.gtm_param->drc_table =	&isp_gen->module_cfg.drc_cfg.drc_table[0];
	isp_gen->gtm_entity_ctx.gtm_param->drc_table_last = &isp_gen->module_cfg.drc_cfg.drc_table_last[0];
	//gtm_ini_cfg.
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gtm_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gtm_type;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gamma_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gamma_type;
	isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.AutoAlphaEn	= isp_gen->isp_ini_cfg.isp_tunning_settings.auto_alpha_en;
	memcpy(&isp_gen->gtm_entity_ctx.gtm_param->gtm_ini.gtm_cfg[0], &isp_gen->ae_settings.ae_hist_eq_cfg[0], ISP_GTM_HEQ_MAX*sizeof(HW_S16));
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_INI_DATA);

	//test cfg.
	isp_gen->gtm_entity_ctx.gtm_param->test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;

	return 0;
}

HW_S32 __isp_ctx_update_pltm_cfg(struct isp_lib_context *isp_gen)
{
	if (!isp_gen->pltm_entity_ctx.pltm_param) {
		return -1;
	}

	isp_gen->pltm_entity_ctx.pltm_param->pltm_enable = isp_gen->isp_ini_cfg.isp_test_settings.pltm_en;
	//pltm_ini_cfg.
	isp_gen->pltm_entity_ctx.pltm_param->gamma_tbl = &isp_gen->module_cfg.gamma_cfg.gamma_tbl[0];
	isp_gen->pltm_entity_ctx.pltm_param->pltm_max_stage = isp_gen->module_cfg.pltm_cfg.max_stage + 1;
	isp_gen->pltm_entity_ctx.pltm_param->hist0_sel = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist0_sel;
	isp_gen->pltm_entity_ctx.pltm_param->hist1_sel = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist1_sel;
	isp_gen->pltm_entity_ctx.pltm_param->hist0_buf = &isp_gen->stats_ctx.stats.ae_stats.hist[0];
	isp_gen->pltm_entity_ctx.pltm_param->hist1_buf = &isp_gen->stats_ctx.stats.ae_stats.hist1[0];
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param->pltm_ini.pltm_cfg[0],
		&isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[0], ISP_PLTM_MAX*sizeof(HW_S32));
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param->pltm_ini.pltm_dynamic_cfg[0],
		&isp_gen->ae_settings.pltm_dynamic_cfg[0], ISP_PLTM_DYNAMIC_MAX*sizeof(HW_S16));

	isp_gen->pltm_entity_ctx.pltm_param->sensor_info = isp_gen->sensor_info;
	return 0;
}

void __get_sensor_temp_param(struct isp_lib_context *isp_gen)
{
	HW_S16 temperature;
	HW_S16 temp_mapping[14] = {55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120};
	HW_U8 i, index;
	if (isp_gen->temp_info.enable) {
		if (isp_gen->sensor_info.temperature >= temp_mapping[13]) {
			for (i = 0; i < TEMP_COMP_MAX; i++) {
				isp_gen->temp_info.temperature_param[i] = isp_gen->isp_ini_cfg.isp_tunning_settings.sensor_temp[13*TEMP_COMP_MAX + i];
			}
		} else {
			for (i = 0; i < 13; i++) {
				index = i;
				if (isp_gen->sensor_info.temperature <= temp_mapping[i+1]) {
					break;
				}
			}
			temperature = clamp(isp_gen->sensor_info.temperature, temp_mapping[0], temp_mapping[13]);
			for (i = 0; i < TEMP_COMP_MAX; i++) {
				isp_gen->temp_info.temperature_param[i] = ValueInterp(temperature, temp_mapping[index], temp_mapping[index+1],
					isp_gen->isp_ini_cfg.isp_tunning_settings.sensor_temp[index*TEMP_COMP_MAX + i], isp_gen->isp_ini_cfg.isp_tunning_settings.sensor_temp[(index+1)*TEMP_COMP_MAX + i]);
			}
		}
#if (ISP_VERSION >= 602)
		ISP_LIB_LOG(ISP_LOG_ISO, "sensor temperature=%d.  temperature_param = %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", isp_gen->sensor_info.temperature,
					isp_gen->temp_info.temperature_param[TEMP_COMP_2D_BLACK], isp_gen->temp_info.temperature_param[TEMP_COMP_2D_WHITE],
					isp_gen->temp_info.temperature_param[TEMP_COMP_3D_BLACK], isp_gen->temp_info.temperature_param[TEMP_COMP_3D_WHITE],
					isp_gen->temp_info.temperature_param[TEMP_COMP_NS_LW], isp_gen->temp_info.temperature_param[TEMP_COMP_NS_HI],
					isp_gen->temp_info.temperature_param[TEMP_COMP_DTC_STREN], isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_R],
					isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_G], isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_B],
					isp_gen->temp_info.temperature_param[TEMP_COMP_SHARP], isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_LOW],
					isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_MID], isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_HIGH]);
#else
		ISP_LIB_LOG(ISP_LOG_ISO, "sensor temperature=%d.  temperature_param = %d %d %d %d %d %d %d %d %d %d %d %d\n", isp_gen->sensor_info.temperature,
					isp_gen->temp_info.temperature_param[TEMP_COMP_2D_BLACK], isp_gen->temp_info.temperature_param[TEMP_COMP_2D_WHITE],
					isp_gen->temp_info.temperature_param[TEMP_COMP_3D_BLACK], isp_gen->temp_info.temperature_param[TEMP_COMP_3D_WHITE],
					isp_gen->temp_info.temperature_param[TEMP_COMP_DTC_STREN], isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_R],
					isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_G], isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_B],
					isp_gen->temp_info.temperature_param[TEMP_COMP_SHARP], isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_LOW],
					isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_MID], isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_HIGH]);
#endif
	} else {
		isp_gen->temp_info.temperature_param[TEMP_COMP_2D_BLACK] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_2D_WHITE] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_3D_BLACK] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_3D_WHITE] = 256;
#if (ISP_VERSION >= 602)
		isp_gen->temp_info.temperature_param[TEMP_COMP_NS_LW] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_NS_HI] = 256;
#endif
		isp_gen->temp_info.temperature_param[TEMP_COMP_DTC_STREN] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_R] = 0;
		isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_G] = 0;
		isp_gen->temp_info.temperature_param[TEMP_COMP_BLC_B] = 0;
		isp_gen->temp_info.temperature_param[TEMP_COMP_SHARP] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_LOW] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_MID] = 256;
		isp_gen->temp_info.temperature_param[TEMP_COMP_SATU_HIGH] = 256;
	}
}

void __isp_ctx_config(struct isp_lib_context *isp_gen)
{
	__isp_ctx_apply_enable(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_cfg_lib(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_cfg_mod(isp_gen);
	FUNCTION_LOG;

	__get_sensor_temp_param(isp_gen);

	__isp_ctx_update_iso_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_af_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_ae_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_awb_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_gtm_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_pltm_cfg(isp_gen);
	FUNCTION_LOG;

	__isp_ctx_update_afs_cfg(isp_gen);
	FUNCTION_LOG;

#if ISP_LIB_USE_MD
	__isp_ctx_update_md_cfg(isp_gen);
	FUNCTION_LOG;
#endif

	__isp_ctx_update_rolloff_cfg(isp_gen);
	FUNCTION_LOG;
}

void __isp_ctx_update(struct isp_lib_context *isp_gen)
{
	__isp_ctx_apply_enable(isp_gen);

	__isp_ctx_cfg_mod(isp_gen);

	__isp_ctx_update_iso_cfg(isp_gen);

	__isp_ctx_update_af_cfg(isp_gen);

	__isp_ctx_update_ae_cfg(isp_gen);

	__isp_ctx_update_awb_cfg(isp_gen);

	__isp_ctx_update_gtm_cfg(isp_gen);

	__isp_ctx_update_pltm_cfg(isp_gen);

	__isp_ctx_update_afs_cfg(isp_gen);

#if ISP_LIB_USE_MD
	__isp_ctx_update_md_cfg(isp_gen);
#endif

	__isp_ctx_update_rolloff_cfg(isp_gen);
}

int isp_ctx_algo_init(struct isp_lib_context *isp_gen, const struct isp_ctx_operations *ops)
{
	//ISO init
	isp_gen->iso_entity_ctx.iso_entity = iso_init(&isp_gen->iso_entity_ctx.ops);
	if (isp_gen->iso_entity_ctx.iso_entity == NULL || NULL == isp_gen->iso_entity_ctx.ops) {
		ISP_ERR("ISO Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->iso_entity_ctx.ops->isp_iso_get_params(isp_gen->iso_entity_ctx.iso_entity, &isp_gen->iso_entity_ctx.iso_param);
		isp_gen->iso_entity_ctx.iso_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}
#if ISP_LIB_USE_AF
	//AF init
	isp_gen->af_entity_ctx.af_entity = af_init(&isp_gen->af_entity_ctx.ops);
	if (isp_gen->af_entity_ctx.af_entity == NULL ||	NULL == isp_gen->af_entity_ctx.ops) {
		ISP_ERR("AF Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->af_entity_ctx.ops->isp_af_get_params(isp_gen->af_entity_ctx.af_entity, &isp_gen->af_entity_ctx.af_param);
		isp_gen->af_entity_ctx.af_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}
#endif
	//AFS init
	isp_gen->afs_entity_ctx.afs_entity = afs_init(&isp_gen->afs_entity_ctx.ops);
	if (isp_gen->afs_entity_ctx.afs_entity == NULL || NULL == isp_gen->afs_entity_ctx.ops) {
		ISP_ERR("AFS Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->afs_entity_ctx.ops->isp_afs_get_params(isp_gen->afs_entity_ctx.afs_entity, &isp_gen->afs_entity_ctx.afs_param);
		isp_gen->afs_entity_ctx.afs_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}

#if ISP_LIB_USE_MD
	//MD init
	isp_gen->md_entity_ctx.md_entity = md_init(&isp_gen->md_entity_ctx.ops);
	if (isp_gen->md_entity_ctx.md_entity == NULL ||	NULL == isp_gen->md_entity_ctx.ops) {
		ISP_ERR("MD Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->md_entity_ctx.ops->isp_md_get_params(isp_gen->md_entity_ctx.md_entity, &isp_gen->md_entity_ctx.md_param);
		isp_gen->md_entity_ctx.md_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}
#endif

	//AWB init
	isp_gen->awb_entity_ctx.awb_entity = awb_init(&isp_gen->awb_entity_ctx.ops);
	if (isp_gen->awb_entity_ctx.awb_entity == NULL || NULL == isp_gen->awb_entity_ctx.ops) {
		ISP_ERR("AWB Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->awb_entity_ctx.ops->isp_awb_get_params(isp_gen->awb_entity_ctx.awb_entity, &isp_gen->awb_entity_ctx.awb_param);
		isp_gen->awb_entity_ctx.awb_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}

	//AE init
	isp_gen->ae_entity_ctx.ae_entity = ae_init(&isp_gen->ae_entity_ctx.ops);
	if (isp_gen->ae_entity_ctx.ae_entity == NULL ||	NULL == isp_gen->ae_entity_ctx.ops) {
		ISP_ERR("AE Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->ae_entity_ctx.ops->isp_ae_get_params(isp_gen->ae_entity_ctx.ae_entity, &isp_gen->ae_entity_ctx.ae_param);
		isp_gen->ae_entity_ctx.ae_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}

	//GTM init
	isp_gen->gtm_entity_ctx.gtm_entity = gtm_init(&isp_gen->gtm_entity_ctx.ops);
	if (isp_gen->gtm_entity_ctx.gtm_entity == NULL || NULL == isp_gen->gtm_entity_ctx.ops) {
		ISP_ERR("GTM Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->gtm_entity_ctx.ops->isp_gtm_get_params(isp_gen->gtm_entity_ctx.gtm_entity, &isp_gen->gtm_entity_ctx.gtm_param);
		isp_gen->gtm_entity_ctx.gtm_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}
	//PLTM init
	isp_gen->pltm_entity_ctx.pltm_entity = pltm_init(&isp_gen->pltm_entity_ctx.ops);
	if (isp_gen->pltm_entity_ctx.pltm_entity == NULL || NULL == isp_gen->pltm_entity_ctx.ops) {
		ISP_ERR("PLTM Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->pltm_entity_ctx.ops->isp_pltm_get_params(isp_gen->pltm_entity_ctx.pltm_entity, &isp_gen->pltm_entity_ctx.pltm_param);
		isp_gen->pltm_entity_ctx.pltm_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}

#if (ISP_VERSION == 600)
	isp_gen->module_cfg.module_entity = isp_module_init();
	if (isp_gen->module_cfg.module_entity == NULL) {
		ISP_ERR("isp module Entity is BUSY or NULL!\n");
		return -1;
	}
#endif

#if ISP_LIB_USE_ROLLOFF
	//ROLLOFF init
	isp_gen->rolloff_entity_ctx.rolloff_entity = rolloff_init(&isp_gen->rolloff_entity_ctx.ops);
	if (isp_gen->rolloff_entity_ctx.rolloff_entity == NULL || NULL == isp_gen->rolloff_entity_ctx.ops) {
		ISP_ERR("ROLLOFF Entity is BUSY or NULL!\n");
		return -1;
	} else {
		isp_gen->rolloff_entity_ctx.ops->isp_rolloff_get_params(isp_gen->rolloff_entity_ctx.rolloff_entity, &isp_gen->rolloff_entity_ctx.rolloff_param);
		isp_gen->rolloff_entity_ctx.rolloff_param->isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	}
#endif
	isp_gen->ops = ops;

	isp_gen->af_frame_cnt  = 0;
	isp_gen->ae_frame_cnt  = 0;
	isp_gen->awb_frame_cnt = 0;
	isp_gen->gtm_frame_cnt = 0;

	isp_gen->md_frame_cnt  = 0;
	isp_gen->afs_frame_cnt  = 0;
	isp_gen->iso_frame_cnt = 0;
	isp_gen->rolloff_frame_cnt = 0;
	isp_gen->alg_frame_cnt = 0;

	if (__isp_alloc_reg_tbl(isp_gen))
		return -1;

	pthread_mutex_init(&(isp_gen->ctx_lock), NULL);
	return 0;
}

HW_S32 isp_ctx_front_algo_run(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	isp_get_saved_regs(isp_gen);

#if ISP_AI_SCENE_CONF
	__isp_set_scene_param(isp_gen);
#endif

	isp_apply_settings(isp_gen);

	__isp_ae_set_params(isp_gen);
	__isp_ae_run(isp_gen);

#if ISP_LIB_USE_AF
	__isp_af_set_params(isp_gen);
	__isp_af_run(isp_gen);
#endif

	isp_gen->ae_frame_cnt++;
	isp_gen->af_frame_cnt++;
	FUNCTION_LOG;
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	FUNCTION_LOG;
	return 0;
}

HW_S32 isp_ctx_rear_algo_run(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	__get_sensor_temp_param(isp_gen);

	__isp_iso_set_params(isp_gen);
	__isp_iso_run(isp_gen);

	if (isp_gen->isp_ini_cfg.isp_test_settings.gtm_en) {
		__isp_gtm_set_params(isp_gen);
	}
	__isp_gtm_run(isp_gen);

	if (isp_gen->isp_ini_cfg.isp_test_settings.pltm_en) {
		__isp_pltm_set_params(isp_gen);
		__isp_pltm_run(isp_gen);
	}
	if (isp_gen->isp_ini_cfg.isp_test_settings.afs_en) {
		__isp_afs_set_params(isp_gen);
		__isp_afs_run(isp_gen);
	}
#if ISP_LIB_USE_MD
	if (isp_gen->isp_ini_cfg.isp_test_settings.af_en) {
		__isp_md_set_params(isp_gen);
		__isp_md_run(isp_gen);
	}
#endif

	__isp_awb_set_params(isp_gen);
	__isp_awb_run(isp_gen);

	if (isp_gen->isp_ini_cfg.isp_test_settings.lsc_en)
		config_lens_table(isp_gen, isp_gen->af_entity_ctx.af_result.std_code_output);

	if (isp_gen->isp_ini_cfg.isp_test_settings.msc_en) {
#if ISP_LIB_USE_ROLLOFF
		if (isp_gen->isp_ini_cfg.isp_tunning_settings.mff_mod >= 3) {
			__isp_rolloff_set_params(isp_gen);
			__isp_rolloff_run(isp_gen);
		}
#endif
		config_msc_table(isp_gen, isp_gen->af_entity_ctx.af_result.std_code_output, isp_gen->iso_entity_ctx.iso_result.shading_comp);
		if (isp_gen->isp_ini_cfg.isp_test_settings.denoise_en || isp_gen->isp_ini_cfg.isp_test_settings.tdf_en) {
			config_nr_msc_table(isp_gen, 0);
		}
	}

	isp_apply_colormatrix(isp_gen);

	//isp_gen->module_cfg.cfa_cfg.min_rgb = isp_gen->isp_ini_cfg.isp_test_settings.defog_en ? 1023 : 0;

	FUNCTION_LOG;

	isp_hardware_update(isp_gen);

	isp_gen->awb_frame_cnt++;
	isp_gen->alg_frame_cnt ++;
	isp_gen->gtm_frame_cnt++;
	isp_gen->md_frame_cnt++;
	isp_gen->afs_frame_cnt ++;
	isp_gen->iso_frame_cnt++;
	isp_gen->rolloff_frame_cnt++;
	FUNCTION_LOG;
	//if (isp_gen->awb_frame_cnt % 10 == 0)
	//	__isp_dump_reg(isp_gen);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	FUNCTION_LOG;
	return 0;
}

int isp_ctx_algo_exit(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
#if ISP_LIB_USE_AF
	af_exit(isp_gen->af_entity_ctx.af_entity);
#endif
	afs_exit(isp_gen->afs_entity_ctx.afs_entity);
#if ISP_LIB_USE_MD
	md_exit(isp_gen->md_entity_ctx.md_entity);
#endif
	awb_exit(isp_gen->awb_entity_ctx.awb_entity);
	ae_exit(isp_gen->ae_entity_ctx.ae_entity);
	gtm_exit(isp_gen->gtm_entity_ctx.gtm_entity);
	pltm_exit(isp_gen->pltm_entity_ctx.pltm_entity);
	iso_exit(isp_gen->iso_entity_ctx.iso_entity);
#if (ISP_VERSION == 600)
	isp_module_exit(isp_gen->module_cfg.module_entity);
#endif
#if ISP_LIB_USE_ROLLOFF
	rolloff_exit(isp_gen->rolloff_entity_ctx.rolloff_entity);
#endif
	if(isp_gen->load_reg_base != NULL) {
		free(isp_gen->load_reg_base);
		isp_gen->load_reg_base = NULL;
	}
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	pthread_mutex_destroy(&(isp_gen->ctx_lock));

	return 0;
}

HW_S32 isp_ctx_update_ae_tbl(struct isp_lib_context *isp_gen, int sensor_fps)
{
	int i = 0;

	pthread_mutex_lock(&(isp_gen->ctx_lock));

	for(i = 0; i < isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length; i++) {
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[i].max_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[i].max_exp = sensor_fps;
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[i].min_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[i].min_exp = sensor_fps;
	}
	for(i = 0; i < isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length; i++) {
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[i].max_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[i].max_exp = sensor_fps;
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[i].min_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[i].min_exp = sensor_fps;
	}
	for(i = 0; i < isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].length; i++) {
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[i].max_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[i].max_exp = sensor_fps;
		if(isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[i].min_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param->ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[i].min_exp = sensor_fps;
	}
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_INI_DATA);
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);

	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}

#if ISP_AI_SCENE_CONF
HW_S32 isp_ctx_set_scene(struct isp_lib_context *isp_gen, scene_mode_t scene_mode)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	if(scene_mode != isp_gen->ai_scene.cur_scene) {
		isp_gen->ai_scene.cur_scene = scene_mode;
		isp_gen->ai_scene.scene_change_flag = 1;
		isp_gen->ai_scene.scene_frame_cnt = 0;
	}

	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}
#endif

int isp_ctx_config_update(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	__isp_ctx_update(isp_gen);

	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}

int isp_ctx_config_reset(struct isp_lib_context *isp_gen)
{
	isp_ctx_config_init(isp_gen);
	return 0;
}

int isp_ctx_config_init(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
	if (isp_gen->sensor_info.sensor_height == 0) {
		pthread_mutex_unlock(&(isp_gen->ctx_lock));
		ISP_ERR("sensor attribute is not init.\n");
		return -1;
	}
	FUNCTION_LOG;

	__isp_ctx_config(isp_gen);

	//for isp_apply_settings
	isp_gen->isp_3a_change_flags = 0xffffffff;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_BRIGHTNESS;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_CONTRAST;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_GAIN_STR;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_SCENE_MODE;

	if (isp_gen->isp_ir_flag == ISP_IR_MODE) {
		isp_gen->isp_3a_change_flags &= ~ISP_SET_HUE;
		isp_gen->tune.effect = ISP_COLORFX_GRAY;
	}

	isp_apply_settings(isp_gen);

	isp_hardware_update(isp_gen);

	memset(&isp_gen->defog_ctx.min_rgb_pre[0], 0, 8*sizeof(int));
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	FUNCTION_LOG;

	return 0;
}

HW_S32 isp_ctx_config_exit(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	return 0;
}

