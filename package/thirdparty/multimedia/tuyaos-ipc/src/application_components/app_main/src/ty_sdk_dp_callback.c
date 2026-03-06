/*********************************************************************************
*Copyright(C),2015-2020, 
*TUYA 
*FileName: tuya_ipc_dp_utils.c
*
* File Description：
* 1. API implementation of DP point
*
* This file code is the basic code, users don't care it
* Please do not modify any contents of this file at will. 
* Please contact the Product Manager if you need to modify it.
*
**********************************************************************************/
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_config.h"
#include "ty_dp_define.h"
#include "ty_sdk_common.h"
#include "utilities/uni_log.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "tkl_video_in.h"
#include "tal_audio.h"

STATIC VOID respone_dp_value(BYTE_T dp_id, INT_T val);
STATIC VOID respone_dp_bool(BYTE_T dp_id, BOOL_T true_false);
STATIC VOID respone_dp_enum(BYTE_T dp_id, CHAR_T *p_val_enum);
STATIC VOID respone_dp_str(BYTE_T dp_id, CHAR_T *p_val_str);
STATIC VOID handle_DP_SD_STORAGE_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp);
extern void add_md_pending_count(void);
extern void remove_md_pending_count(void);

//------------------------------------------
VOID TUYA_IPC_upload_all_status(VOID)
{
#ifdef TUYA_DP_SLEEP_MODE
    respone_dp_bool(TUYA_DP_SLEEP_MODE, IPC_APP_get_sleep_mode() );
#endif

#ifdef TUYA_DP_LIGHT
    respone_dp_bool(TUYA_DP_LIGHT, IPC_APP_get_light_onoff() );
#endif

#ifdef TUYA_DP_FLIP
    respone_dp_bool(TUYA_DP_FLIP, IPC_APP_get_flip_onoff());
    IPC_APP_set_flip_onoff(IPC_APP_get_flip_onoff());
#endif

#ifdef TUYA_DP_WATERMARK
    BOOL_T watermark_onoff = IPC_APP_get_watermark_onoff();
    if (watermark_onoff == true) {
        IPC_APP_set_watermark_onoff(true);
    }
    respone_dp_bool(TUYA_DP_WATERMARK, watermark_onoff);
#endif

#ifdef TUYA_DP_WDR
    respone_dp_bool(TUYA_DP_WDR, IPC_APP_get_wdr_onoff() );
#endif

#ifdef TUYA_DP_NIGHT_MODE
    creat_check_night_thread();

    CHAR_T *night_mode = IPC_APP_get_night_mode();
    PR_INFO("current night_mode :%s\n", night_mode);
    respone_dp_enum(TUYA_DP_NIGHT_MODE, night_mode);
    IPC_APP_set_night_mode(night_mode);
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
    respone_dp_bool(TUYA_DP_ALARM_ZONE_ENABLE, IPC_APP_get_alarm_zone_onoff() );
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
    CHAR_T alarm_zone[256] = {0};
    IPC_APP_get_alarm_zone_draw(alarm_zone);
    respone_dp_str(TUYA_DP_ALARM_ZONE_DRAW, alarm_zone);
#endif
#ifdef TUYA_DP_SD_STATUS_ONLY_GET
    respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, IPC_APP_get_sd_status() );
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
    handle_DP_SD_STORAGE_ONLY_GET(NULL);
#endif

#ifdef TUYA_DP_SD_RECORD_ENABLE
    respone_dp_bool(TUYA_DP_SD_RECORD_ENABLE, IPC_APP_get_sd_record_onoff() );
#endif

#ifdef TUYA_DP_SD_RECORD_MODE
    CHAR_T sd_mode[4];
    snprintf(sd_mode,4,"%d",IPC_APP_get_sd_record_mode());
    respone_dp_enum(TUYA_DP_SD_RECORD_MODE, sd_mode);
#endif


#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
    respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, 0 );
#endif

#ifdef TUYA_DP_BLUB_SWITCH
    IPC_APP_blub_init();

    BOOL_T blub_onoff = IPC_APP_get_blub_onoff();
    respone_dp_bool(TUYA_DP_BLUB_SWITCH, blub_onoff);
    if (blub_onoff == TRUE) {
        IPC_APP_set_blub_onoff(TRUE);
    }
#endif

#ifdef TUYA_DP_SOUND_SENSITIVITY
    CHAR_T *p_sens = IPC_APP_get_sound_sensitivity();
    PR_INFO("current sound p_sens:%s\n", p_sens);
    IPC_APP_set_sound_sensitivity(p_sens);

    respone_dp_enum(TUYA_DP_SOUND_SENSITIVITY, p_sens);
#endif

#ifdef TUYA_DP_SOUND_DETECT
    BOOL_T sound_detect_onoff = IPC_APP_get_sound_detect_onoff();
    respone_dp_bool(TUYA_DP_SOUND_DETECT, sound_detect_onoff);
    PR_INFO("current sound_detect_onoff :%d\n", sound_detect_onoff);
    if (sound_detect_onoff == TRUE) {
        IPC_APP_set_sound_detect_onoff(true);
    }
#endif

#ifdef TUYA_DP_DEVICE_VOLUME
    IPC_APP_update_device_volume_status();
#endif

#ifdef TUYA_DP_IPC_CONTRAST
    // INT_T contrast_value = IPC_APP_get_ipc_contrast();
    respone_dp_value(TUYA_DP_IPC_CONTRAST, 50);
#endif

#ifdef TUYA_DP_IPC_BRIGHT
    // INT_T bright_value = IPC_APP_get_ipc_bright();
    respone_dp_value(TUYA_DP_IPC_BRIGHT, 50);
#endif

#ifdef TUYA_DP_BAT_REPORT_CAP
    respone_dp_value(TUYA_DP_BAT_REPORT_CAP, 1);
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
    IPC_APP_update_alarm_function_status();
#endif

#if defined(TUYA_DP_SIREN_SWITCH) && defined(TUYA_DP_SIREN_SQUIRREL)
    IPC_APP_update_siren_squirrel_status();
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
    IPC_APP_update_alarm_sensitivity_status();
#endif

#ifdef TUYA_DP_PIR_ALARM_INTERVAL
    IPC_APP_update_alarm_interval_status();
#endif

#ifdef TUYA_DP_LOWELECTRIC
    IPC_APP_update_lowelectric_status();
#endif

#ifdef TUYA_DP_HUM_FILTER
    respone_dp_bool(TUYA_DP_HUM_FILTER, IPC_APP_get_human_filter());
    if (IPC_APP_get_human_filter() == TRUE) { //读取到上次的状态开的话，需调用一次
        IPC_APP_set_human_filter(true);
    }
#endif

#ifdef TUYA_DP_SHOW_MAC
    IPC_APP_update_show_mac();
#endif

