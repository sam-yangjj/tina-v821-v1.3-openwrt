//#include <netinet/in.h>
#include <arpa/inet.h>

#include "../log_handle.h"
#include "../socket_protocol.h"
#include "../../include/device/isp_dev.h"

#include "isp_handle.h"
#include "capture_image.h"
#include "server_core.h"

enum _isp_flag_e {
	ISP_MODULE_INIT_NOT      = 0,
	ISP_MODULE_INIT_YES      = 1,
};
enum _isp_status_e {
	ISP_IDLE                 = 0,
	ISP_RUNNING              = 1,
};

#define VALID_ISP_SEL(id) \
	((id) >= 0 && (id) < HW_ISP_DEVICE_NUM)


int g_module_init = ISP_MODULE_INIT_NOT;
int g_cur_isp_sel = -1;
int g_isp_status[HW_ISP_DEVICE_NUM];

void get_isp_version(char* version)
{
	isp_get_version(version);
}

int get_isp_id()
{
	return g_cur_isp_sel;
}


int init_isp_module(int isp_id)
{
	int ret = -1, i = 0;
	static int id = 0;

	if (ISP_MODULE_INIT_YES == g_module_init && id == isp_id) {
		isp_stop(isp_id);
		exit_isp_module();
	}

	ret = media_dev_init();
	if (ret < 0) {
		LOG("%s: failed to init media dev\n", __FUNCTION__);
	} else {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			g_isp_status[i] = ISP_IDLE;
		}
		g_module_init = ISP_MODULE_INIT_YES;
		id = isp_id;
	}

	return ret;
}

int exit_isp_module()
{
	int i = 0;
	if (ISP_MODULE_INIT_YES == g_module_init) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (ISP_RUNNING == g_isp_status[i]) {
				LOG("%s: ready to exit isp %d\n", __FUNCTION__, i);
				isp_exit(i);
				g_isp_status[i] = ISP_IDLE;
			}
		}

		media_dev_exit();
		g_cur_isp_sel = -1;
		g_module_init = ISP_MODULE_INIT_NOT;
	}
	LOG("%s: exits\n", __FUNCTION__);
	return 0;
}

int select_isp(int id, int init_flag)
{
	int ret = -1;

	//LOG("%s: isp - %d\n", __FUNCTION__, id);
	if (VALID_ISP_SEL(id)) {
		//if (ISP_MODULE_INIT_YES != g_module_init) {  // init module first
		if (init_flag) {
			ret = init_isp_module(id);
			if (ret < 0) {
				LOG("%s: failed to init isp module\n", __FUNCTION__);
				return -1;
			}
		}

		if (get_vich_status() == 0) {
			LOG("%s: !!!!!!! NO VICH OPENED !!!!!!!\n", __FUNCTION__);
			return -1;
		}

		if (ISP_IDLE == g_isp_status[id]) { // not running
			ret = isp_init(id);
			if (ret) {
				LOG("%s: failed to init isp - %d, %d\n", __FUNCTION__, id, ret);
				return -1;
			}
			ret = isp_run(id);
			if (ret) {
				isp_exit(id);
				LOG("%s: failed to run isp - %d\n", __FUNCTION__, id);
				return -1;
			}
			g_cur_isp_sel = id;
			g_isp_status[id] = ISP_RUNNING;
			ret = 0;
		} else {
			g_cur_isp_sel = id;
			ret = 0;
		}
		//LOG("%s: select isp %d\n", __FUNCTION__, id);
	} else {
		LOG("%s: invalid isp sel - %d\n", __FUNCTION__, id);
	}
	return ret;
}

int select_isp_stitch_mode(int isp_id, enum stitch_mode_t stitch_mode)
{
	int ret = 0;

	isp_set_stitch_mode(isp_id, stitch_mode);
	switch (stitch_mode)
	{
	case STITCH_2IN1_LINNER:
		if (isp_id % 2 == 0)
			ret = isp_set_sync((1<<(isp_id)) | (1<<(isp_id + 1)));
		else
			ret = isp_set_sync((1<<(isp_id - 1)) | (1<<(isp_id)));
		break;

	default:
		break;
	}
	return ret;
}

#if (DO_LOG_EN && 0)
#define DD(fmt, ...)         printf("####### "fmt, ##__VA_ARGS__);
#else
#define DD(fmt, ...)
#endif

typedef HW_S32 (*convert_long_func)(HW_S32);
typedef HW_S16 (*convert_short_func)(HW_S16);

HW_S32 hton_long(HW_S32 ret)
{
	HW_S32 ret0 = (HW_S32)htonl(ret);
	DD("%d-%08x-%08x ", ret, ret, ret0);
	return ret0;
}
HW_S32 ntoh_long(HW_S32 ret)
{
	HW_S32 ret0 = (HW_S32)ntohl(ret);
	DD("%d-%08x-%08x ", ret0, ret0, ret);
	return ret0;
}
HW_S16 hton_short(HW_S16 ret)
{
	HW_S16 ret0 = (HW_S16)htons(ret);
	DD("%d-%04x-%04x ", ret, ret, ret0);
	return ret0;
}
HW_S16 ntoh_short(HW_S16 ret)
{
	HW_S16 ret0 = (HW_S16)ntohs(ret);
	DD("%d-%04x-%04x ", ret0, ret0, ret);
	return ret0;
}