#ifdef TUYA_DP_FLICKER
    UINT_T flicker_value = IPC_APP_get_flicker();
    CHAR_T flicker_sValue[2];
    snprintf(flicker_sValue, 2, "%d", flicker_value);
    respone_dp_enum(TUYA_DP_FLICKER, flicker_sValue);
    IPC_APP_set_flicker(flicker_value);
#endif

#ifdef TUYA_DP_POWERMODE
    IPC_APP_update_battery_status();
#endif
}

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
VOID IPC_APP_report_sd_format_status(INT_T status)
{
    respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, status);
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
VOID IPC_APP_report_sd_status_changed(INT_T status)
{
    respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, status);
}
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_report_sd_storage()
{
    CHAR_T tmp_str[100] = {0};

    UINT_T total = 100;
    UINT_T used = 0;
    UINT_T empty = 100;
    IPC_APP_get_sd_storage(&total, &used, &empty);

    //"total capacity|Current usage|remaining capacity"
    snprintf(tmp_str, 100, "%u|%u|%u", total, used, empty);
    respone_dp_str(TUYA_DP_SD_STORAGE_ONLY_GET, tmp_str);
}
#endif


#ifdef TUYA_DP_POWERMODE
VOID IPC_APP_update_battery_status(VOID)
{
    CHAR_T *power_mode = IPC_APP_get_power_mode();
    INT_T percent = IPC_APP_get_battery_percent();

    PR_INFO("current power mode:%s\r\n", power_mode);
    respone_dp_enum(TUYA_DP_POWERMODE, power_mode);
    PR_INFO("current battery percent:%d\r\n", percent);
    respone_dp_value(TUYA_DP_ELECTRICITY, percent);
}
#endif

#ifdef TUYA_DP_DEVICE_VOLUME
VOID IPC_APP_update_device_volume_status(VOID)
{
    INT_T volume = IPC_APP_get_device_volume();
    // 视频流音量,强制85
    INT_T tal_ao_volume = 85;
    PR_INFO("tal_ao current volume :%d\n", tal_ao_volume);
    tal_ao_set(0, 0, NULL, TAL_AO_CMD_VOL, &tal_ao_volume);
    
    PR_INFO("tkl_ao current volume :%d\n", volume);
    IPC_APP_set_device_volume(volume);
    respone_dp_value(TUYA_DP_DEVICE_VOLUME, volume);
}
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
VOID IPC_APP_update_alarm_function_status(VOID)
{
    BOOL_T alarm_on_off = IPC_APP_get_alarm_function_onoff_only();
    BOOL_T bird_on_off = IPC_APP_get_bird_onoff();

    PR_INFO("current alarm_on_off :%d\n", alarm_on_off);
    PR_INFO("current bird_on_off :%d\n", bird_on_off);

    /* always open*/
    tkl_vi_detect_start(0, TKL_MEDIA_DETECT_TYPE_MOTION);

    // 同时为1,默认开移动
    if ((alarm_on_off == 1) && (bird_on_off == 1)){
        IPC_APP_set_bird_onoff(0);
        IPC_APP_set_alarm_function_onoff(1);
    }

    respone_dp_bool(TUYA_DP_ALARM_FUNCTION, alarm_on_off);
    respone_dp_bool(TUYA_DP_BIRD_SWITCH, bird_on_off);

    alarm_on_off = IPC_APP_get_alarm_function_onoff();
    PR_INFO("current alarm_on_off|bird_on_off :%d\n", alarm_on_off);
}
#endif

#if defined(TUYA_DP_SIREN_SWITCH) && defined(TUYA_DP_SIREN_SQUIRREL)
VOID IPC_APP_update_siren_squirrel_status(VOID)
{
    BOOL_T squirrel_on_off = IPC_APP_get_squirrel_onoff();
    BOOL_T siren_on_off = IPC_APP_get_siren_onoff();

    respone_dp_bool(TUYA_DP_SIREN_SWITCH, siren_on_off);
    respone_dp_bool(TUYA_DP_SIREN_SQUIRREL, squirrel_on_off);
}
#endif

#ifdef TUYA_DP_AWAKE_STATUS
VOID IPC_APP_update_awake_status(BOOL_T true_false)
{
    BOOL_T awake_state = true_false;
    PR_INFO("current awake_state :%d\n", awake_state);
    respone_dp_bool(TUYA_DP_AWAKE_STATUS, awake_state);
}
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
VOID IPC_APP_update_alarm_sensitivity_status(VOID)
{
    CHAR_T *p_sensitivity = IPC_APP_get_alarm_sensitivity();
    PR_INFO("current p_sensitivity :%s\n", p_sensitivity);

    respone_dp_enum(TUYA_DP_ALARM_SENSITIVITY, p_sensitivity);
}
#endif

#ifdef TUYA_DP_PIR_ALARM_INTERVAL
VOID IPC_APP_update_alarm_interval_status(VOID)
{
    CHAR_T *p_interval = IPC_APP_get_alarm_interval();
    PR_INFO("current p_interval :%s\n", p_interval);

    respone_dp_enum(TUYA_DP_PIR_ALARM_INTERVAL, p_interval);
}
#endif

#ifdef TUYA_DP_LOWELECTRIC
VOID IPC_APP_update_lowelectric_status(VOID)
{
    int value = IPC_APP_get_lowelectric();
    PR_INFO("current lowelectric :%d\n", value);

    respone_dp_value(TUYA_DP_LOWELECTRIC, value);
}
#endif


#ifdef TUYA_DP_SHOW_MAC
VOID IPC_APP_update_show_mac(VOID)
{
    int ret;
    NW_MAC_S mac = { {0} };
    char mac_str[18] = { 0 };

    ret = IPC_APP_get_mac(&mac);
    if (ret == 0) {
        snprintf(mac_str, sizeof(mac_str), "%02x-%02x-%02x-%02x-%02x-%02x",
            mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);
        PR_INFO("wifi mac:%s\n", mac_str);
        respone_dp_str(TUYA_DP_SHOW_MAC, mac_str);
    }
}
#endif

//------------------------------------------
STATIC VOID respone_dp_value(BYTE_T dp_id, INT_T val)
{
    tuya_ipc_dp_report(NULL, dp_id,PROP_VALUE,&val,1);
}

STATIC VOID respone_dp_bool(BYTE_T dp_id, BOOL_T true_false)
{
    tuya_ipc_dp_report(NULL, dp_id,PROP_BOOL,&true_false,1);
}

STATIC VOID respone_dp_enum(BYTE_T dp_id, CHAR_T *p_val_enum)
{
    tuya_ipc_dp_report(NULL, dp_id,PROP_ENUM,p_val_enum,1);
}

STATIC VOID respone_dp_str(BYTE_T dp_id, CHAR_T *p_val_str)
{
    tuya_ipc_dp_report(NULL, dp_id,PROP_STR,p_val_str,1);
}

//------------------------------------------
STATIC BOOL_T check_dp_bool_invalid(IN TY_OBJ_DP_S *p_obj_dp)
{
    if (p_obj_dp == NULL) {
        PR_ERR("error! input is null \r\n");
        return -1;
    }

    if (p_obj_dp->type != PROP_BOOL) {
        PR_ERR("error! input is not bool %d \r\n", p_obj_dp->type);
        return -2;
    }

    if (p_obj_dp->value.dp_bool == 0) {
        return FALSE;
    } else if (p_obj_dp->value.dp_bool == 1) {
        return TRUE;
    } else {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->value.dp_bool);
        return -2;
    }
}

//------------------------------------------

#ifdef TUYA_DP_SLEEP_MODE
STATIC VOID handle_DP_SLEEP_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T sleep_mode = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_sleep_mode(sleep_mode);
    sleep_mode = IPC_APP_get_sleep_mode();

    respone_dp_bool(TUYA_DP_SLEEP_MODE, sleep_mode);
}
#endif

#ifdef TUYA_DP_LIGHT
STATIC VOID handle_DP_LIGHT(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T light_on_off = check_dp_bool_invalid(p_obj_dp);

    add_md_pending_count();
    IPC_APP_set_light_onoff(light_on_off);
    light_on_off = IPC_APP_get_light_onoff();

    respone_dp_bool(TUYA_DP_LIGHT, light_on_off);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_FLIP
STATIC VOID handle_DP_FLIP(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T flip_on_off = check_dp_bool_invalid(p_obj_dp);

    add_md_pending_count();
    IPC_APP_set_flip_onoff(flip_on_off);
    flip_on_off = IPC_APP_get_flip_onoff();

    respone_dp_bool(TUYA_DP_FLIP, flip_on_off);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_WATERMARK
STATIC VOID handle_DP_WATERMARK(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T watermark_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_watermark_onoff(watermark_on_off);
    watermark_on_off = IPC_APP_get_watermark_onoff();

    respone_dp_bool(TUYA_DP_WATERMARK, watermark_on_off);
}
#endif

#ifdef TUYA_DP_WDR
STATIC VOID handle_DP_WDR(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T wdr_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_wdr_onoff(wdr_on_off);
    wdr_on_off = IPC_APP_get_wdr_onoff();

    respone_dp_bool(TUYA_DP_WDR, wdr_on_off);
}
#endif

#ifdef TUYA_DP_NIGHT_MODE
STATIC VOID handle_DP_NIGHT_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    CHAR_T tmp_str[2] = { 0 };
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    add_md_pending_count();
    IPC_APP_set_night_mode(tmp_str);
    CHAR_T *p_night_mode = IPC_APP_get_night_mode();

    respone_dp_enum(TUYA_DP_NIGHT_MODE, p_night_mode);
    remove_md_pending_count();
}
#endif


#ifdef TUYA_DP_ALARM_FUNCTION
STATIC VOID handle_DP_ALARM_FUNCTION(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T alarm_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_alarm_function_onoff(alarm_on_off);
    alarm_on_off = IPC_APP_get_alarm_function_onoff();
    respone_dp_bool(TUYA_DP_ALARM_FUNCTION, alarm_on_off);

    #ifdef LOW_BAT_IPC
    BOOL_T bird_on_off = 0;
    if (alarm_on_off == 1){
        IPC_APP_set_bird_onoff(0);
    }
    bird_on_off = IPC_APP_get_bird_onoff();
    respone_dp_bool(TUYA_DP_BIRD_SWITCH, bird_on_off);
    #endif

    //当关闭报警时，也关闭人形过滤
    if (alarm_on_off == false) {
        respone_dp_bool(TUYA_DP_HUM_FILTER, false);
    }
}
#endif

#ifdef TUYA_DP_BIRD_SWITCH
STATIC VOID handle_DP_BIRD_SWITCH(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T bird_on_off = check_dp_bool_invalid(p_obj_dp);
    BOOL_T alarm_on_off = 0;

    IPC_APP_set_bird_onoff(bird_on_off);
    bird_on_off = IPC_APP_get_bird_onoff();
    respone_dp_bool(TUYA_DP_BIRD_SWITCH, bird_on_off);

    if (bird_on_off == 1){
        IPC_APP_set_alarm_function_onoff(0);
        respone_dp_bool(TUYA_DP_ALARM_FUNCTION, 0);
    }
}
#endif

#ifdef TUYA_DP_SIREN_SWITCH
STATIC VOID handle_DP_SIREN_SWITCH(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T siren_on_off = check_dp_bool_invalid(p_obj_dp);
    BOOL_T squirrel_on_off = 0;

    IPC_APP_set_siren_onoff(siren_on_off);
    if (siren_on_off == 1){
        IPC_APP_set_squirrel_onoff(0);
    }

    siren_on_off = IPC_APP_get_siren_onoff();
    squirrel_on_off = IPC_APP_get_squirrel_onoff();

    respone_dp_bool(TUYA_DP_SIREN_SWITCH, siren_on_off);
    respone_dp_bool(TUYA_DP_SIREN_SQUIRREL, squirrel_on_off);
}
#endif

#ifdef TUYA_DP_SIREN_SQUIRREL
STATIC VOID handle_DP_SIREN_SQUIRREL(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T squirrel_on_off = check_dp_bool_invalid(p_obj_dp);
    BOOL_T siren_on_off = 0;

    IPC_APP_set_squirrel_onoff(squirrel_on_off);
    if (squirrel_on_off == 1){
        IPC_APP_set_siren_onoff(0);
    }

    siren_on_off = IPC_APP_get_siren_onoff();
    squirrel_on_off = IPC_APP_get_squirrel_onoff();

    respone_dp_bool(TUYA_DP_SIREN_SWITCH, siren_on_off);
    respone_dp_bool(TUYA_DP_SIREN_SQUIRREL, squirrel_on_off);
}
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
STATIC VOID handle_DP_ALARM_SENSITIVITY(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = {0};
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    IPC_APP_set_alarm_sensitivity(tmp_str);
    CHAR_T *p_sensitivity = IPC_APP_get_alarm_sensitivity();

    respone_dp_enum(TUYA_DP_ALARM_SENSITIVITY, p_sensitivity);
}
#endif

#ifdef TUYA_DP_PIR_ALARM_INTERVAL
STATIC VOID handle_DP_PIR_ALARM_INTERVAL(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = { 0 };
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    IPC_APP_set_alarm_interval(tmp_str);
    CHAR_T *p_interval = IPC_APP_get_alarm_interval();

    respone_dp_enum(TUYA_DP_PIR_ALARM_INTERVAL, p_interval);
}
#endif

#ifdef TUYA_DP_IPC_RECORDING_TIME
STATIC VOID handle_DP_IPC_RECORDING_TIME(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = { 0 };
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;
    respone_dp_enum(TUYA_DP_IPC_RECORDING_TIME, tmp_str);
}
#endif

#ifdef TUYA_DP_IPC_BIRD_DETECT_MODE
STATIC VOID handle_DP_IPC_BIRD_DETECT_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = { 0 };
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    respone_dp_enum(TUYA_DP_IPC_BIRD_DETECT_MODE, tmp_str);
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
STATIC VOID handle_DP_ALARM_ZONE_ENABLE(IN TY_OBJ_DP_S *p_dp_json)
{
    if (p_dp_json == NULL) {
        PR_ERR("Error!! type invalid %p \r\n", p_dp_json);
        return;
    }
    BOOL_T alarm_zone_enable = check_dp_bool_invalid(p_dp_json);
    IPC_APP_set_alarm_zone_onoff(alarm_zone_enable);
    respone_dp_bool(TUYA_DP_ALARM_ZONE_ENABLE, IPC_APP_get_alarm_zone_onoff());
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
STATIC VOID handle_DP_ALARM_ZONE_DRAW(IN TY_OBJ_DP_S *p_dp_json)
{
    if (p_dp_json == NULL) {
        PR_ERR("Error!! type invalid\r\n");
        return;
    }
    IPC_APP_set_alarm_zone_draw((cJSON *)(p_dp_json->value.dp_str));
    respone_dp_str(TUYA_DP_ALARM_ZONE_DRAW, IPC_APP_get_alarm_zone_draw());
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
STATIC VOID handle_DP_SD_STATUS_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp)
{
    INT_T sd_status = IPC_APP_get_sd_status();

    respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, sd_status);
}
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
STATIC VOID handle_DP_SD_STORAGE_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp)
{
    CHAR_T tmp_str[100] = {0};

    UINT_T total = 100;
    UINT_T used = 0;
    UINT_T empty = 100;
    IPC_APP_get_sd_storage(&total, &used, &empty);

    //"total capacity|Current usage|remaining capacity"
    snprintf(tmp_str, 100, "%u|%u|%u", total, used, empty);
    respone_dp_str(TUYA_DP_SD_STORAGE_ONLY_GET, tmp_str);
}
#endif