void convert_tuning_cfg_func(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data,
	convert_long_func cvt_long, convert_short_func cvt_short)
{
	HW_S32 *int_ptr = NULL;
	HW_S16 *short_ptr = NULL;
	HW_U16 *ushort_ptr = NULL;
	HW_U8  *uchar_ptr = NULL;
	HW_S8  *char_ptr = NULL;
	int attr_len = 0, i = 0;

	if (!cfg_data || !cvt_long || !cvt_short) {
		return ;
	}

	DD("%s starts\n", __FUNCTION__);
	switch (group_id) {
	case HW_ISP_CFG_TEST:
		if (cfg_ids & HW_ISP_CFG_TEST_PUB) {
			DD("test pub: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_EXPTIME) {
			DD("test exp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_GAIN) {
			DD("test gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FOCUS) {
			DD("test focus: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FORCED) {
			DD("test forced: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_forced_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_forced_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_ENABLE) {
			DD("test enable: ");
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_test_enable_cfg) / sizeof(HW_S8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_enable_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_SPECIAL_CTRL)
		{
			DD("special ctrl: ");
			char_ptr = (HW_S8 *)cfg_data;
			attr_len = sizeof(struct isp_test_special_ctrl_cfg) / sizeof(HW_S8);
			for (i = 0; i < attr_len; i++, char_ptr++) {
				DD("%d ", *char_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_special_ctrl_cfg);
		}
		break;
	case HW_ISP_CFG_3A:
		if (cfg_ids & HW_ISP_CFG_AE_PUB) {
			DD("ae pub: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_PREVIEW_TBL) {
			DD("ae preview tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_CAPTURE_TBL) {
			DD("ae capture tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_VIDEO_TBL) {
			DD("ae video tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_WIN_WEIGHT) {
			DD("ae win weight: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_weight_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_weight_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_DELAY) {
			DD("ae delay: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_delay_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_delay_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_PUB) {
			DD("awb speed: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_pub_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_TEMP_RANGE) {
			DD("awb temp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_range_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_range_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_DIST) {
			DD("awb dist: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_dist_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_dist_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_LIGHT_INFO) {
			DD("awb light: ");
			int_ptr = (HW_S32 *)cfg_data;
#if (ISP_VERSION >= 602)
			attr_len = sizeof(struct isp_awb_light_info_cfg) / sizeof(HW_S32);
#else
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
#endif
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
#if (ISP_VERSION >= 602)
			cfg_data += sizeof(struct isp_awb_light_info_cfg);
#else
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
#endif
		}
		if (cfg_ids & HW_ISP_CFG_AWB_EXT_LIGHT_INFO) {
			DD("awb ext: ");
			int_ptr = (HW_S32 *)cfg_data;
#if (ISP_VERSION >= 602)
			attr_len = sizeof(struct isp_awb_ext_light_info_cfg) / sizeof(HW_S32);
#else
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
#endif
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
#if (ISP_VERSION >= 602)
			cfg_data += sizeof(struct isp_awb_ext_light_info_cfg);
#else
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
#endif
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SKIN_INFO) {
			DD("awb skin: ");
			int_ptr = (HW_S32 *)cfg_data;
#if (ISP_VERSION >= 602)
			attr_len = sizeof(struct isp_awb_skin_color_info_cfg) / sizeof(HW_S32);
#else
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
#endif
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
#if (ISP_VERSION >= 602)
			cfg_data += sizeof(struct isp_awb_skin_color_info_cfg);
#else
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
#endif
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SPECIAL_INFO) {
			DD("awb special: ");
			int_ptr = (HW_S32 *)cfg_data;
#if (ISP_VERSION >= 602)
			attr_len = sizeof(struct isp_awb_special_color_info_cfg) / sizeof(HW_S32);
#else
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
#endif
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
#if (ISP_VERSION >= 602)
			cfg_data += sizeof(struct isp_awb_special_color_info_cfg);
#else
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
#endif
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PRESET_GAIN) {
			DD("awb preset: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_preset_gain_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_preset_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_FAVOR) {
			DD("awb favor: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_favor_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_favor_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_VCM_CODE) {
			DD("af vcm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_vcm_code_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_vcm_code_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_OTP) {
			DD("af otp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_otp_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_otp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SPEED) {
			DD("af speed: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_speed_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_speed_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_FINE_SEARCH) {
			DD("af fine search: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_fine_search_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_fine_search_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_REFOCUS) {
			DD("af refocus: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_refocus_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_refocus_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_TOLERANCE) {
			DD("af tolerance: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_tolerance_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_tolerance_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SCENE) {
			DD("af scene: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_scene_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_scene_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_WDR_SPLIT) {
			DD("wdr split: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_wdr_split_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_wdr_split_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_WDR_STITCH) {
			DD("wdr stitch: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_wdr_comm_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_wdr_comm_cfg);
		}
		break;
	case HW_ISP_CFG_TUNING:
		if (cfg_ids & HW_ISP_CFG_TUNING_FLASH) {
			DD("tuning flash: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_flash_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_flash_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_FLICKER) {
			DD("tuning flicker: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_flicker_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_flicker_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_VISUAL_ANGLE) {
			DD("tuning visual: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_visual_angle_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_visual_angle_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_GTM) {
			DD("tuning gtm: ");
			int_ptr = (HW_S32 *)cfg_data;
			// 1. hist_sel, type, gamma_type, auto_alpha_en, hist_pix_cnt, dark_minval, bright_minval
			for (i = 0; i < 7; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 2. plum_var
			short_ptr = (HW_S16 *)int_ptr;
			attr_len = 9*9;
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gtm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CFA) {
			DD("tuning cfa: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cfa_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cfa_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CTC) {
			DD("tuning ctc: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_ctc_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ctc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DIGITAL_GAIN) {
			DD("tuning digital gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_blc_gain_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_blc_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_LOW) {
			DD("tuning ccm low: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_MID) {
			DD("tuning ccm mid: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_HIGH) {
			DD("tuning ccm high: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM) {
			DD("tuning pltm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_pltm_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_pltm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_GCA) {
			DD("tuning gca: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_gca_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gca_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_BDNF_COMM) {
			DD("tuning bdnf comm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_bdnf_comm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_bdnf_comm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_TDNF_COMM) {
			DD("tuning tdnf comm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_tdnf_comm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_tdnf_comm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SHARP_COMM) {
			DD("tuning sharp comm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_sharp_comm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_sharp_comm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DPC) {
			DD("tuning dpc: ");
			char_ptr = (HW_S8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_dpc_cfg) / sizeof(HW_S8);
			for (i = 0; i < attr_len; i++, char_ptr++) {
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_dpc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_ENCPP_SHARP_COMM) {
			DD("tuning sharp comm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_encpp_sharp_comm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_encpp_sharp_comm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SENSOR) {
			DD("tuning sensor temp: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_sensor_temp_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_sensor_temp_cfg);
		}
		break;
	case HW_ISP_CFG_TUNING_TABLES:
		if (cfg_ids & HW_ISP_CFG_TUNING_LSC) {
			DD("tuning lsc: ");
			// 1. ff_mod, center, rolloff, lsc_mode
			int_ptr = (HW_S32 *)cfg_data;
			for (i = 0; i < 5; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 2. value
			ushort_ptr = (HW_U16 *)int_ptr;
			attr_len = (ISP_LSC_TEMP_NUM + ISP_LSC_TEMP_NUM) * ISP_LSC_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			// 3. temp.
			attr_len = ISP_LSC_TEMP_NUM;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_lsc_table_cfg);
		}

		if (cfg_ids & HW_ISP_CFG_TUNING_GAMMA) {
			DD("tuning gamma: ");
			// 1. number
			int_ptr = (HW_S32 *)cfg_data;
			*int_ptr = cvt_long(*int_ptr);
			// 2. value
			ushort_ptr = (HW_U16 *)(cfg_data + sizeof(HW_S32));
			attr_len = ISP_GAMMA_TRIGGER_POINTS * ISP_GAMMA_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			// 3. trigger
			for (i = 0; i < ISP_GAMMA_TRIGGER_POINTS; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gamma_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_BDNF) {
			DD("tuning bdnf: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_bdnf_table_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_bdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_TDNF) {
			DD("tuning tdnf: ");
			// 1. thres
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = ISP_REG_TBL_LENGTH * 14;
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
				}
			// 2. k, diff
			uchar_ptr = (HW_U8 *)short_ptr;
			attr_len = ISP_REG1_TBL_LENGTH * 10;
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_tdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SHARP) {
			DD("tuning sharp: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_sharp_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_sharp_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM) {
			DD("tuning cem: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cem_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cem_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM_1) {
			DD("tuning cem_1: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cem_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cem_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM_TBL) {
			DD("tuning pltm table: ");
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = PLTM_MAX_STAGE * (15 + ISP_REG_TBL_LENGTH + 128);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			ushort_ptr = (HW_U16 *)uchar_ptr;
			attr_len = 512;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_pltm_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_WDR) {
			DD("tuning wdr table: ");
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_wdr_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_wdr_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_LCA_TBL){
			DD("tuning lca: ");
			// 1. isp_tuning_lca_pf_satu_lut
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_lca_pf_satu_lut) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			cfg_data += sizeof(struct isp_tuning_lca_pf_satu_lut);
			// 2. isp_tuning_lca_gf_satu_lut
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_lca_gf_satu_lut) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			DD("\n");
			/* offset */
			//cfg_data += sizeof(struct isp_tuning_lca_pf_satu_lut);
			/* offset */
			cfg_data += sizeof(struct isp_tuning_lca_gf_satu_lut);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_MSC) {
			DD("tuning msc: ");
			// 1. mff_mod, msc_mode
			char_ptr = (HW_S8 *)cfg_data;
			for (i = 0; i < 2; i++, char_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			// 2. msc_blw_lut, msc_blh_lut
			short_ptr = (HW_S16 *)char_ptr;
			attr_len = ISP_MSC_TBL_LUT_SIZE + ISP_MSC_TBL_LUT_SIZE;
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			// 3. value
			ushort_ptr = (HW_U16 *)short_ptr;
			attr_len = (ISP_MSC_TEMP_NUM + ISP_MSC_TEMP_NUM) * ISP_MSC_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			// 4. temp.
			for (i = 0; i < ISP_MSC_TEMP_NUM; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}

			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_msc_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_ENCPP_SHARP) {
			DD("tuning sharp: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_sharp_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_sharp_table_cfg);
		}
		break;
	case HW_ISP_CFG_DYNAMIC:
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LUM_POINT) {
			DD("dynamic lum: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_single_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GAIN_POINT) {
			DD("dynamic gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_single_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SHARP) {
			DD("dynamic sharp: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_sharp_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_sharp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DENOISE) {
			DD("dynamic denoise: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_denoise_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_denoise_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_BLACK_LV) {
			DD("dynamic blc: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_black_level_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_black_level_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DPC) {
			DD("dynamic dpc: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_dpc_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_dpc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_PLTM) {
			DD("dynamic pltm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_pltm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_pltm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DEFOG) {
			DD("dynamic defog: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_defog_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_defog_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_HISTOGRAM) {
			DD("dynamic histogram: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_histogram_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_histogram_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CEM) {
			DD("dynamic cem: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_cem_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_cem_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_TDF) {
			DD("dynamic tdf: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_tdf_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_tdf_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_AE) {
			DD("dynamic ae: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_ae_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_ae_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GTM) {
			DD("dynamic gtm: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_gtm_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_gtm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LCA) {
			DD("dynamic lca: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_lca_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_lca_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CFA) {
			DD("dynamic cfa: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_cfa_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_cfa_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_ENCPP_SHARP) {
			DD("dynamic encpp sharp: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_encpp_sharp_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_encpp_sharp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_ENCODER_DENOISE) {
			DD("dynamic encoder denoise: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_encoder_denoise_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_encoder_denoise_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_WDR) {
			DD("dynamic wdr: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_wdr_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_wdr_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SHADING) {
			DD("dynamic shading: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_shading_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_shading_cfg);
		}
#if (ISP_VERSION >= 602)
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_AWB) {
			DD("dynamic awb: ");
			short_ptr = (HW_S16 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_awb_cfg) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_awb_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_TUNING_ENCODER:
		if (cfg_ids & HW_VENCODER_CFG_TUNING_BASE)
		{
			DD("tuning vencoder base : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_base_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_base_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_CONSTRAINTFLAG)
		{
			DD("tuning vencoder ConstraintFlag : ");
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(vencoder_h264ConstraintFlag_cfg_t) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_h264ConstraintFlag_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_SAVEBSFILE)
		{
			DD("tuning vencoder savebsfile cfg: ");
			uchar_ptr = (HW_U8 *)cfg_data;
			for (i = 0; i < VENCODER_FILENAME_LEN; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}

			int_ptr = (HW_S32 *)uchar_ptr;
			for (i = 0; i < 3; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_savebsfile_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_PROC)
		{
			DD("tuning vencoder proc : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_proc_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_proc_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_JPEGQUALITY)
		{
			DD("tuning vencoder jpeg quality : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_jpeg_quality_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_jpeg_quality_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_QPCONTROL)
		{
			DD("tuning vencoder QP control : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_QPcontrol_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_QPcontrol_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_VBR)
		{
			DD("tuning vencoder vbr : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_vbr_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_vbr_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_D3D)
		{
			DD("tuning vencoder d3d : ");
			uchar_ptr = (HW_U8 *)cfg_data;
			for (i = 0; i < 16; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
			int_ptr = (HW_S32 *)uchar_ptr;
			for (i = 0; i < 1; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_d3d_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_RIGIOND3D)
		{
			DD("tuning vencoder rigion d3d : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_region_d3d_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_region_d3d_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_VE2ISP)
		{
			DD("tuning vencoder ve2isp : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_ve2isp_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_ve2isp_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_SUPERFRAME)
		{
			DD("tuning vencoder super frame : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_super_frame_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_super_frame_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_ROI)
		{
			DD("tuning vencoder roi : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_roi_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_roi_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_MOTIONSEARCH)
		{
			DD("tuning vencoder motion search : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_motion_search_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_motion_search_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_H264VUITIMING)
		{
			DD("tuning vencoder h264 vuitiming : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_h264VuiTiming_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_h264VuiTiming_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_H265VUITIMING)
		{
			DD("tuning vencoder h265 vuitiming : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_h265VuiTiming_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_h265VuiTiming_cfg_t);
		}
		if (cfg_ids & HW_VENCODER_CFG_TUNING_SPECIAL)
		{
			DD("tuning vencoder special : ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(vencoder_special_cfg_t) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(vencoder_special_cfg_t);
		}
		break;
	default:
		break;
	}
}
void convert_tuning_cfg_to_local(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data)
{
	convert_tuning_cfg_func(group_id, cfg_ids, cfg_data, &ntoh_long, &ntoh_short);
}
void convert_tuning_cfg_to_network(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data)
{
	convert_tuning_cfg_func(group_id, cfg_ids, cfg_data, &hton_long, &hton_short);
}

void output_3a_info(const void *stat_info, int type)
{
#if 1
	if (stat_info) {
		FILE *fp = NULL;
		char *buf = (char *)malloc(1<<20);  // 1M
		int index = 0;
		int i = 0, j = 0;
		if (SOCK_CMD_STAT_AE == type) {
			const struct isp_ae_stats_s *ae_info = (const struct isp_ae_stats_s *)stat_info;
			index += sprintf(&buf[index], "ae stat: win_pix_n %u\n", ae_info->win_pix_n);
			// avg
			index += sprintf(&buf[index], "-------------- avg ----------------\n");
			for (i = 0; i < ISP_AE_ROW; i++) {
				for (j = 0; j < ISP_AE_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->avg[i * ISP_AE_COL + j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// hist
			index += sprintf(&buf[index], "-------------- hist ----------------\n");
			for (i = 0; i < 16; i++) {
				for (j = 0; j < 16; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->hist[i * 16 + j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			const struct isp_awb_stats_s *awb_info = (const struct isp_awb_stats_s *)stat_info;
			index += sprintf(&buf[index], "awb stat\n");
			// awb_avg_r
			index += sprintf(&buf[index], "-------------- awb_avg_r ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_r[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg_g
			index += sprintf(&buf[index], "-------------- awb_avg_g ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_g[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg_b
			index += sprintf(&buf[index], "-------------- awb_avg_b ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_b[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg
			index += sprintf(&buf[index], "-------------- awb_avg_b ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->avg[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			const struct isp_af_stats_s *af_info = (const struct isp_af_stats_s *)stat_info;
			index += sprintf(&buf[index], "af stat:\n");
			// af_count
			index += sprintf(&buf[index], "-------------- af_count ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_count[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_h_d1
			index += sprintf(&buf[index], "-------------- af_h_d1 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_h_d1[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_h_d2
			index += sprintf(&buf[index], "-------------- af_h_d2 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_h_d2[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_v_d1
			index += sprintf(&buf[index], "-------------- af_v_d1 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_v_d1[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_v_d2
			index += sprintf(&buf[index], "-------------- af_v_d2 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_v_d2[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		}

		free(buf);
		buf = NULL;
	}
#endif
}

void hton_3a_info(void *stat_info, int type)
{
	if (stat_info) {
		HW_U64 *ptr_u64 = NULL;
		HW_U32 *ptr_u32 = NULL;
		int i = 0;
		if (SOCK_CMD_STAT_AE == type) {
			struct isp_ae_stats_s *ae_stat = (struct isp_ae_stats_s *)stat_info;
			ae_stat->win_pix_n = htonl(ae_stat->win_pix_n);
			// avg
			ptr_u32 = &ae_stat->avg[0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// hist
			ptr_u32 = &ae_stat->hist[0];
			for (i = 0; i < ISP_HIST_NUM; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// hist 1
			ptr_u32 = &ae_stat->hist1[0];
			for (i = 0; i < ISP_HIST_NUM; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			struct isp_awb_stats_s *awb_stat = (struct isp_awb_stats_s *)stat_info;
			// awb_avg_r
			ptr_u32 = &awb_stat->awb_avg_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg_g
			ptr_u32 = &awb_stat->awb_avg_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg_b
			ptr_u32 = &awb_stat->awb_avg_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg
			ptr_u32 = &awb_stat->avg[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			struct isp_af_stats_s *af_stat = (struct isp_af_stats_s *)stat_info;
			// af_iir
			ptr_u64 = &af_stat->af_iir[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_fir
			ptr_u64 = &af_stat->af_fir[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_iir_cnt
			ptr_u64 = &af_stat->af_iir_cnt[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_fir_cnt
			ptr_u64 = &af_stat->af_fir_cnt[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_hlt_cnt
			ptr_u64 = &af_stat->af_hlt_cnt[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_count
			ptr_u64 = &af_stat->af_count[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_h_d1
			ptr_u64 = &af_stat->af_h_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_h_d2
			ptr_u64 = &af_stat->af_h_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_v_d1
			ptr_u64 = &af_stat->af_v_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_v_d2
			ptr_u64 = &af_stat->af_v_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
		}
	}
}
#if 0
void ntoh_3a_info(void *stat_info, int type)
{
	if (stat_info) {
		HW_U64 *ptr_u64 = NULL;
		HW_U32 *ptr_u32 = NULL;
		int i = 0;
		if (SOCK_CMD_STAT_AE == type) {
			struct isp_ae_stats_s *ae_stat = (struct isp_ae_stats_s *)stat_info;
			ae_stat->win_pix_n = ntohl(ae_stat->win_pix_n);
			// avg
			ptr_u32 = &ae_stat->avg[0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// hist
			ptr_u32 = &ae_stat->hist[0];
			for (i = 0; i < ISP_HIST_NUM; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			struct isp_awb_stats_s *awb_stat = (struct isp_awb_stats_s *)stat_info;
			// awb_avg_r
			ptr_u32 = &awb_stat->awb_avg_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg_g
			ptr_u32 = &awb_stat->awb_avg_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg_b
			ptr_u32 = &awb_stat->awb_avg_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg
			ptr_u32 = &awb_stat->avg[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			struct isp_af_stats_s *af_stat = (struct isp_af_stats_s *)stat_info;
			// af_count
			ptr_u64 = &af_stat->af_count[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_h_d1
			ptr_u64 = &af_stat->af_h_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_h_d2
			ptr_u64 = &af_stat->af_h_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_v_d1
			ptr_u64 = &af_stat->af_v_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_v_d2
			ptr_u64 = &af_stat->af_v_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
		}
	}
}
#endif
HW_S32 ini_tuning_get_cfg(HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	HW_S32 ret = AW_ERR_VI_INVALID_PARA;
	char isp_cfg_path[128], buf[50], rdval = 0;
	FILE *fd = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();

	if (!cfg_data)
		return AW_ERR_VI_INVALID_PARA;

	sprintf(isp_cfg_path, "%sisp_tuning_isp%d", ini_cfg.base_path, ini_cfg.sensor_cfg.isp);
	fd = fopen(isp_cfg_path, "r");
	if (fd) {
		if (fgets(buf, 50, fd) != NULL) {
			rdval = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s:%s open failed\n", __FUNCTION__, isp_cfg_path);
		return AW_ERR_VI_INVALID_DEVID;
	}
	if (!rdval) {
		ISP_ERR("%s:isp%d wait for ISP to finish reading parameters\n", __FUNCTION__, ini_cfg.sensor_cfg.isp);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/cfg", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "r");
		if (fd) {
			fread(ini_cfg.params, sizeof(struct isp_param_config), 1, fd);

			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s:%s open failed\n", __FUNCTION__, isp_cfg_path);
			return AW_ERR_VI_INVALID_DEVID;
		}
		//ir, colorspace
		if ((group_id == HW_ISP_CFG_TEST) && (cfg_ids & HW_ISP_CFG_TEST_SPECIAL_CTRL)) {
			struct isp_test_special_ctrl_cfg *isp_test_ctrl = (struct isp_test_special_ctrl_cfg *)cfg_data;
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/ir", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "r");
			if (fd) {
				if (fgets(buf, 50, fd) != NULL) {
					rdval = atoi(buf);
				}
				fclose(fd);
				fd = NULL;
				isp_test_ctrl->ir_mode = rdval;
			} else {
				ISP_ERR("%s:%s open failed\n", __FUNCTION__, isp_cfg_path);
				return AW_ERR_VI_INVALID_DEVID;
			}
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/colorspace", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "r");
			if (fd) {
				if (fgets(buf, 50, fd) != NULL) {
					rdval = atoi(buf);
				}
				fclose(fd);
				fd = NULL;
				isp_test_ctrl->color_space = rdval;
			} else {
				ISP_ERR("%s:%s open failed\n", __FUNCTION__, isp_cfg_path);
				return AW_ERR_VI_INVALID_DEVID;
			}
		}
	}

	/* fill cfg data */
	ret = isp_tuning_get_cfg_run(NULL, group_id, cfg_ids, ini_cfg.params, cfg_data);

	return ret;
}

HW_S32 ini_tuning_set_cfg(HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	int ret = AW_ERR_VI_INVALID_PARA;
	char isp_cfg_path[128], buf[50], rdval = 0;
	FILE *fd = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();

	if (!cfg_data)
		return AW_ERR_VI_INVALID_PARA;

	sprintf(isp_cfg_path, "%sisp_tuning_isp%d", ini_cfg.base_path, ini_cfg.sensor_cfg.isp);
	fd = fopen(isp_cfg_path, "r");
	if (fd) {
		if (fgets(buf, 50, fd) != NULL) {
			rdval = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s:%s open failed\n", __FUNCTION__, isp_cfg_path);
		return AW_ERR_VI_INVALID_DEVID;
	}

	if (!rdval) {
		ISP_ERR("%s:isp%d wait for ISP to finish reading parameters\n", __FUNCTION__, ini_cfg.sensor_cfg.isp);
	} else {
		/* fill cfg data */
		ret = isp_tuning_set_cfg_run(NULL, group_id, cfg_ids, ini_cfg.params, cfg_data);
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/cfg", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "w+");
		fwrite(ini_cfg.params, sizeof(struct isp_param_config), 1, fd);

		fclose(fd);
		fd = NULL;

		//ir, colorspace
		if ((group_id == HW_ISP_CFG_TEST) && (cfg_ids & HW_ISP_CFG_TEST_SPECIAL_CTRL)) {
			struct isp_test_special_ctrl_cfg *isp_test_ctrl = (struct isp_test_special_ctrl_cfg *)cfg_data;
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/ir", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "w+");
			if (fd) {
				sprintf(isp_cfg_path, "%d", isp_test_ctrl->ir_mode);
				fputs(isp_cfg_path, fd);
				fclose(fd);
				fd = NULL;
			} else {
				ISP_ERR("%s open failed\n", isp_cfg_path);
			}

			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/colorspace", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "w+");
			if (fd) {
				sprintf(isp_cfg_path, "%d", isp_test_ctrl->color_space);
				fputs(isp_cfg_path, fd);
				fclose(fd);
				fd = NULL;
			} else {
				ISP_ERR("%s open failed\n", isp_cfg_path);
			}
		}
	}

	return ret;
}

HW_S32 ini_tuning_update_cfg(void)
{
	char isp_cfg_path[128];
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();
	FILE *fd = NULL;
	sprintf(isp_cfg_path, "%sisp_tuning_isp%d", ini_cfg.base_path, ini_cfg.sensor_cfg.isp);
	fd = fopen(isp_cfg_path, "w+");
	if (fd) {
		fputc('0', fd);
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
	}
	return 0;
}

HW_S32 ini_tuning_get_frame(capture_format *cap_fmt, sock_packet *comm_packet, int sock_fd)
{
	char isp_cfg_path[128], buf[50];
	int i, ret, cnt = 0, pic_size = 0, size_act = 0, width_act = 0, height_act = 0, format_act = 0, define_path, thread_status;
	FILE *fd = NULL;
	//unsigned char *uptr = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();
	memset(isp_cfg_path, 0, sizeof(isp_cfg_path));
	memset(buf, 0, sizeof(buf));
	thread_status = CheckThreadsStatus();

	if (strcmp(ini_cfg.base_path, ini_cfg.capture_path) == 0) {
		define_path = 0;
	} else {
		define_path = 1;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	if (access(isp_cfg_path, F_OK) != 0) {
#ifndef ANDROID_TUNING
		sprintf(isp_cfg_path, "mkdir %sisp%d_%d_%d_%d_%d/capture/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		ret = system(isp_cfg_path);
		if (ret) {
			ISP_ERR("system run error!!! (command: %s, ret: %d)\n", isp_cfg_path, ret);
			return CAP_ERR_GET_FRAME;
		}
#else
		ISP_ERR("path %s is invalid\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
#endif
	}
	//sprintf(isp_cfg_path, "touch %sisp%d_%d_%d_%d_%d/capture/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
	//				ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	//system(isp_cfg_path);
	//usleep(1 << 10);

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	cnt = 0;
	if (access(isp_cfg_path, F_OK) != 0) {
		while(1) {
			fd = fopen(isp_cfg_path, "w+");
			if (fd == NULL) {
				cnt++;
			} else {
				break;
			}
			usleep(1 << 10);
			if (cnt > 100) {
				ISP_ERR("cannot open %s, open timeout\n", isp_cfg_path);
				return CAP_ERR_GET_FRAME;
			}
		}
		if (define_path) {
			sprintf(isp_cfg_path, "%s", ini_cfg.capture_path);
		} else {
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		}
		fputs(isp_cfg_path, fd);
		fclose(fd);
		fd = NULL;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	cnt = 0;
	while(1) {
		if (access(isp_cfg_path, F_OK) != 0) {
			break;
		} else {
			if ((thread_status & TH_STATUS_STATISTICS) && cnt > 60) {
				ISP_ERR("cannot get frame, skip!!!\n");
				goto skip_frame;
			}
			cnt++;
		}
		usleep(1 << 10);
		if (cnt > 5000) {
			ISP_ERR("get frame timeout\n");
			return CAP_ERR_GET_FRAME;
		}
	}

	if (define_path) {
		sprintf(isp_cfg_path, "%swidth", ini_cfg.capture_path);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/width", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
						ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	}
	fd = fopen(isp_cfg_path, "r");
	if (fd != NULL) {
		if (fgets(buf, 50, fd) != NULL) {
			width_act = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	if (define_path) {
		sprintf(isp_cfg_path, "%sheight", ini_cfg.capture_path);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/height", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
						ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	}
	fd = fopen(isp_cfg_path, "r");
	if (fd != NULL) {
		if (fgets(buf, 50, fd) != NULL) {
			height_act = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	if (define_path) {
		sprintf(isp_cfg_path, "%sformat", ini_cfg.capture_path);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/format", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
						ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	}
	fd = fopen(isp_cfg_path, "r");
	if (fd != NULL) {
		if (fgets(buf, 50, fd) != NULL) {
			format_act = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	if (format_act == V4L2_PIX_FMT_NV12)
		format_act = V4L2_PIX_FMT_NV12M;
	else if (format_act == V4L2_PIX_FMT_NV21)
		format_act = V4L2_PIX_FMT_NV21M;

	if (format_act != cap_fmt->format) {
		LOG("%s: format error\n", __FUNCTION__);
		switch (format_act) {
		case V4L2_PIX_FMT_NV12M:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_NV12\n", __FUNCTION__, format_act);
			break;
		case V4L2_PIX_FMT_NV21M:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_NV21\n", __FUNCTION__, format_act);
			break;
		case V4L2_PIX_FMT_LBC_1_0X:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_LBC_1_0X\n", __FUNCTION__, format_act);
			break;
		case V4L2_PIX_FMT_LBC_1_5X:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_LBC_1_5X\n", __FUNCTION__, format_act);
			break;
		case V4L2_PIX_FMT_LBC_2_0X:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_LBC_2_0X\n", __FUNCTION__, format_act);
			break;
		case V4L2_PIX_FMT_LBC_2_5X:
			LOG("%s: source format --- %d --- V4L2_PIX_FMT_LBC_2_5X\n", __FUNCTION__, format_act);
			break;
		default:
			LOG("%s: source format --- %d --- other format\n", __FUNCTION__, format_act);
			break;
		}
		switch (cap_fmt->format) {
		case V4L2_PIX_FMT_NV12M:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_NV12\n", __FUNCTION__, cap_fmt->format);
			break;
		case V4L2_PIX_FMT_NV21M:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_NV21\n", __FUNCTION__, cap_fmt->format);
			break;
		case V4L2_PIX_FMT_LBC_1_0X:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_LBC_1_0X\n", __FUNCTION__, cap_fmt->format);
			break;
		case V4L2_PIX_FMT_LBC_1_5X:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_LBC_1_5X\n", __FUNCTION__, cap_fmt->format);
			break;
		case V4L2_PIX_FMT_LBC_2_0X:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_LBC_2_0X\n", __FUNCTION__, cap_fmt->format);
			break;
		case V4L2_PIX_FMT_LBC_2_5X:
			LOG("%s: tools format --- %d --- V4L2_PIX_FMT_LBC_2_5X\n", __FUNCTION__, cap_fmt->format);
			break;
		default:
			LOG("%s: tools format --- %d --- other format\n", __FUNCTION__, cap_fmt->format);
			break;
		}
		return CAP_ERR_GET_FRAME;
	}

	if (width_act != cap_fmt->width || abs(height_act - cap_fmt->height) >= 16 ||
		((cap_fmt->format == V4L2_PIX_FMT_LBC_1_0X || cap_fmt->format == V4L2_PIX_FMT_LBC_1_5X ||
		cap_fmt->format == V4L2_PIX_FMT_LBC_2_0X || cap_fmt->format == V4L2_PIX_FMT_LBC_2_5X) && height_act != cap_fmt->height)) {//height_act != height
		LOG("%s: resolution error ------ yuv source = %d x %d, tools = %d x %d\n", __FUNCTION__, width_act, height_act, cap_fmt->width, cap_fmt->height);
		return CAP_ERR_GET_FRAME;
	}

	if (define_path) {
		sprintf(isp_cfg_path, "%sframe", ini_cfg.capture_path);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/capture/frame", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
						ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	}
	fd = fopen(isp_cfg_path, "r");
	if (fd != NULL) {
		if (!cap_fmt->framecount) {
			cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
			pic_size = cap_fmt->length;
			cnt = fread(cap_fmt->buffer, 1, pic_size, fd);
			size_act += cnt;
		} else {
			cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
			pic_size = cap_fmt->length / cap_fmt->framecount + 1;
			comm_packet->data_length = htonl(cap_fmt->length);
			comm_packet->reserved[0] = htonl(cap_fmt->format);
			comm_packet->reserved[1] = htonl((cap_fmt->width << 16) | (cap_fmt->height & 0x0000ffff));
			comm_packet->reserved[2] = htonl((cap_fmt->width_stride[0] << 16) | (cap_fmt->width_stride[1] & 0x0000ffff));
			comm_packet->reserved[3] = htonl((cap_fmt->width_stride[2] << 16));
			ret = sock_write_check_packet(__FUNCTION__, sock_fd, comm_packet, SOCK_CMD_PREVIEW, SOCK_DEFAULT_TIMEOUT);
			if (SOCK_RW_CHECK_OK == ret) {
				for (i = 0; i < cap_fmt->framecount; i++) {
					cnt = fread(cap_fmt->buffer, 1, pic_size, fd);
					size_act += cnt;
					if (size_act > cap_fmt->length) {
						cnt -= size_act - cap_fmt->length;
						size_act -= size_act - cap_fmt->length;
					}
					sock_write(sock_fd, (const void *)cap_fmt->buffer, cnt, SOCK_DEFAULT_TIMEOUT);
				}
			}
		}

		if (size_act != cap_fmt->length)
			LOG("%s: get frame error, size_act:%d != length:%d\n", __FUNCTION__, size_act, cap_fmt->length);

		fclose(fd);
		fd = NULL;
		#if 0
		if(cap_fmt->height % 16) { // height ALIGN
			uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
			for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
				*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
			}
		}
		#endif
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	return CAP_ERR_NONE;

skip_frame:
	if (!cap_fmt->framecount) {
		cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
		memset(cap_fmt->buffer, 0, cap_fmt->length);
	} else {
		cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
		pic_size = cap_fmt->length / cap_fmt->framecount + 1;
		comm_packet->data_length = htonl(cap_fmt->length);
		comm_packet->reserved[0] = htonl(cap_fmt->format);
		comm_packet->reserved[1] = htonl((cap_fmt->width << 16) | (cap_fmt->height & 0x0000ffff));
		comm_packet->reserved[2] = htonl((cap_fmt->width_stride[0] << 16) | (cap_fmt->width_stride[1] & 0x0000ffff));
		comm_packet->reserved[3] = htonl((cap_fmt->width_stride[2] << 16));
		ret = sock_write_check_packet(__FUNCTION__, sock_fd, comm_packet, SOCK_CMD_PREVIEW, SOCK_DEFAULT_TIMEOUT);
		if (SOCK_RW_CHECK_OK == ret) {
			for (i = 0; i < cap_fmt->framecount; i++) {
				memset(cap_fmt->buffer, 0, pic_size);
				size_act += pic_size;
				if (size_act > cap_fmt->length) {
					sock_write(sock_fd, (const void *)cap_fmt->buffer, pic_size - (size_act - cap_fmt->length), SOCK_DEFAULT_TIMEOUT);
				} else {
					sock_write(sock_fd, (const void *)cap_fmt->buffer, pic_size, SOCK_DEFAULT_TIMEOUT);
				}
			}
		}
	}
	return CAP_ERR_NONE;
}

HW_S32 ini_tuning_get_encpp_frame(capture_format *cap_fmt, sock_packet *comm_packet, int sock_fd)
{
	char isp_cfg_path[128];
	int i, ret, cnt = 0, pic_size = 0, size_act = 0, define_path;
	FILE *fd = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();
	memset(isp_cfg_path, 0, sizeof(isp_cfg_path));

	if (strcmp(ini_cfg.base_path, ini_cfg.capture_path) == 0) {
		define_path = 0;
	} else {
		define_path = 1;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	if (access(isp_cfg_path, F_OK) != 0) {
#ifndef ANDROID_TUNING
		sprintf(isp_cfg_path, "mkdir %sisp%d_%d_%d_%d_%d/encpp/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		ret = system(isp_cfg_path);
		if (ret) {
			ISP_ERR("system run error!!! (command: %s, ret: %d)\n", isp_cfg_path, ret);
			return CAP_ERR_GET_FRAME;
		}
#else
		ISP_ERR("path %s is invalid\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
#endif
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/encpp_width", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	fd = fopen(isp_cfg_path, "w+");
	if (fd) {
		sprintf(isp_cfg_path, "%d", cap_fmt->width);
		fputs(isp_cfg_path, fd);
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/encpp_height", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	fd = fopen(isp_cfg_path, "w+");
	if (fd) {
		sprintf(isp_cfg_path, "%d", cap_fmt->height);
		fputs(isp_cfg_path, fd);
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/encpp_flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	cnt = 0;
	if (access(isp_cfg_path, F_OK) != 0) {
		while(1) {
			fd = fopen(isp_cfg_path, "w+");
			if (fd == NULL) {
				cnt++;
			} else {
				break;
			}
			usleep(1 << 10);
			if (cnt > 100) {
				ISP_ERR("cannot open %s, open timeout\n", isp_cfg_path);
				return CAP_ERR_GET_FRAME;
			}
		}
		if (define_path) {
			sprintf(isp_cfg_path, "%s", ini_cfg.capture_path);
		} else {
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		}
		fputs(isp_cfg_path, fd);
		fclose(fd);
		fd = NULL;
	}

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/encpp_flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	cnt = 0;
	while(1) {
		if (access(isp_cfg_path, F_OK) != 0) {
			break;
		} else {
			cnt++;
		}
		usleep(1 << 10);
		if (cnt > 5000) {
			ISP_ERR("get frame timeout\n");
			return CAP_ERR_GET_FRAME;
		}
	}

	if (define_path) {
		sprintf(isp_cfg_path, "%sencpp_frame", ini_cfg.capture_path);
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/encpp/encpp_frame", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
						ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
	}
	fd = fopen(isp_cfg_path, "r");
	if (fd != NULL) {
		if (!cap_fmt->framecount) {
			cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
			pic_size = cap_fmt->length;
			cnt = fread(cap_fmt->buffer, 1, pic_size, fd);
			size_act += cnt;
		} else {
			cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
			pic_size = cap_fmt->length / cap_fmt->framecount + 1;
			comm_packet->data_length = htonl(cap_fmt->length);
			comm_packet->reserved[0] = htonl(cap_fmt->format);
			comm_packet->reserved[1] = htonl((cap_fmt->width << 16) | (cap_fmt->height & 0x0000ffff));
			comm_packet->reserved[2] = htonl((cap_fmt->width_stride[0] << 16) | (cap_fmt->width_stride[1] & 0x0000ffff));
			comm_packet->reserved[3] = htonl((cap_fmt->width_stride[2] << 16));
			ret = sock_write_check_packet(__FUNCTION__, sock_fd, comm_packet, SOCK_CMD_VENCODE, SOCK_DEFAULT_TIMEOUT);
			if (SOCK_RW_CHECK_OK == ret) {
				for (i = 0; i < cap_fmt->framecount; i++) {
					cnt = fread(cap_fmt->buffer, 1, pic_size, fd);
					size_act += cnt;
					if (size_act > cap_fmt->length) {
						cnt -= size_act - cap_fmt->length;
						size_act -= size_act - cap_fmt->length;
					}
					sock_write(sock_fd, (const void *)cap_fmt->buffer, cnt, SOCK_DEFAULT_TIMEOUT);
				}
			}
		}

		if (size_act != cap_fmt->length)
			LOG("%s: get frame error, size_act:%d != length:%d\n", __FUNCTION__, size_act, cap_fmt->length);

		fclose(fd);
		fd = NULL;
		#if 0
		if(height % 16) { // height ALIGN
			uptr = buffer + (width * height);
			for(i = 0; i < width * height / 2; i++) {
				*(uptr + i) = *(uptr + i + (width * (16 - height % 16)));
			}
		}
		#endif
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
		return CAP_ERR_GET_FRAME;
	}

	return CAP_ERR_NONE;
}

HW_S32 ini_tuning_get_3a_stat(struct isp_stats_context *stats_ctx)
{
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();
	char isp_cfg_path[128], buf[50], rdval = 0;
	int retry = 0;
	FILE *fd = NULL;
	memset(isp_cfg_path, 0, sizeof(isp_cfg_path));

	sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/3a_stat/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
re_try:
	fd = fopen(isp_cfg_path, "r");
	if (fd) {
		if (fgets(buf, 50, fd) != NULL) {
			rdval = atoi(buf);
		}
		fclose(fd);
		fd = NULL;
	} else {
		ISP_ERR("%s open failed\n", isp_cfg_path);
	}

	if (!rdval) {
		usleep(50 << 10);
		retry++;
		if (retry >= 10) {
			ISP_ERR("get 3a_stat timeout\n");
			return -1;
		} else {
			goto re_try;
		}
	} else {
		memset(stats_ctx, 0, sizeof(struct isp_stats_context));
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/3a_stat/ae", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "r");
		if (fd) {
			fread(&stats_ctx->stats.ae_stats, sizeof(struct isp_ae_stats_s), 1, fd);

			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s open failed\n", isp_cfg_path);
			return -1;
		}

		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/3a_stat/awb", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "r");
		if (fd) {
			fread(&stats_ctx->stats.awb_stats, sizeof(struct isp_awb_stats_s), 1, fd);

			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s open failed\n", isp_cfg_path);
			return -1;
		}

		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/3a_stat/d3d_k", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "r");
		if (fd) {
			fread(&stats_ctx->stats.d3d_k_stats, sizeof(struct isp_d3d_k_stats_s), 1, fd);

			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s open failed\n", isp_cfg_path);
			return -1;
		}

		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/3a_stat/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
		fd = fopen(isp_cfg_path, "w+");
		if (fd) {
			fputc('0', fd);
			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s open failed\n", isp_cfg_path);
			return -1;
		}
	}
	return 0;
}

HW_S32 get_log_params(int dev_id, unsigned char *buffer)
{
	struct isp_log_cfg *log_cfg = (struct isp_log_cfg *)buffer;
	struct isp_lib_context *ctx = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();
	char isp_cfg_path[128], buf[50], rdval = 0;
	int i, retry = 0;
	FILE *fd = NULL;

	ctx = isp_get_ctx_addr(dev_id);
	if (ctx == NULL)
		return -1;

	if (!ini_cfg.enable) {
		log_cfg->ae_log.ae_frame_id = ctx->ae_frame_cnt;
		log_cfg->ae_log.ev_sensor_exp_line = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_sensor_exp_line;
		log_cfg->ae_log.ev_exposure_time = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_exposure_time;
		log_cfg->ae_log.ev_analog_gain = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_analog_gain;
		log_cfg->ae_log.ev_digital_gain = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_digital_gain;
		log_cfg->ae_log.ev_total_gain = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_total_gain;
		log_cfg->ae_log.ev_idx = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_idx;
		log_cfg->ae_log.ev_idx_max = ctx->ae_entity_ctx.ae_result.sensor_set.ev_idx_max;
		log_cfg->ae_log.ev_lv = ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_lv;
		log_cfg->ae_log.ev_lv_adj = ctx->ae_entity_ctx.ae_result.ev_lv_adj;
		log_cfg->ae_log.ae_target = ctx->ae_entity_ctx.ae_result.ae_target;
		log_cfg->ae_log.ae_avg_lum = ctx->ae_entity_ctx.ae_result.ae_avg_lum;
		log_cfg->ae_log.ae_weight_lum = ctx->ae_entity_ctx.ae_result.ae_weight_lum;
		log_cfg->ae_log.ae_delta_exp_idx = ctx->ae_entity_ctx.ae_result.ae_delta_exp_idx;
		log_cfg->ae_log.ev_sensor_exp_line_short = ctx->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_sensor_exp_line;
		log_cfg->ae_log.ev_exposure_time_short = ctx->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_exposure_time;
		log_cfg->ae_log.ev_analog_gain_short = ctx->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_analog_gain;
		log_cfg->ae_log.ev_digital_gain_short = ctx->ae_entity_ctx.ae_result.sensor_set_short.ev_set_curr.ev_digital_gain;
		log_cfg->ae_log.ratio_lm = ctx->ae_entity_ctx.ae_result.ae_wdr_ratio.real;
		log_cfg->ae_log.ae_mode = ctx->ae_settings.ae_mode;

		log_cfg->awb_log.awb_frame_id = ctx->awb_frame_cnt;
		log_cfg->awb_log.r_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.r_gain;
		log_cfg->awb_log.gr_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.gr_gain;
		log_cfg->awb_log.gb_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.gb_gain;
		log_cfg->awb_log.b_gain = ctx->awb_entity_ctx.awb_result.wb_gain_output.b_gain;
		log_cfg->awb_log.color_temp_output = ctx->awb_entity_ctx.awb_result.color_temp_output;
		log_cfg->awb_log.color_temp_target = ctx->awb_entity_ctx.awb_result.color_temp_target;
		for (i = 0; i < 10; i++) {
			log_cfg->awb_log.LightWinNum[i] = ctx->awb_entity_ctx.awb_result.LightWinNum[i];
			log_cfg->awb_log.LightTempMean[i] = ctx->awb_entity_ctx.awb_result.LightTempMean[i];
		}

		log_cfg->af_log.af_frame_id = ctx->af_frame_cnt;
		log_cfg->af_log.last_code_output = ctx->af_entity_ctx.af_result.last_code_output;
		log_cfg->af_log.real_code_output = ctx->af_entity_ctx.af_result.real_code_output;
		log_cfg->af_log.std_code_output = ctx->af_entity_ctx.af_result.std_code_output;

		log_cfg->iso_log.gain_idx = ctx->iso_entity_ctx.iso_result.gain_idx;
		log_cfg->iso_log.lum_idx = ctx->iso_entity_ctx.iso_result.lum_idx;
		log_cfg->iso_log.temp_enable = ctx->temp_info.enable;
		log_cfg->iso_log.temperature = ctx->sensor_info.temperature;
		for (i = 0; i < TEMP_COMP_MAX; i++) {
			log_cfg->iso_log.temperature_param[i] = ctx->temp_info.temperature_param[i];
		}

		log_cfg->pltm_log.pltm_auto_stren = ctx->pltm_entity_ctx.pltm_result.pltm_auto_stren;
		log_cfg->pltm_log.pltm_sharp_ss_compensation = ctx->pltm_entity_ctx.pltm_result.pltm_sharp_ss_compensation;
		log_cfg->pltm_log.pltm_sharp_ls_compensation = ctx->pltm_entity_ctx.pltm_result.pltm_sharp_ls_compensation;
		log_cfg->pltm_log.pltm_d2d_compensation = ctx->pltm_entity_ctx.pltm_result.pltm_d2d_compensation;
		log_cfg->pltm_log.pltm_d3d_compensation = ctx->pltm_entity_ctx.pltm_result.pltm_d3d_compensation;
		log_cfg->pltm_log.pltm_dark_block_num = ctx->pltm_entity_ctx.pltm_result.pltm_dark_block_num;
		log_cfg->pltm_log.cur_pic_lum = ctx->pltm_entity_ctx.pltm_result.cur_pic_lum;
		log_cfg->pltm_log.tar_pic_lum = ctx->pltm_entity_ctx.pltm_result.tar_pic_lum;
	} else {
		sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/LOG/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
re_try:
		fd = fopen(isp_cfg_path, "r");
		if (fd) {
			if (fgets(buf, 50, fd) != NULL) {
				rdval = atoi(buf);
			}
			fclose(fd);
			fd = NULL;
		} else {
			ISP_ERR("%s open failed\n", isp_cfg_path);
		}

		if (!rdval) {
			usleep(50 << 10);
			retry++;
			if (retry >= 10) {
				ISP_ERR("get log timeout\n");
				return -1;
			} else {
				goto re_try;
			}
		} else {
			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/LOG/Info", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "r");
			if (fd) {
				fread(log_cfg, sizeof(struct isp_log_cfg), 1, fd);

				fclose(fd);
				fd = NULL;
			} else {
				ISP_ERR("%s open failed\n", isp_cfg_path);
				return -1;
			}

			sprintf(isp_cfg_path, "%sisp%d_%d_%d_%d_%d/LOG/flag", ini_cfg.base_path, ini_cfg.sensor_cfg.isp, ini_cfg.sensor_cfg.width, ini_cfg.sensor_cfg.height,
					ini_cfg.sensor_cfg.fps, ini_cfg.sensor_cfg.wdr);
			fd = fopen(isp_cfg_path, "w+");
			if (fd) {
				fputc('0', fd);
				fclose(fd);
				fd = NULL;
			} else {
				ISP_ERR("%s open failed\n", isp_cfg_path);
				return -1;
			}
		}
	}

	return 0;
}