#ifdef TUYA_DP_SD_RECORD_ENABLE
STATIC VOID handle_DP_SD_RECORD_ENABLE(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T sd_record_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_sd_record_onoff(sd_record_on_off);
    sd_record_on_off = IPC_APP_get_sd_record_onoff();

    respone_dp_bool(TUYA_DP_SD_RECORD_ENABLE, sd_record_on_off);
}
#endif

#ifdef TUYA_DP_SD_RECORD_MODE
STATIC VOID handle_DP_SD_RECORD_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    IPC_APP_set_sd_record_mode(p_obj_dp->value.dp_enum);
    UINT_T mode = IPC_APP_get_sd_record_mode();
    CHAR_T sMode[2];
    snprintf(sMode,2,"%d",mode);
    respone_dp_enum(TUYA_DP_SD_RECORD_MODE,sMode);
}
#endif

#ifdef TUYA_DP_SD_UMOUNT
STATIC VOID handle_DP_SD_UMOUNT(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T umount_result = IPC_APP_unmount_sd_card();
    respone_dp_bool(TUYA_DP_SD_UMOUNT, umount_result);
}
#endif

#ifdef TUYA_DP_SD_FORMAT
STATIC VOID handle_DP_SD_FORMAT(IN TY_OBJ_DP_S *p_obj_dp)
{
    IPC_APP_format_sd_card();
    respone_dp_bool(TUYA_DP_SD_FORMAT, TRUE);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
STATIC VOID handle_DP_SD_FORMAT_STATUS_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp)
{
    INT_T progress = IPC_APP_get_sd_format_status();
    respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, progress);
}
#endif

#ifdef TUYA_DP_PTZ_CONTROL
STATIC VOID handle_DP_PTZ_CONTROL(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    add_md_pending_count(); //pending motion detect
    //dp 119 format: {"range":["1","2","3","4","5","6","7","0"],"type":"enum"}
    UINT_T dp_directions[8] = { 1,2,3,4,5,6,7,0 };
    UINT_T direction = dp_directions[p_obj_dp->value.dp_enum];
    CHAR_T tmp_str[2] = { 0 };
    snprintf(tmp_str, 2, "%d", direction);
    IPC_APP_ptz_start_move(tmp_str);
    respone_dp_enum(TUYA_DP_PTZ_CONTROL, tmp_str);
}
#endif

#ifdef TUYA_DP_PTZ_STOP
STATIC VOID handle_DP_PTZ_STOP(IN TY_OBJ_DP_S *p_obj_dp)
{
    IPC_APP_ptz_stop_move();
    respone_dp_bool(TUYA_DP_PTZ_STOP, TRUE);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_PTZ_CHECK
STATIC VOID handle_DP_PTZ_CHECK(IN TY_OBJ_DP_S *p_obj_dp)
{
    add_md_pending_count();
    IPC_APP_ptz_check();
    respone_dp_bool(TUYA_DP_PTZ_CHECK, TRUE);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_TRACK_ENABLE
STATIC VOID handle_DP_TRACK_ENABLE(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T track_enable = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_track_enable(track_enable);

    respone_dp_bool(TUYA_DP_TRACK_ENABLE, track_enable);
}

#endif

#ifdef TUYA_DP_LINK_MOVE_ACTION
STATIC VOID handle_DP_LINK_MOVE(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = {0};
    int bind_move = 0;
    bind_move = p_obj_dp->value.dp_enum;
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    IPC_APP_set_link_move(bind_move);
    respone_dp_enum(TUYA_DP_LINK_MOVE_ACTION, tmp_str);
}
#endif

#ifdef TUYA_DP_LINK_MOVE_SET
STATIC VOID handle_DP_LINK_MOVE_SET(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = {0};
    int bind_move = 0;
    bind_move = p_obj_dp->value.dp_enum;
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    IPC_APP_set_link_pos(bind_move);
    respone_dp_enum(TUYA_DP_LINK_MOVE_SET, tmp_str);
}
#endif


#ifdef TUYA_DP_HUM_FILTER
STATIC VOID handle_DP_HUM_FILTER(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T hum_filter = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_human_filter(hum_filter);

    respone_dp_bool(TUYA_DP_HUM_FILTER, IPC_APP_get_human_filter());
}
#endif

#ifdef TUYA_DP_PATROL_MODE
STATIC VOID handle_DP_patrol_mode(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    IPC_APP_set_patrol_mode(p_obj_dp->value.dp_enum);
    CHAR_T sMode[2];
    snprintf(sMode, 2, "%d", p_obj_dp->value.dp_enum);

    respone_dp_enum(TUYA_DP_PATROL_MODE,sMode);
}


#endif

#ifdef TUYA_DP_PATROL_SWITCH
STATIC VOID handle_DP_patrol_switch(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T patrol_mode = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_patrol_switch(patrol_mode);

    respone_dp_bool(TUYA_DP_PATROL_SWITCH, patrol_mode);
}
#endif

#ifdef TUYA_DP_PATROL_TMODE
STATIC VOID handle_DP_patrol_tmode(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    IPC_APP_set_patrol_tmode(p_obj_dp->value.dp_enum);
    CHAR_T sMode[2];
    snprintf(sMode, 2, "%d", p_obj_dp->value.dp_enum);
    respone_dp_enum(TUYA_DP_PATROL_TMODE, sMode);
}


#endif

#ifdef TUYA_DP_PATROL_TIME
STATIC VOID handle_DP_patrol_time(IN TY_OBJ_DP_S *p_dp_json)
{
    PR_INFO("---%s---\n",p_dp_json->value.dp_str);
    IPC_APP_set_patrol_time((cJSON *)(p_dp_json->value.dp_str));
    return ;
}
#endif
#ifdef TUYA_DP_PATROL_STATE
STATIC VOID handle_DP_patrol_state(IN TY_OBJ_DP_S *p_dp_json)
{
    int patrol_state = 0;
    //PR_INFO("---get_patrol_state\n");
    IPC_APP_patrol_state(&patrol_state);
    PR_INFO("---get_patrol_state:%d\n",patrol_state);

    CHAR_T sd_mode[4];
    snprintf(sd_mode,4,"%d",patrol_state);
    respone_dp_enum(TUYA_DP_PATROL_STATE, sd_mode);
    return ;
}
#endif

#ifdef TUYA_DP_PRESET_SET
STATIC VOID handle_DP_SET_PRESET(IN TY_OBJ_DP_S *p_dp_json)
{

    IPC_APP_set_preset((cJSON *)(p_dp_json->value.dp_str));
    return;
}
#endif


#ifdef TUYA_DP_DOOR_BELL
STATIC VOID handle_DP_DOOR_BELL(IN TY_OBJ_DP_S *p_obj_dp)
{
    PR_INFO("error! door bell can only trigged by IPC side.\r\n");
    respone_dp_str(TUYA_DP_DOOR_BELL, "-1");
}
#endif

#ifdef TUYA_DP_BLUB_SWITCH
STATIC VOID handle_DP_BLUB_SWITCH(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T blub_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_blub_onoff(blub_on_off);
    blub_on_off = IPC_APP_get_blub_onoff();

    respone_dp_bool(TUYA_DP_BLUB_SWITCH, blub_on_off);
}
#endif

#ifdef TUYA_DP_SOUND_DETECT
STATIC VOID handle_DP_SOUND_DETECT(IN TY_OBJ_DP_S *p_obj_dp)
{
    BOOL_T sound_detect_on_off = check_dp_bool_invalid(p_obj_dp);

    IPC_APP_set_sound_detect_onoff(sound_detect_on_off);
    sound_detect_on_off = IPC_APP_get_sound_detect_onoff();

    respone_dp_bool(TUYA_DP_SOUND_DETECT, sound_detect_on_off);
}
#endif

#ifdef TUYA_DP_SOUND_SENSITIVITY
STATIC VOID handle_DP_SOUND_SENSITIVITY(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    CHAR_T tmp_str[2] = { 0 };
    tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

    IPC_APP_set_sound_sensitivity(tmp_str);
    CHAR_T *p_sensitivity = IPC_APP_get_sound_sensitivity();

    respone_dp_enum(TUYA_DP_SOUND_SENSITIVITY, p_sensitivity);
}
#endif

#ifdef TUYA_DP_ELECTRICITY
STATIC VOID handle_DP_ELECTRICITY(IN TY_OBJ_DP_S *p_obj_dp)
{
    INT_T percent = IPC_APP_get_battery_percent();
    PR_INFO("current battery percent:%d\r\n", percent);
    respone_dp_value(TUYA_DP_ELECTRICITY, percent);
}
#endif

#ifdef TUYA_DP_POWERMODE
STATIC VOID handle_DP_POWERMODE(IN TY_OBJ_DP_S *p_obj_dp)
{
    CHAR_T *power_mode = IPC_APP_get_power_mode();
    PR_INFO("current power mode:%s\r\n", power_mode);
    respone_dp_enum(TUYA_DP_POWERMODE, power_mode);
}
#endif

#ifdef TUYA_DP_LOWELECTRIC
STATIC VOID handle_DP_LOWELECTRIC(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_VALUE)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    IPC_APP_set_lowelectric(p_obj_dp->value.dp_value);
    int value = IPC_APP_get_lowelectric();

    respone_dp_value(TUYA_DP_LOWELECTRIC, value);
}
#endif

STATIC VOID handle_DP_RESERVED(IN TY_OBJ_DP_S *p_obj_dp)
{
    PR_ERR("error! not implememt yet.\r\n");
}



#if defined(WIFI_GW) && (WIFI_GW==1)
#ifdef TUYA_DP_AP_MODE
STATIC VOID handle_DP_AP_MODE(IN TY_OBJ_DP_S *p_dp_json)
{
    if (p_dp_json == NULL) {
        PR_ERR("Error!! type invalid\r\n");
        return;
    }
    respone_dp_str(TUYA_DP_AP_MODE, IPC_APP_get_ap_mode());
}
#endif

#ifdef TUYA_DP_AP_SWITCH
STATIC VOID handle_DP_AP_SWITCH(IN TY_OBJ_DP_S *p_dp_json)
{
    CHAR_T resp[32] = { 0 };
    INT_T ap_enable = IPC_APP_set_ap_mode((cJSON *)p_dp_json->value.dp_str);
    if (ap_enable < 0) {
        snprintf(resp, 32, "{\"ap_enable\":0,\"errcode\":0}");
    } else {
        snprintf(resp, 32, "{\"ap_enable\":%d,\"errcode\":0}", ap_enable);
    }
    respone_dp_str(TUYA_DP_AP_SWITCH, resp);
    respone_dp_str(TUYA_DP_AP_MODE, IPC_APP_update_ap_mode(ap_enable)); //switch ap mode, need update ap mode value
    if (ap_enable >= 0) {
        change_ap_process();
    }
}
#endif

#ifdef TUYA_DP_AP_TIME_SYNC
STATIC VOID handle_DP_AP_TIME_SYNC(IN TY_OBJ_DP_S *p_dp_json)
{
    PR_INFO("%s get time sync dp\n",__func__);

    IPC_APP_set_time(p_dp_json->value.dp_str);
}
#endif

#ifdef TUYA_DP_AP_TIME_ZONE

STATIC VOID handle_DP_AP_TIME_ZONE(IN TY_OBJ_DP_S *p_dp_json)
{
    PR_INFO("%s get time ZONE sync dp\n",__func__); 
    IPC_APP_set_time_zone(p_dp_json->value.dp_str);
}

#endif
#endif

#ifdef handle_RAW_DP_DEMO_STYLE
STATIC VOID handle_RAW_DP_DEMO_STYLE(TY_RECV_RAW_DP_S * raw_ctx)
{
    PR_INFO("rec raw dp is %s\n",raw_ctx->data);
    // developer do it according to needing
    return;
}
#endif

#ifdef TUYA_DP_MOVE_POINT
STATIC VOID handle_DP_MOVE_POINT(IN TY_OBJ_DP_S *p_dp_json)
{
    if(p_dp_json == NULL ){
        PR_ERR("Error!! type invalid\r\n");
        return;
    }
    INT_T ret = IPC_APP_set_move_point((ty_cJSON *)(p_dp_json->value.dp_str));
    if (0 != ret){
        PR_ERR("set move point failed");
    }else{
        PR_DEBUG("set move point succ");
    }
}
#endif
#ifdef TUYA_DP_MOVE_PATH
STATIC VOID handle_DP_MOVE_PATH(IN TY_OBJ_DP_S *p_dp_json)
{
    if(p_dp_json == NULL ){
        PR_ERR("Error!! type invalid\r\n");
        return;
    }
    INT_T ret = IPC_APP_set_move_path((ty_cJSON *)(p_dp_json->value.dp_str));
    if (0 != ret){
        PR_ERR("set move path failed");
    }else{
        PR_DEBUG("set move path succ");
    }
}
#endif

#ifdef TUYA_DP_DEVICE_VOLUME
STATIC VOID handle_DP_DEVICE_VOLUME(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_VALUE)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    int volume_value = p_obj_dp->value.dp_value;
    PR_INFO("volume value is %d\n", volume_value);

    IPC_APP_set_device_volume(volume_value);

    respone_dp_value(TUYA_DP_DEVICE_VOLUME, p_obj_dp->value.dp_value);
}
#endif

#ifdef TUYA_DP_IPC_CONTRAST
STATIC VOID handle_DP_IPC_CONTRAST(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_VALUE)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    add_md_pending_count();
    int value = p_obj_dp->value.dp_value;
    PR_INFO("contrast value is %d\n", value);

    IPC_APP_set_ipc_contrast(value);
    respone_dp_value(TUYA_DP_IPC_CONTRAST, p_obj_dp->value.dp_value);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_IPC_BRIGHT
STATIC VOID handle_DP_IPC_BRIGHT(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_VALUE)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }
    add_md_pending_count();
    int value = p_obj_dp->value.dp_value;
    PR_INFO("bright value is %d\n", value);

    IPC_APP_set_ipc_bright(value);
    respone_dp_value(TUYA_DP_IPC_BRIGHT, p_obj_dp->value.dp_value);
    remove_md_pending_count();
}
#endif

#ifdef TUYA_DP_IPC_REBOOT
STATIC VOID handle_DP_IPC_REBOOT(IN TY_OBJ_DP_S *p_obj_dp)
{
    respone_dp_bool(TUYA_DP_IPC_REBOOT, TRUE);

    PR_ERR("\nrecv app request restart the ipc\r\n");
    system("reboot");
}
#endif

#ifdef TUYA_DP_FLICKER
STATIC VOID handle_DP_FLICKER(IN TY_OBJ_DP_S *p_obj_dp)
{
    if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
        PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
        return;
    }

    IPC_APP_set_flicker(p_obj_dp->value.dp_enum);
    UINT_T value = IPC_APP_get_flicker();
    CHAR_T sValue[2];
    snprintf(sValue, 2, "%d", value);
    respone_dp_enum(TUYA_DP_FLICKER, sValue);
}
#endif

typedef VOID(*TUYA_DP_HANDLER)(IN TY_OBJ_DP_S *p_obj_dp);
typedef struct {
    BYTE_T dp_id;
    TUYA_DP_HANDLER handler;
}TUYA_DP_INFO_S;

STATIC TUYA_DP_INFO_S s_dp_table[] =
{
#ifdef TUYA_DP_SLEEP_MODE
    {TUYA_DP_SLEEP_MODE,            handle_DP_SLEEP_MODE},
#endif
#ifdef TUYA_DP_LIGHT
    {TUYA_DP_LIGHT,                 handle_DP_LIGHT},
#endif
#ifdef TUYA_DP_FLIP
    {TUYA_DP_FLIP,                  handle_DP_FLIP},
#endif
#ifdef TUYA_DP_WATERMARK
    {TUYA_DP_WATERMARK,             handle_DP_WATERMARK},
#endif
#ifdef TUYA_DP_WDR
    {TUYA_DP_WDR,                   handle_DP_WDR},
#endif
#ifdef TUYA_DP_NIGHT_MODE
    {TUYA_DP_NIGHT_MODE,            handle_DP_NIGHT_MODE},
#endif
#ifdef TUYA_DP_ALARM_FUNCTION
    {TUYA_DP_ALARM_FUNCTION,        handle_DP_ALARM_FUNCTION},
#endif
#ifdef TUYA_DP_ALARM_SENSITIVITY
    {TUYA_DP_ALARM_SENSITIVITY,     handle_DP_ALARM_SENSITIVITY},
#endif

#ifdef TUYA_DP_IPC_RECORDING_TIME
    {TUYA_DP_IPC_RECORDING_TIME,     handle_DP_IPC_RECORDING_TIME},
#endif

#ifdef TUYA_DP_PIR_ALARM_INTERVAL
    {TUYA_DP_PIR_ALARM_INTERVAL,     handle_DP_PIR_ALARM_INTERVAL},
#endif

#ifdef TUYA_DP_IPC_BIRD_DETECT_MODE
    {TUYA_DP_IPC_BIRD_DETECT_MODE,   handle_DP_IPC_BIRD_DETECT_MODE},
#endif

#ifdef TUYA_DP_BIRD_SWITCH
    {TUYA_DP_BIRD_SWITCH,   handle_DP_BIRD_SWITCH},
#endif

#ifdef TUYA_DP_SIREN_SWITCH
    {TUYA_DP_SIREN_SWITCH,   handle_DP_SIREN_SWITCH},
#endif

#ifdef TUYA_DP_SIREN_SQUIRREL
    {TUYA_DP_SIREN_SQUIRREL,   handle_DP_SIREN_SQUIRREL},
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
    {TUYA_DP_ALARM_ZONE_ENABLE,     handle_DP_ALARM_ZONE_ENABLE},
#endif
#ifdef TUYA_DP_ALARM_ZONE_DRAW
    {TUYA_DP_ALARM_ZONE_DRAW,     handle_DP_ALARM_ZONE_DRAW},
#endif
#ifdef TUYA_DP_SD_STATUS_ONLY_GET
    {TUYA_DP_SD_STATUS_ONLY_GET,    handle_DP_SD_STATUS_ONLY_GET},
#endif
#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
    {TUYA_DP_SD_STORAGE_ONLY_GET,   handle_DP_SD_STORAGE_ONLY_GET},
#endif
#ifdef TUYA_DP_SD_RECORD_ENABLE
    {TUYA_DP_SD_RECORD_ENABLE,      handle_DP_SD_RECORD_ENABLE},
#endif
#ifdef TUYA_DP_SD_RECORD_MODE
    {TUYA_DP_SD_RECORD_MODE,        handle_DP_SD_RECORD_MODE},
#endif
#ifdef TUYA_DP_SD_UMOUNT
    {TUYA_DP_SD_UMOUNT,             handle_DP_SD_UMOUNT},
#endif
#ifdef TUYA_DP_SD_FORMAT
    {TUYA_DP_SD_FORMAT,             handle_DP_SD_FORMAT},
#endif
#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
    {TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, handle_DP_SD_FORMAT_STATUS_ONLY_GET},
#endif
#ifdef TUYA_DP_PTZ_CONTROL
    {TUYA_DP_PTZ_CONTROL,           handle_DP_PTZ_CONTROL},
#endif
#ifdef TUYA_DP_PTZ_STOP
    {TUYA_DP_PTZ_STOP,              handle_DP_PTZ_STOP},
#endif
#ifdef TUYA_DP_PTZ_CHECK
    {TUYA_DP_PTZ_CHECK,              handle_DP_PTZ_CHECK},
#endif
#ifdef TUYA_DP_TRACK_ENABLE
    {TUYA_DP_TRACK_ENABLE,           handle_DP_TRACK_ENABLE},
#endif
#ifdef TUYA_DP_HUM_FILTER
    {TUYA_DP_HUM_FILTER,             handle_DP_HUM_FILTER},
#endif
#ifdef TUYA_DP_PATROL_MODE
    {TUYA_DP_PATROL_MODE,            handle_DP_patrol_mode},
#endif
#ifdef TUYA_DP_PATROL_SWITCH
    {TUYA_DP_PATROL_SWITCH,          handle_DP_patrol_switch},
#endif
#ifdef TUYA_DP_PATROL_TMODE
    {TUYA_DP_PATROL_TMODE,           handle_DP_patrol_tmode},
#endif
#ifdef TUYA_DP_PATROL_TIME
    {TUYA_DP_PATROL_TIME,           handle_DP_patrol_time},
#endif
#ifdef TUYA_DP_PATROL_STATE
    {TUYA_DP_PATROL_STATE,           handle_DP_patrol_state},
#endif
#ifdef TUYA_DP_PRESET_SET
    {TUYA_DP_PRESET_SET,              handle_DP_SET_PRESET},
#endif
#ifdef TUYA_DP_LINK_MOVE_ACTION
    {TUYA_DP_LINK_MOVE_ACTION,    handle_DP_LINK_MOVE},
#endif
#ifdef TUYA_DP_LINK_MOVE_SET
    {TUYA_DP_LINK_MOVE_SET,    handle_DP_LINK_MOVE_SET},
#endif
#ifdef TUYA_DP_DOOR_BELL
    {TUYA_DP_DOOR_BELL,             handle_DP_DOOR_BELL},
#endif
#ifdef TUYA_DP_BLUB_SWITCH
    {TUYA_DP_BLUB_SWITCH,           handle_DP_BLUB_SWITCH},
#endif
#ifdef TUYA_DP_SOUND_DETECT
    {TUYA_DP_SOUND_DETECT,          handle_DP_SOUND_DETECT},
#endif
#ifdef TUYA_DP_SOUND_SENSITIVITY
    {TUYA_DP_SOUND_SENSITIVITY,     handle_DP_SOUND_SENSITIVITY},
#endif
#ifdef TUYA_DP_SOUND_ALARM
    {TUYA_DP_SOUND_ALARM,           handle_DP_RESERVED},
#endif
#ifdef TUYA_DP_TEMPERATURE
    {TUYA_DP_TEMPERATURE,           handle_DP_RESERVED},
#endif
#ifdef TUYA_DP_HUMIDITY
    {TUYA_DP_HUMIDITY,              handle_DP_RESERVED},
#endif
#ifdef TUYA_DP_ELECTRICITY
    {TUYA_DP_ELECTRICITY,           handle_DP_ELECTRICITY},
#endif
#ifdef TUYA_DP_POWERMODE
    {TUYA_DP_POWERMODE,             handle_DP_POWERMODE},
#endif
#ifdef TUYA_DP_LOWELECTRIC
    {TUYA_DP_LOWELECTRIC,           handle_DP_LOWELECTRIC},
#endif

#if defined(WIFI_GW) && (WIFI_GW==1)
#ifdef TUYA_DP_AP_MODE
    {TUYA_DP_AP_MODE,               handle_DP_AP_MODE},
#endif
#ifdef TUYA_DP_AP_SWITCH
    {TUYA_DP_AP_SWITCH,             handle_DP_AP_SWITCH},
#endif
#ifdef TUYA_DP_AP_TIME_SYNC
    {TUYA_DP_AP_TIME_SYNC,          handle_DP_AP_TIME_SYNC},
#endif
#ifdef TUYA_DP_AP_TIME_ZONE
    {TUYA_DP_AP_TIME_ZONE,          handle_DP_AP_TIME_ZONE},
#endif
#ifdef TUYA_DP_DEVICE_VOLUME
    {TUYA_DP_DEVICE_VOLUME,         handle_DP_DEVICE_VOLUME},
#endif
#ifdef TUYA_DP_IPC_CONTRAST
    {TUYA_DP_IPC_CONTRAST,         handle_DP_IPC_CONTRAST},
#endif
#ifdef TUYA_DP_IPC_BRIGHT
    {TUYA_DP_IPC_BRIGHT,         handle_DP_IPC_BRIGHT},
#endif
#ifdef TUYA_DP_IPC_REBOOT
    {TUYA_DP_IPC_REBOOT,         handle_DP_IPC_REBOOT},
#endif
#ifdef TUYA_DP_FLICKER
    {TUYA_DP_FLICKER,               handle_DP_FLICKER},
#endif

#endif
};

typedef VOID(*TUYA_RAW_DP_HANDLER)(IN TY_RECV_RAW_DP_S *p_obj_dp);
typedef struct {
    BYTE_T dp_id;
    TUYA_RAW_DP_HANDLER handler;
}TUYA_RAW_DP_INFO_S;

STATIC TUYA_RAW_DP_INFO_S s_raw_dp_table[] =
{
#ifdef TUYA_RAW_DP_DEMO_STYLE
    {TUYA_RAW_DP_DEMO_STYLE,            handle_RAW_DP_DEMO_STYLE},
#endif

};

VOID TUYA_IPC_handle_dp_cmd_objs(IN CONST TY_RECV_OBJ_DP_S *dp_rev)
{
    TY_OBJ_DP_S *dp_data = (TY_OBJ_DP_S *)(dp_rev->dps);
    UINT_T cnt = dp_rev->dps_cnt;
    INT_T table_idx = 0;
    INT_T table_count = (sizeof(s_dp_table) / sizeof(s_dp_table[0]));
    INT_T index = 0;
    for (index = 0; index < cnt; index++) {
        TY_OBJ_DP_S *p_dp_obj = dp_data + index;
        //printf("TUYA_IPC_handle_dp_cmd_objs dp=%d\n",p_dp_obj->dpid);
        for (table_idx = 0; table_idx < table_count; table_idx++) {
            if (s_dp_table[table_idx].dp_id == p_dp_obj->dpid) {
                s_dp_table[table_idx].handler(p_dp_obj);
                break;
            }
        }
    }
}
VOID TUYA_IPC_handle_raw_dp_cmd_objs(IN CONST TY_RECV_RAW_DP_S *dp_rev)
{

    INT_T table_idx = 0;
    INT_T table_count = ( sizeof(s_raw_dp_table) / sizeof(s_raw_dp_table[0]) );
    INT_T index = 0;

    for (table_idx = 0; table_idx < table_count; table_idx++) {
        if (s_raw_dp_table[table_idx].dp_id == dp_rev->dpid) {
            s_raw_dp_table[table_idx].handler(dp_rev);
            break;
        }
    }
}

VOID TUYA_IPC_handle_dp_query_objs(IN CONST TY_DP_QUERY_S *dp_query)
{
    INT_T table_idx = 0;
    INT_T table_count = (sizeof(s_dp_table) / sizeof(s_dp_table[0]));
    INT_T index = 0;
    for (index = 0; index < dp_query->cnt; index++) {
        for (table_idx = 0; table_idx < table_count; table_idx++) {
            if (s_dp_table[table_idx].dp_id == dp_query->dpid[index]) {
                s_dp_table[table_idx].handler(NULL);
                break;
            }
        }
    }
}

#ifdef TUYA_DP_SLEEP_MODE
STATIC OPERATE_RET APP_DP_SLEEP_MODE_PROC(IN CONST BOOL_T dp_value,OUT  BOOL_T * dp_report_value )
{
    IPC_APP_set_sleep_mode(dp_value);
    *dp_report_value = IPC_APP_get_sleep_mode();
    return 0;
}
#endif
#ifdef TUYA_DP_LIGHT
STATIC OPERATE_RET APP_DP_LIGHT_PROC(IN CONST BOOL_T light_on_off,OUT  BOOL_T * dp_report_value)
{

    IPC_APP_set_light_onoff(light_on_off);
    *dp_report_value = IPC_APP_get_light_onoff();
    return  0;
}
#endif

OPERATE_RET APP_DP_BOOL_CMD_PROC(IN CONST INT_T dp_id, IN CONST BOOL_T dp_value,OUT  BOOL_T * dp_report_value )
{
    OPERATE_RET ret = 0;
    switch (dp_id) {
    case TUYA_DP_LIGHT:
    {
        APP_DP_LIGHT_PROC(dp_value, dp_report_value);
        break;
    }
    case TUYA_DP_SLEEP_MODE:
    {
        //用户处理 TUYA_DP_SLEEP_MODE
        ret = APP_DP_SLEEP_MODE_PROC(dp_value, dp_report_value);
        break;
    }
    case TUYA_DP_TRACK_ENABLE:
    {
        //用户处理TUYA_DP_TRACK_ENABLE
        *dp_report_value = dp_value;
        break;
    }
    case TUYA_DP_ALARM_FUNCTION:
    {
        //用户处理移动侦测报警
        //保存配置
    //s	__tuya_app_write_INT("tuya_alarm_on_off", alarm_on_off);
        IPC_APP_set_alarm_function_onoff(dp_value);
        *dp_report_value = dp_value;

            break;
        }
        case TUYA_DP_SD_RECORD_ENABLE:
        {
            //用户处理存储功能设置
            IPC_APP_set_sd_record_onoff(dp_value);
            *dp_report_value = dp_value;
            break;
        }
        default:
        {
            ret = -1;
            break;
        }

    }
    return ret;
}
OPERATE_RET APP_DP_VALUE_CMD_PROC(IN CONST INT_T dp_id, IN CONST INT_T dp_value,OUT  INT_T * dp_report_value)
{
    OPERATE_RET ret = 0;
    switch (dp_id) {
    case TUYA_DP_SD_STATUS_ONLY_GET:
    {
        /* SD card status, VALUE type, 1-normal, 2-anomaly, 3-insufficient space, 4-formatting, 5-no SD card */
        /* Developer needs to return local SD card status */
        *dp_report_value = 1;
        break;
    }
defalut:
    {
        return -1;
        break;
    }
    }
    return ret;
}
OPERATE_RET APP_DP_STR_CMD_PROC(IN CONST INT_T dp_id, IN CONST CHAR_T *dp_value,OUT  CHAR_T * dp_report_value)
{
    OPERATE_RET ret = 0;
    switch (dp_id) {
    case TUYA_DP_SD_STORAGE_ONLY_GET:
    {

            /* Developer needs to return local SD card status */
            UINT_T total = 128 * 1000 * 1000;
            UINT_T used =32 * 1000 * 1000;
            UINT_T empty = total - used;
            //"total capacity|Current usage|remaining capacity"
            //dp_report_value 最大长度256
            snprintf(dp_report_value, 100, "%u|%u|%u", total, used, empty);

            break;
        }
        default:
        {
            ret = -1;
            break;
        }
    }
    return ret;
}

OPERATE_RET APP_DP_ENUM_CMD_PROC(IN CONST INT_T dp_id, IN CONST UINT_T dp_value,OUT  CHAR_T * dp_report_value)
{
    OPERATE_RET ret = 0;
    switch (dp_id) {
    case TUYA_DP_SD_RECORD_MODE:
    {
        IPC_APP_set_sd_record_mode(dp_value);
        UINT_T mode = IPC_APP_get_sd_record_mode();
        snprintf(dp_report_value, 2, "%d", mode);
        break;
    }
    default:
    {
        ret = -1;
        break;
    }
    }
    return ret;
}
OPERATE_RET APP_DP_BITMAP_CMD_PROC(IN CONST INT_T dp_id, IN CONST UINT_T dp_value,OUT  UINT_T * dp_report_value)
{
    OPERATE_RET ret = 0;
    return ret;
}



