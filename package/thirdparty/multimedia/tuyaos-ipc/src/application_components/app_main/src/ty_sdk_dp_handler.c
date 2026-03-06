/*********************************************************************************
*Copyright(C),2015-2020, 
*TUYA 
*www.tuya.comm

*FileName: tuya_ipc_dp_handler.c
*
* File Description：
* 1. DP Point Setting and Acquisition Function API
*
* Developer work：
* 1. Local configuration acquisition and update.
* 2. Set local IPC attributes, such as picture flip, time watermarking, etc.
*    If the function is not supported, leave the function blank.
*
**********************************************************************************/
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_ptz.h"
#include "tuya_ipc_move_path.h"
#include "ty_dp_define.h"
#include "ty_sdk_common.h"

#include "tkl_wifi.h"
#include "tal_video_in.h"
#include "tal_audio.h"
#include "tkl_motor.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <errno.h>
#include "tkl_video_in.h"
#include "tkl_video_enc.h"
#include "tkl_gpio.h"

extern CHAR_T s_ipc_sd_path[128];
STATIC pthread_mutex_t g_config_mutex = PTHREAD_MUTEX_INITIALIZER;

__attribute__((constructor))
STATIC VOID __mutex_init(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_config_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

extern CHAR_T s_ipc_storage[64];

#define CONFIG_LOCK() pthread_mutex_lock(&g_config_mutex)
#define CONFIG_UNLOCK() pthread_mutex_unlock(&g_config_mutex)

STATIC ty_cJSON *__tuya_json_parse(BOOL_T need_lock);
STATIC VOID __tuya_json_write(CHAR_T *buffer);

STATIC ty_cJSON *__tuya_json_parse_safe()
{
    return __tuya_json_parse(TRUE);
}

STATIC VOID __tuya_json_write_safe(CHAR_T *buffer)
{
    CONFIG_LOCK();
    __tuya_json_write(buffer);
    CONFIG_UNLOCK();
}

STATIC VOID __tuya_json_write(CHAR_T *buffer)
{
    FILE *fp = NULL;
    CHAR_T config_path[256] = { 0 };

    if (buffer == NULL) {
        PR_ERR("Buffer is NULL, cannot write configuration");
        return;
    }

    if (strlen(s_ipc_storage) + strlen("tuya_dp_cfg.json") >= sizeof(config_path)) {
        PR_ERR("Storage path too long: %s", s_ipc_storage);
        return;
    }

    snprintf(config_path, sizeof(config_path), "%stuya_dp_cfg.json", s_ipc_storage);

    if (strlen(s_ipc_storage) == 0 || s_ipc_storage[0] != '/') {
        PR_ERR("Invalid storage path: %s", s_ipc_storage);
        return;
    }

    struct stat st = {0};
    if (stat(s_ipc_storage, &st) == -1) {
        if (mkdir(s_ipc_storage, 0755) != 0) {
            PR_ERR("Failed to create directory %s: %s", s_ipc_storage, strerror(errno));
            return;
        }
    }

    fp = fopen(config_path, "w");
    if (fp == NULL) {
        PR_ERR("Error opening file %s: %s", config_path, strerror(errno));
        return;
    }

    size_t len = strlen(buffer);
    size_t written = fwrite(buffer, 1, len, fp);

    if (written != len) {
        PR_ERR("Warning: not all data written to %s, %zu of %zu bytes", config_path, written, len);
    }

    if (fflush(fp) != 0) {
        PR_ERR("Error flushing file %s: %s", config_path, strerror(errno));
        fclose(fp);
        return;
    }

    if (fclose(fp) != 0) {
        PR_ERR("Error closing file %s: %s", config_path, strerror(errno));
    }
}

VOID TUYA_IPC_INIT_DP_CONFIG()
{
    CONFIG_LOCK();

    ty_cJSON *cfg_json = NULL;
    CHAR_T *buffer = NULL;
    INT_T fd = -1;
    CHAR_T config_path[256] = { 0 };

    INT_T sleep_mode = 0;
    INT_T light_switch = 1;
    INT_T flip_switch = 0;
    INT_T watermark_switch = 1;
    INT_T wdr_switch = 0;
    INT_T alarm_switch = 1; //defaule open
    INT_T bird_switch = 0;
    INT_T siren_switch = 0;
    INT_T siren_squirrel = 0;
    INT_T alarm_zone_switch = 0;
    INT_T sd_record_mode = 0;
    INT_T sd_record_switch = 1;
    INT_T blub_switch = 0;
    INT_T ap_switch = 0;
    INT_T player_volume = 85;
    INT_T lowelectric = 20;
    CHAR_T night_mode[8] = "0";
    CHAR_T alarm_sen[8] = "1";
    CHAR_T alarm_int[8] = "2";
    CHAR_T ap_ssid[32] = { 0 };
    CHAR_T ap_passwd[32] = { 0 };
    CHAR_T sound_sen[8] = "0";

    if (strlen(s_ipc_storage) + strlen("tuya_dp_cfg.json") >= sizeof(config_path)) {
        PR_ERR("Storage path too long: %s", s_ipc_storage);
        goto cleanup;
    }

    snprintf(config_path, sizeof(config_path), "%stuya_dp_cfg.json", s_ipc_storage);

    if (strlen(s_ipc_storage) == 0 || s_ipc_storage[0] != '/') {
        PR_ERR("Invalid storage path: %s", s_ipc_storage);
        goto cleanup;
    }

    struct stat st = {0};
    if (stat(s_ipc_storage, &st) == -1) {
        if (mkdir(s_ipc_storage, 0755) != 0) {
            PR_ERR("Failed to create directory %s: %s", s_ipc_storage, strerror(errno));
            goto cleanup;
        }
    }

    if (0 == access(config_path, F_OK)) {
        if (access(config_path, R_OK) != 0) {
            PR_ERR("Config file exists but is not readable: %s", strerror(errno));
            if (unlink(config_path) == 0) {
                PR_INFO("Deleted unreadable config file, will recreate");
            } else {
                PR_ERR("Failed to delete unreadable config file: %s", strerror(errno));
                goto cleanup;
            }
        } else {
            CONFIG_UNLOCK();
            ty_cJSON *test_json = __tuya_json_parse(FALSE);
            CONFIG_LOCK();

            if (test_json) {
                ty_cJSON_Delete(test_json);
                goto cleanup;
            } else {
                PR_ERR("Config file exists but is invalid, will recreate");
                if (unlink(config_path) != 0) {
                    PR_ERR("Failed to delete invalid config file: %s", strerror(errno));
                    goto cleanup;
                }
            }
        }
    }

    fd = open(config_path, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        PR_ERR("create %s failed: %s", config_path, strerror(errno));
        goto cleanup;
    }
    close(fd);
    fd = -1;

    cfg_json = ty_cJSON_CreateObject();
    if (!cfg_json) {
        PR_ERR("Failed to create JSON object");
        goto cleanup;
    }

    ty_cJSON_AddItemToObject(cfg_json, "tuya_sleep_mode", ty_cJSON_CreateNumber(sleep_mode));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_light_onoff", ty_cJSON_CreateNumber(light_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_flip_onoff", ty_cJSON_CreateNumber(flip_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_watermark_onoff", ty_cJSON_CreateNumber(watermark_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_wdr_onoff", ty_cJSON_CreateNumber(wdr_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_alarm_on_off", ty_cJSON_CreateNumber(alarm_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_bird_on_off", ty_cJSON_CreateNumber(bird_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_siren_on_off", ty_cJSON_CreateNumber(siren_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_squirrel_on_off", ty_cJSON_CreateNumber(siren_squirrel));
    ty_cJSON_AddItemToObject(cfg_json, "alarm_zone_on_off", ty_cJSON_CreateNumber(alarm_zone_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_sd_record_mode", ty_cJSON_CreateNumber(sd_record_mode));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_sd_record_on_off", ty_cJSON_CreateNumber(sd_record_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_blub_on_off", ty_cJSON_CreateNumber(blub_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_on_off", ty_cJSON_CreateNumber(ap_switch));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_lowelectric", ty_cJSON_CreateNumber(lowelectric));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_night_mode", ty_cJSON_CreateString(night_mode));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_alarm_sen", ty_cJSON_CreateString(alarm_sen));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_alarm_int", ty_cJSON_CreateString(alarm_int));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_ssid", ty_cJSON_CreateString(ap_ssid));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_passwd", ty_cJSON_CreateString(ap_passwd));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_human_filter", ty_cJSON_CreateNumber(0));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_sound_detect_on_off", ty_cJSON_CreateNumber(0));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_player_volume", ty_cJSON_CreateNumber(player_volume));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_flicker", ty_cJSON_CreateNumber(0));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_pan_pos", ty_cJSON_CreateNumber(0));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_tilt_pos", ty_cJSON_CreateNumber(0));
    ty_cJSON_AddItemToObject(cfg_json, "tuya_sound_sen", ty_cJSON_CreateString(sound_sen));

    buffer = ty_cJSON_Print(cfg_json);
    if (!buffer) {
        PR_ERR("Failed to print JSON object");
        goto cleanup;
    }

    __tuya_json_write(buffer);

cleanup:
    if (fd >= 0) close(fd);
    if (buffer) ty_cJSON_free(buffer);
    if (cfg_json) ty_cJSON_Delete(cfg_json);
    CONFIG_UNLOCK();
}

STATIC ty_cJSON *__tuya_json_parse(BOOL_T need_lock)
{
    CHAR_T buffer[4096] = { 0 };
    INT_T fd = -1;
    CHAR_T config_path[256] = { 0 };
    static atomic_int reinit_count = 0;
    ty_cJSON *cfg_json = NULL;

    if (need_lock) {
        CONFIG_LOCK();
    }

    if (strlen(s_ipc_storage) + strlen("tuya_dp_cfg.json") >= sizeof(config_path)) {
        PR_ERR("Storage path too long: %s", s_ipc_storage);
        goto cleanup;
    }

    snprintf(config_path, sizeof(config_path), "%stuya_dp_cfg.json", s_ipc_storage);

    if (strlen(s_ipc_storage) == 0 || s_ipc_storage[0] != '/') {
        PR_ERR("Invalid storage path: %s", s_ipc_storage);
        goto cleanup;
    }

    fd = open(config_path, O_RDONLY);
    if (fd < 0) {
        PR_ERR("open %s failed: %s", config_path, strerror(errno));

        if (atomic_load(&reinit_count) < 3) {
            PR_INFO("Config file not found, attempting reinitialization (attempt %d)", atomic_load(&reinit_count) + 1);
            atomic_fetch_add(&reinit_count, 1);

            if (need_lock) {
                CONFIG_UNLOCK();
            }
            TUYA_IPC_INIT_DP_CONFIG();
            if (need_lock) {
                CONFIG_LOCK();
            }

            fd = open(config_path, O_RDONLY);
            if (fd < 0) {
                PR_ERR("Reopen after reinitialization failed: %s", strerror(errno));
                goto cleanup;
            }
            atomic_store(&reinit_count, 0);
        } else {
            PR_ERR("Max reinitialization attempts reached");
            atomic_store(&reinit_count, 0);
            goto cleanup;
        }
    }

    ssize_t read_len = read(fd, buffer, sizeof(buffer) - 1);
    if (read_len < 0) {
        PR_ERR("Failed to read from %s: %s", config_path, strerror(errno));
        goto cleanup;
    }
    buffer[read_len] = '\0';

    close(fd);
    fd = -1;

    cfg_json = ty_cJSON_Parse(buffer);
    if (cfg_json == NULL) {
        PR_ERR("Failed to parse JSON data");

        // 输出错误信息（前100字符）
        CHAR_T err_json_snippet[101] = {0};
        strncpy(err_json_snippet, buffer, sizeof(err_json_snippet) - 1);
        PR_ERR("Invalid JSON content: %.100s", err_json_snippet);

        if (atomic_load(&reinit_count) < 3) {
            PR_INFO("JSON parse error, attempting reinitialization (attempt %d)", atomic_load(&reinit_count) + 1);
            atomic_fetch_add(&reinit_count, 1);

            if (need_lock) {
                CONFIG_UNLOCK();
            }
            TUYA_IPC_INIT_DP_CONFIG();

            cfg_json = __tuya_json_parse(FALSE);
            if (need_lock) {
                CONFIG_LOCK();
            }
        } else {
            PR_ERR("Max reinitialization attempts reached");
            atomic_store(&reinit_count, 0);
        }
    } else {
        atomic_store(&reinit_count, 0);
    }

cleanup:
    if (fd >= 0) close(fd);
    if (need_lock) {
        CONFIG_UNLOCK();
    }
    return cfg_json;
}


/* Setting and Getting Integer Local Configuration.
The reference code here is written directly to tmp, requiring developers to replace the key.*/
STATIC VOID __tuya_app_write_INT(CHAR_T *key, INT_T value)
{
    if (!key || strlen(key) == 0) {
        PR_ERR("Invalid key parameter");
        return;
    }

    ty_cJSON *cfg_json = __tuya_json_parse_safe();
    if (NULL == cfg_json) {
        PR_ERR("load configure failed, will try to reinitialize");
        TUYA_IPC_INIT_DP_CONFIG();
        cfg_json = __tuya_json_parse_safe();
        if (NULL == cfg_json) {
            PR_ERR("Failed to reinitialize configure");
            return;
        }
    }

    ty_cJSON_ReplaceItemInObject(cfg_json, key, ty_cJSON_CreateNumber(value));

    CHAR_T *json_tmp = ty_cJSON_Print(cfg_json);
    if (!json_tmp) {
        PR_ERR("Failed to print JSON object");
        ty_cJSON_Delete(cfg_json);
        return;
    }

    __tuya_json_write_safe(json_tmp);
    ty_cJSON_free(json_tmp);
    ty_cJSON_Delete(cfg_json);
}

STATIC INT_T __tuya_app_read_INT(CHAR_T *key)
{
    if (!key || strlen(key) == 0) {
        PR_ERR("Invalid key parameter");
        return OPRT_CJSON_PARSE_ERR;
    }

    int tmp_val = 0;
    ty_cJSON *cfg_json = __tuya_json_parse_safe();
    if (NULL == cfg_json) {
        PR_ERR("load configure failed, will try to reinitialize");
        TUYA_IPC_INIT_DP_CONFIG();
        cfg_json = __tuya_json_parse_safe();
        if (NULL == cfg_json) {
            PR_ERR("Failed to reinitialize configure");
            return OPRT_CJSON_PARSE_ERR;
        }
    }

    ty_cJSON *val = ty_cJSON_GetObjectItem(cfg_json, key);
    if (val) {
        tmp_val = val->valueint;
    } else {
        PR_WARN("Key %s not found in config", key);
    }
    ty_cJSON_Delete(cfg_json);
    return tmp_val;
}

/* Setting and Getting String Local Configuration.
The reference code here is written directly to tmp, requiring developers to replace the key.*/
STATIC VOID __tuya_app_write_STR(CHAR_T *key, CHAR_T *value)
{
    if (!key || strlen(key) == 0) {
        PR_ERR("Invalid key parameter");
        return;
    }

    if (!value) {
        value = "";
    }

    ty_cJSON *cfg_json = __tuya_json_parse_safe();
    if (NULL == cfg_json) {
        PR_ERR("load configure failed, will try to reinitialize");
        TUYA_IPC_INIT_DP_CONFIG();
        cfg_json = __tuya_json_parse_safe();
        if (NULL == cfg_json) {
            PR_ERR("Failed to reinitialize configure");
            return;
        }
    }

    ty_cJSON_ReplaceItemInObject(cfg_json, key, ty_cJSON_CreateString(value));

    CHAR_T *json_tmp = ty_cJSON_Print(cfg_json);
    if (!json_tmp) {
        PR_ERR("Failed to print JSON object");
        ty_cJSON_Delete(cfg_json);
        return;
    }

    __tuya_json_write_safe(json_tmp);
    ty_cJSON_free(json_tmp);
    ty_cJSON_Delete(cfg_json);
}

STATIC INT_T __tuya_app_read_STR(CHAR_T *key, CHAR_T *value, INT_T value_size)
{
    if (!key || strlen(key) == 0 || !value || value_size <= 0) {
        PR_ERR("Invalid parameters");
        return OPRT_CJSON_PARSE_ERR;
    }

    memset(value, 0, value_size);
    ty_cJSON *cfg_json = __tuya_json_parse_safe();
    if (NULL == cfg_json) {
        PR_ERR("load configure failed, will try to reinitialize");
        TUYA_IPC_INIT_DP_CONFIG();
        cfg_json = __tuya_json_parse_safe();
        if (NULL == cfg_json) {
            PR_ERR("Failed to reinitialize configure");
            return OPRT_CJSON_PARSE_ERR;
        }
    }

    ty_cJSON *val = ty_cJSON_GetObjectItem(cfg_json, key);
    if (val) {
        strncpy(value, val->valuestring, value_size);
        value[value_size - 1] = '\0';
    } else {
        PR_WARN("Key %s not found in config", key);
    }

    ty_cJSON_Delete(cfg_json);
    return 0;
}

#ifdef TUYA_DP_SLEEP_MODE
VOID IPC_APP_set_sleep_mode(BOOL_T sleep_mode)
{
    PR_INFO("set sleep_mode:%d \r\n", sleep_mode);
    //TODO
    /* sleep mode,BOOL type,true means sleep,false means wake */
    if (sleep_mode == true) {
        tuya_ipc_media_service_pause();
    } else {
        tuya_ipc_media_service_resume();
    }

    __tuya_app_write_INT("tuya_sleep_mode", sleep_mode);
}

BOOL_T IPC_APP_get_sleep_mode(VOID)
{
    BOOL_T sleep_mode = __tuya_app_read_INT("tuya_sleep_mode");
    PR_INFO("curr sleep_mode:%d \r\n", sleep_mode);
    return sleep_mode;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_LIGHT
VOID IPC_APP_set_light_onoff(BOOL_T light_on_off)
{
    PR_INFO("set light_on_off:%d \r\n", light_on_off);
    //TODO
    /* Status indicator,BOOL type,true means on,false means off */

    __tuya_app_write_INT("tuya_light_onoff", light_on_off);
}

BOOL_T IPC_APP_get_light_onoff(VOID)
{
    BOOL_T light_on_off = __tuya_app_read_INT("tuya_light_onoff");
    PR_INFO("curr light_on_off:%d \r\n", light_on_off);
    ledkey_set_mode((light_on_off==0) ? led_keep_off : led_online);
    return light_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_FLIP
VOID IPC_APP_set_flip_onoff(BOOL_T flip_on_off)
{
    int ret;
    PR_INFO("set flip_on_off:%d \r\n", flip_on_off);

    TKL_VI_MIRROR_FLIP_E val = TKL_VI_MIRROR_FLIP_NONE;
    if (flip_on_off == 0) {
        val = TKL_VI_MIRROR_FLIP_NONE;
    } else {
        val = TKL_VI_MIRROR_FLIP;
    }

    ret = tkl_vi_set_mirror_flip(0, val);
    if (ret == 0) {
        __tuya_app_write_INT("tuya_flip_onoff", flip_on_off);
    }
}

BOOL_T IPC_APP_get_flip_onoff(VOID)
{
    BOOL_T flip_on_off = __tuya_app_read_INT("tuya_flip_onoff");
    PR_INFO("curr flip_on_off:%d \r\n", flip_on_off);
    return flip_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_WATERMARK
VOID IPC_APP_set_watermark_onoff(BOOL_T watermark_on_off)
{
    PR_INFO("set watermark_on_off:%d \r\n", watermark_on_off);
    int ret = 0;
    TKL_VENC_OSD_T posd;

    if ((watermark_on_off == 1)) {
        posd.enable = 1;
        ret = tkl_venc_set_osd(0, 0, &posd);
    } else {
        posd.enable = 0;
        ret = tkl_venc_set_osd(0, 0, &posd);
    }

    if (ret == 0) {
        __tuya_app_write_INT("tuya_watermark_onoff", watermark_on_off);
    }
}

BOOL_T IPC_APP_get_watermark_onoff(VOID)
{
    BOOL_T watermark_on_off = __tuya_app_read_INT("tuya_watermark_onoff");
    PR_INFO("curr watermark_on_off:%d \r\n", watermark_on_off);
    return watermark_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_WDR
VOID IPC_APP_set_wdr_onoff(BOOL_T wdr_on_off)
{
    PR_INFO("set wdr_on_off:%d \r\n", wdr_on_off);
    //TODO
    /* Wide Dynamic Range Model,BOOL type,true means on,false means off */

    __tuya_app_write_INT("tuya_wdr_onoff", wdr_on_off);

}

BOOL_T IPC_APP_get_wdr_onoff(VOID)
{
    BOOL_T wdr_on_off = __tuya_app_read_INT("tuya_wdr_onoff");
    PR_INFO("curr watermark_on_off:%d \r\n", wdr_on_off);
    return wdr_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_NIGHT_MODE
#ifdef LOW_BAT_IPC
#define ADC0_THRESHOLD 50
// #define ADC0_THRESHOLD 500 //dual hw
#define HW_SUPPORT_LDR
#endif
STATIC CHAR_T s_night_mode[4] = { 0 };//for demo
STATIC pthread_t check_night_pid = -1;
STATIC BOOL_T is_enable_auto_check;

#ifndef HW_SUPPORT_LDR

#define DAY_TO_IRNIGHT_THRESHOLD 180
#define IRNIGHT_TO_DAY_THRESHOLD 290
#define IR_AWB_GAIN_THRESHOLD -210

#define DAY_TO_NIGHT_SIGNAL_CNT 3
#define NIGHT_TO_DAY_SIGNAL_CNT 3

#define ISP_DEV 0

#define TURN_TO_DAY 1
#define TURN_TO_NIGHT 2
/*
* check day or night by ISP
*/
static int get_day_or_night_status(void)
{
    static unsigned int day2ir_night_signal_cnt = 0;
    static unsigned int ir_night2day_signal_cnt = 0;
    int awb_rgain_ir, awb_bgain_ir;
    int ret;

    ret = AW_MPI_ISP_GetEnvLV(ISP_DEV);  //get sensor brightness
    AW_MPI_ISP_GetAwbGainIr(ISP_DEV, &awb_rgain_ir, &awb_bgain_ir);
    // PR_INFO("###get LV:%d, awb_rgain_ir: %d, awb_bgain_ir: %d\n", ret, awb_rgain_ir, awb_bgain_ir);

    if (tkl_vi_get_day_night_mode() == 1) { // ir_night --> day
        if (ret >= IRNIGHT_TO_DAY_THRESHOLD) {
        // if ((ret > IRNIGHT_TO_DAY_THRESHOLD) && ((awb_rgain_ir + awb_bgain_ir) < (IR_AWB_GAIN_THRESHOLD * 2))) {
            PR_INFO("--------> get LV:%d>=%d, awb_rgain_ir: %d, awb_bgain_ir: %d\n", ret, IRNIGHT_TO_DAY_THRESHOLD, awb_rgain_ir, awb_bgain_ir);
            PR_INFO("--------> ir_night --> day, ir_night2day_signal_cnt = %d\n", ir_night2day_signal_cnt);
            if (++ir_night2day_signal_cnt >= NIGHT_TO_DAY_SIGNAL_CNT) {
                ir_night2day_signal_cnt = 0;
                return TURN_TO_DAY;
            }
        } else {
            ir_night2day_signal_cnt = 0;
        }
    } else { // day --> night
        if (ret <= DAY_TO_IRNIGHT_THRESHOLD) {
            PR_INFO("--------> get LV:%d<=%d, awb_rgain_ir: %d, awb_bgain_ir: %d\n", DAY_TO_IRNIGHT_THRESHOLD, ret, awb_rgain_ir, awb_bgain_ir);
            PR_INFO("--------> day --> ir_night, day2ir_night_signal_cnt = %d\n", day2ir_night_signal_cnt);
            if (++day2ir_night_signal_cnt >= DAY_TO_NIGHT_SIGNAL_CNT) {
                day2ir_night_signal_cnt = 0;
                return TURN_TO_NIGHT;
            }
        } else {
            day2ir_night_signal_cnt = 0;
        }
    }

    return 0;
}

static void *__check_night(void *argv)
{
    unsigned int get_status = 0; 
    while(1) {
        if (is_enable_auto_check == 1) {
            get_status = get_day_or_night_status();
            if (get_status == TURN_TO_NIGHT) {
                tkl_vi_set_day_night_mode(1);
            } else if (get_status == TURN_TO_DAY) {
                tkl_vi_set_day_night_mode(0);
            }
        }
        usleep(2 * 1000 * 1000);
    }
}
#else //HW_SUPPORT_LDR

static void *__check_night(void *argv)
{
    FILE *file;
    char buffer[64];
    unsigned int gpadc0_value = 0;

    file = fopen("/sys/class/gpadc/gpadc_chip0/data", "r");
    if (file == NULL) {
        PR_ERR("Error opening file\r\n");
        return;
    }

    while (1) {
        if (is_enable_auto_check == 1) {
            memset(buffer, 0, sizeof(buffer));
            //gpadc0-channel0 voltage data is 1795
            fseek(file, 32, SEEK_SET);
            if (fgets(buffer, sizeof(buffer), file) != NULL) {
                gpadc0_value = atoi(buffer);
                // PR_ERR("gpadc0_value:%d \r\n", gpadc0_value);
        #ifdef IPC_DUAL
                        if (gpadc0_value > ADC0_THRESHOLD) {
        #else
                        if (gpadc0_value < ADC0_THRESHOLD) {
        #endif
                    tkl_vi_set_day_night_mode(1);
                } else {
                    tkl_vi_set_day_night_mode(0);
                }
            }
        }
        usleep(2 * 1000 * 1000);
    }
    fclose(file);
}
#endif //HW_SUPPORT_LDR

int creat_check_night_thread(void)
{
    int result = 0;
    result = pthread_create(&check_night_pid, NULL, __check_night, NULL);
    if (result != 0) {
        PR_ERR("fatal error! pthread create fail[%d]\r\n", result);
        return -1;
    }
    return 0;
}

VOID IPC_APP_set_night_mode(CHAR_T * p_night_mode)
{//0-automatic 1-off 2-on
    PR_INFO("set night_mode:%s \r\n", p_night_mode);
    int int_night_mode = atoi(p_night_mode);

    switch (int_night_mode) {
    case 0:
        is_enable_auto_check = 1;
        break;
    case 1:
        is_enable_auto_check = 0;
        tkl_vi_set_day_night_mode(0); //off
        break;
    case 2:
        is_enable_auto_check = 0;
        tkl_vi_set_day_night_mode(1); //on
        break;
    default:
        PR_INFO("not support,int_night_mode:%d \r\n", int_night_mode);
        break;
    }

    __tuya_app_write_STR("tuya_night_mode", p_night_mode);
}

CHAR_T *IPC_APP_get_night_mode(VOID)
{
    __tuya_app_read_STR("tuya_night_mode", s_night_mode, 4);
    PR_INFO("curr IPC_APP_get_night_mode:%s  \r\n", s_night_mode );
    return  s_night_mode;
}

#endif


//------------------------------------------

#ifdef TUYA_DP_SIREN_SWITCH
static uint8_t _siren_on_off = 0;
VOID IPC_APP_set_siren_onoff(BOOL_T siren_on_off)
{
    PR_INFO("set tuya_siren_on_off:%d \r\n", siren_on_off);
    _siren_on_off = siren_on_off;
}
BOOL_T IPC_APP_get_siren_onoff(VOID)
{
    BOOL_T siren_on_off = _siren_on_off;
    // PR_INFO("curr siren_on_off:%d \r\n", siren_on_off);
    return siren_on_off;
}
#endif

#ifdef TUYA_DP_SIREN_SQUIRREL
VOID IPC_APP_set_squirrel_onoff(BOOL_T squirrel_on_off)
{
    PR_INFO("set squirrel_on_off:%d \r\n", squirrel_on_off);
    __tuya_app_write_INT("tuya_squirrel_on_off", squirrel_on_off);
}
BOOL_T IPC_APP_get_squirrel_onoff(VOID)
{
    BOOL_T squirrel_on_off = __tuya_app_read_INT("tuya_squirrel_on_off");
    PR_INFO("curr squirrel_on_off:%d \r\n", squirrel_on_off);
    return squirrel_on_off;
}
#endif

#ifdef TUYA_DP_BIRD_SWITCH
VOID IPC_APP_set_bird_onoff(BOOL_T bird_on_off)
{
    PR_INFO("set tuya_bird_on_off:%d \r\n", bird_on_off);
    __tuya_app_write_INT("tuya_bird_on_off", bird_on_off);
}
BOOL_T IPC_APP_get_bird_onoff(VOID)
{
    BOOL_T bird_on_off = __tuya_app_read_INT("tuya_bird_on_off");
    PR_INFO("curr bird_on_off:%d \r\n", bird_on_off);
    return bird_on_off;
}
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
VOID IPC_APP_set_alarm_function_onoff(BOOL_T alarm_on_off)
{
    int ret = 0;
    PR_INFO("set alarm_on_off:%d \r\n", alarm_on_off);
    /* motion detection alarm switch,BOOL type,true means on,false means off.
    * This feature has been implemented, and developers can make local configuration settings and properties.*/
    /* 涂鸦要求移动检测功能常开，该状态用于控制是否报警 */
    // if (alarm_on_off == true) {
    //     ret = tkl_vi_detect_start(0, TKL_MEDIA_DETECT_TYPE_MOTION);
    // } else {
    //     ret = tkl_vi_detect_stop(0, TKL_MEDIA_DETECT_TYPE_MOTION);
    // }

    if (ret == 0) {
        __tuya_app_write_INT("tuya_alarm_on_off", alarm_on_off);
    }

    //当关闭移动检测，同时把人形过滤关闭掉
    if (alarm_on_off == false) {
        ret = tkl_vi_detect_stop(0, TKL_MEDIA_DETECT_TYPE_HUMAN);
        __tuya_app_write_INT("tuya_human_filter", false);
    }
}

BOOL_T IPC_APP_get_alarm_function_onoff(VOID)
{
    BOOL_T alarm_on_off = __tuya_app_read_INT("tuya_alarm_on_off");
    BOOL_T bird_on_off = __tuya_app_read_INT("tuya_bird_on_off");
    // PR_INFO("curr alarm_on_off:%d,  bird_on_off:%d\r\n", alarm_on_off, bird_on_off);
    return (alarm_on_off||bird_on_off) ? 1 : 0;
}

BOOL_T IPC_APP_get_alarm_function_onoff_only(VOID)
{
    BOOL_T alarm_on_off = __tuya_app_read_INT("tuya_alarm_on_off");
    // PR_INFO("curr alarm_on_off:%d,  bird_on_off:%d\r\n", alarm_on_off, bird_on_off);
    return (alarm_on_off) ? 1 : 0;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ALARM_SENSITIVITY
STATIC CHAR_T s_alarm_sensitivity[4] = {0};//for demo
VOID IPC_APP_set_alarm_sensitivity(CHAR_T *p_sensitivity)
{
    PR_INFO("set alarm_sensitivity:%s \r\n", p_sensitivity);
    //TODO
    /* Motion detection alarm sensitivity,ENUM type,0 means low sensitivity,1 means medium sensitivity,2 means high sensitivity*/
    TKL_VI_DETECT_PARAM_T param;
    param.sensitivity = atoi(p_sensitivity);
    tkl_vi_detect_set(0, TKL_MEDIA_DETECT_TYPE_MOTION, &param);

    __tuya_app_write_STR("tuya_alarm_sen", p_sensitivity);
}

CHAR_T *IPC_APP_get_alarm_sensitivity(VOID)
{
    __tuya_app_read_STR("tuya_alarm_sen", s_alarm_sensitivity, 4);
    PR_INFO("curr alarm_sensitivity:%s \r\n", s_alarm_sensitivity);
    return s_alarm_sensitivity;
}
#endif

#ifdef TUYA_DP_PIR_ALARM_INTERVAL
STATIC CHAR_T s_alarm_interval[4] = { 0 };//for demo
VOID IPC_APP_set_alarm_interval(CHAR_T * p_interval)
{
    PR_INFO("set alarm_interval:%s \r\n", p_interval);
    __tuya_app_write_STR("tuya_alarm_int", p_interval);
}

CHAR_T *IPC_APP_get_alarm_interval(VOID)
{
    __tuya_app_read_STR("tuya_alarm_int", s_alarm_interval, 4);
    PR_INFO("curr alarm_interval:%s \r\n", s_alarm_interval);
    return s_alarm_interval;
}
#endif

#ifdef TUYA_DP_LOWELECTRIC
VOID IPC_APP_set_lowelectric(INT_T value)
{
    PR_INFO("set lowelectric:%d \r\n", value);
    __tuya_app_write_INT("tuya_lowelectric", value);
}

INT_T IPC_APP_get_lowelectric(VOID)
{
    int value = __tuya_app_read_INT("tuya_lowelectric");
    PR_INFO("curr lowelectric:%d \r\n", value);
    return value;
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
VOID IPC_APP_set_alarm_zone_onoff(BOOL_T alarm_zone_on_off)
{
    /* Motion detection area setting switch,BOOL type,true means on,false is off*/
    PR_INFO("set alarm_zone_onoff:%d \r\n", alarm_zone_on_off);

    /* This feature has been implemented, and developers can make local configuration settings and properties.*/

    __tuya_app_write_INT("alarm_zone_on_off", alarm_zone_on_off);

}

BOOL_T IPC_APP_get_alarm_zone_onoff(VOID)
{
    BOOL_T alarm_zone_on_off = __tuya_app_read_INT("alarm_zone_on_off");
    PR_INFO("curr alarm_on_off:%d \r\n", alarm_zone_on_off);
    return alarm_zone_on_off;
}
#endif


#ifdef TUYA_DP_ALARM_ZONE_DRAW

#define MAX_ALARM_ZONE_NUM      (6)     //Supports the maximum number of detection areas
//Detection area structure
typedef struct{
    char pointX;    //Starting point x  [0-100]
    char pointY;    //Starting point Y  [0-100]
    char width;     //width    [0-100]
    char height;    //height    [0-100]
}ALARM_ZONE_T;

typedef struct{
    int iZoneNum;   //Number of detection areas
    ALARM_ZONE_T alarmZone[MAX_ALARM_ZONE_NUM];
}ALARM_ZONE_INFO_T;


VOID IPC_APP_set_alarm_zone_draw(cJSON * p_alarm_zone)
{
    if (NULL == p_alarm_zone){
        return ;
    }
    #if 0
    /*demo code*/
    /*Motion detection area setting switch*/
    PR_INFO("%s %d set alarm_zone_set:%s \r\n",__FUNCTION__,__LINE__, (char *)p_alarm_zone);
    ALARM_ZONE_INFO_T strAlarmZoneInfo;
    INT_T i = 0;
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_alarm_zone);

    if (NULL == pJson){
        PR_INFO("%s %d step error\n",__FUNCTION__,__LINE__);
        //free(pResult);
        return;
    }
    cJSON * tmp = cJSON_GetObjectItem(pJson, "num");
    if (NULL == tmp){
        PR_INFO("%s %d step error\n",__FUNCTION__,__LINE__);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
    strAlarmZoneInfo.iZoneNum = tmp->valueint;
    PR_INFO("%s %d step num[%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.iZoneNum);
    if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM){
        PR_INFO("#####error zone num too big[%d]\n",strAlarmZoneInfo.iZoneNum);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++){
        char region[12] = {0};
        cJSON * cJSONRegion = NULL;
        snprintf(region, 12, "region%d",i);
        cJSONRegion = cJSON_GetObjectItem(pJson, region);
        if (NULL == cJSONRegion){
            PR_INFO("#####[%s][%d]error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            //free(pResult);
            return;
        }
        strAlarmZoneInfo.alarmZone[i].pointX = cJSON_GetObjectItem(cJSONRegion, "x")->valueint;
        strAlarmZoneInfo.alarmZone[i].pointY = cJSON_GetObjectItem(cJSONRegion, "y")->valueint;
        strAlarmZoneInfo.alarmZone[i].width = cJSON_GetObjectItem(cJSONRegion,  "xlen")->valueint;
        strAlarmZoneInfo.alarmZone[i].height = cJSON_GetObjectItem(cJSONRegion, "ylen")->valueint;
        PR_INFO("#####[%s][%d][%d,%d,%d,%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.alarmZone[i].pointX,\
            strAlarmZoneInfo.alarmZone[i].pointY,strAlarmZoneInfo.alarmZone[i].width,strAlarmZoneInfo.alarmZone[i].height);
    }
    cJSON_Delete(pJson);
    //free(pResult);
    #endif
    return ;
}

VOID IPC_APP_get_alarm_zone_draw(char * p_alarm_zone)
{
    /*demo code*/
    int i;
    ALARM_ZONE_INFO_T strAlarmZoneInfo;

    memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
    strAlarmZoneInfo.iZoneNum = 1;
    strAlarmZoneInfo.alarmZone[0].pointX = 0;
    strAlarmZoneInfo.alarmZone[0].pointY = 0;
    strAlarmZoneInfo.alarmZone[0].width = 100;
    strAlarmZoneInfo.alarmZone[0].height = 100;
    /*get param of alarmzoneInfo yourself*/
    if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM){
        PR_ERR("[%s] [%d ]get iZoneNum[%d] error",__FUNCTION__,__LINE__,strAlarmZoneInfo.iZoneNum);
        return ;
    }
    for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++){
        char region[64] = {0};
        //{"169":"{\"num\":1,\"region0\":{\"x\":0,\"y\":0,\"xlen\":50,\"ylen\":50}}"}
        if (0 == i){
            snprintf(p_alarm_zone, 256,"{\"num\":%d",strAlarmZoneInfo.iZoneNum);
        }
        snprintf(region, 64, ",\"region%d\":{\"x\":%d,\"y\":%d,\"xlen\":%d,\"ylen\":%d}",i,strAlarmZoneInfo.alarmZone[i].pointX,\
            strAlarmZoneInfo.alarmZone[i].pointY,strAlarmZoneInfo.alarmZone[i].width,strAlarmZoneInfo.alarmZone[i].height);
        strcat(p_alarm_zone, region);
        if(i == (strAlarmZoneInfo.iZoneNum - 1)){
            strcat(p_alarm_zone, "}");
        }
    }
    return ;
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_status(VOID)
{
    INT_T sd_status = 1;
    /* SD card status, VALUE type, 1-normal, 2-anomaly, 3-insufficient space, 4-formatting, 5-no SD card */
    /* Developer needs to return local SD card status */
    sd_status = tuya_ipc_sd_get_status();

    PR_INFO("curr sd_status:%d \r\n", sd_status);
    return sd_status;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_get_sd_storage(UINT_T *p_total, UINT_T *p_used, UINT_T *p_empty)
{//unit is kb
    /* Developer needs to return local SD card status */
    *p_total = 128 * 1000 * 1000;
    *p_used = 32 * 1000 * 1000;
    *p_empty = *p_total - *p_used;

    tuya_ipc_sd_get_capacity_by_mount_path(s_ipc_sd_path, p_total, p_used, p_empty);

    PR_INFO("curr sd total:%u used:%u empty:%u \r\n", *p_total, *p_used, *p_empty);
}
#endif
//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_MODE
VOID IPC_APP_set_sd_record_mode(UINT_T sd_record_mode)
{
    PR_INFO("set sd_record_mode:%d \r\n", sd_record_mode);
    if (0 == sd_record_mode) {
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);
    } else if (1 == sd_record_mode) {
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
    } else {
        PR_INFO("Error, should not happen\r\n");
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
    }
    __tuya_app_write_INT("tuya_sd_record_mode", sd_record_mode);
}

UINT_T IPC_APP_get_sd_record_mode(VOID)
{
    BOOL_T sd_record_mode = __tuya_app_read_INT("tuya_sd_record_mode");
    PR_TRACE("curr sd_record_mode:%d \r\n", sd_record_mode);
    return sd_record_mode;
}
#endif
//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_ENABLE
BOOL_T IPC_APP_get_sd_record_onoff(VOID)
{
    BOOL_T sd_record_on_off  = __tuya_app_read_INT("tuya_sd_record_on_off");
    PR_TRACE("curr sd_record_on_off:%d \r\n", sd_record_on_off);
    return sd_record_on_off;
}
VOID IPC_APP_set_sd_record_onoff(BOOL_T sd_record_on_off)
{
    PR_INFO("set sd_record_on_off:%d \r\n", sd_record_on_off);
    /* SD card recording function swith, BOOL type, true means on, false means off.
    * This function has been implemented, and developers can make local configuration settings and properties.*/

    if (sd_record_on_off == TRUE) {
        IPC_APP_set_sd_record_mode(IPC_APP_get_sd_record_mode());
    } else {
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
    }

    __tuya_app_write_INT("tuya_sd_record_on_off", sd_record_on_off);
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_UMOUNT
BOOL_T IPC_APP_unmount_sd_card(VOID)
{
    BOOL_T umount_ok = TRUE;

    //TODO
    /* unmount sdcard */

    PR_INFO("unmount result:%d \r\n", umount_ok);
    return umount_ok;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_FORMAT
/* -2000: SD card is being formatted, -2001: SD card formatting is abnormal, -2002: No SD card, 
-2003: SD card error. Positive number is formatting progress */
STATIC INT_T s_sd_format_progress = 0;
void *thread_sd_format(void *arg)
{
    /* report format status 4*/
    IPC_APP_report_sd_status_changed(4);

    /* First notify to app, progress 0% */
    s_sd_format_progress = 0;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tal_system_sleep(1000);

    /* Stop local SD card recording and playback, progress 10%*/
    s_sd_format_progress = 10;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
    tuya_ipc_ss_pb_stop_all();
    tal_system_sleep(1000);

    /* Delete the media files in the SD card, the progress is 30% */
    s_sd_format_progress = 30;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tuya_ipc_ss_delete_all_files();
    tal_system_sleep(1000);

    /* Perform SD card formatting operation */
    tuya_ipc_sd_format();

    s_sd_format_progress = 80;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    //TODO
    // tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
//    IPC_APP_set_sd_record_onoff( IPC_APP_get_sd_record_onoff());

    tal_system_sleep(1000);
    IPC_APP_report_sd_storage();
    /* progress 100% */
    s_sd_format_progress = 100;
    IPC_APP_report_sd_format_status(s_sd_format_progress);

    /* report format status 1*/
    INT_T sd_status = IPC_APP_get_sd_status();
    IPC_APP_report_sd_status_changed(sd_status);

    pthread_exit(0);
}

VOID IPC_APP_format_sd_card(VOID)
{
    PR_INFO("start to format sd_card \r\n");
    /* SD card formatting.
    * The SDK has already completed the writing of some of the code, 
    and the developer only needs to implement the formatting operation. */

    pthread_t sd_format_thread;
    pthread_create(&sd_format_thread, NULL, thread_sd_format, NULL);
    pthread_detach(sd_format_thread);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_format_status(VOID)
{
    return s_sd_format_progress;
}
#endif

//------------------------------------------
VOID __ptz_pos_update(VOID)
{
    int pan_pos = 0;
    int tilt_pos = 0;

    tkl_motor_get_state(TKL_MOTOR_STATE_GET_PAN_POS, &pan_pos);
    tkl_motor_get_state(TKL_MOTOR_STATE_GET_TILT_POS, &tilt_pos);

    PR_DEBUG("update pan pos:%d, tilt pos:%d\n", pan_pos, tilt_pos);

    __tuya_app_write_INT("tuya_pan_pos", pan_pos);
    __tuya_app_write_INT("tuya_tilt_pos", tilt_pos);
}

VOID IPC_APP_ptz_pos_recover(VOID)
{
    int res;
    int pan_pos = __tuya_app_read_INT("tuya_pan_pos");
    int tilt_pos = __tuya_app_read_INT("tuya_tilt_pos");

    PR_INFO("pos_recover, pan pos:%d, tilt pos:%d\n", pan_pos, tilt_pos);

    TKL_MOTOR_INIT_PARAM_T param;
#ifdef IPC_DUAL
    param.pan_step_max = 1450;
    param.tilt_step_max = 140;
    param.pan_roll_step = pan_pos;
    param.tilt_roll_step = tilt_pos;
#else
    param.pan_step_max = 512;
    param.tilt_step_max = 100;
    param.pan_roll_step = pan_pos;
    param.tilt_roll_step = tilt_pos;
#endif

    res = tkl_motor_init(&param);
    if (res != 0) {
        PR_ERR("tkl_motor_init failed,%d\n", res);
        return;
    }
}

#ifdef TUYA_DP_PTZ_CONTROL
VOID IPC_APP_ptz_start_move(CHAR_T *p_direction)
{
    PR_INFO("ptz start move:%s \r\n", p_direction);
    //0-up, 1-upper right, 2-right, 3-lower right, 4-down, 5-down left, 6-left, 7-top left
    int int_direction = atoi(p_direction);

    int func_num = int_direction - 1; //根据涂鸦app实际下发的值调整
    TKL_MOTOR_CMD_PARAM_T cmd_param;
    memset(&cmd_param, 0, sizeof(TKL_MOTOR_CMD_PARAM_T));

    cmd_param.pan_step_periods = 4;
    cmd_param.tilt_step_periods = 4;
    switch (func_num) {
    case 0:
        // tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_UP,&cmd_param);
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_DOWN, &cmd_param);//actual
        break;
    case 1:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT_UP, &cmd_param);
        break;
    case 2:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT, &cmd_param);
        break;
    case 3:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT_DOWN, &cmd_param);
        break;
    case 4:
        // tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_DOWN,&cmd_param);
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_UP, &cmd_param);//actual
        break;
    case 5:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT_DOWN, &cmd_param);
        break;
    case 6:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT, &cmd_param);
        break;
    case 7:
        tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT_UP, &cmd_param);
        break;
    default:
        PR_INFO("not support,func_num:%d \r\n", func_num);
        break;
    }
}
#endif

#ifdef TUYA_DP_PTZ_STOP
VOID IPC_APP_ptz_stop_move(VOID)
{
    PR_INFO("ptz stop move \r\n");

    tkl_motor_set_cmd(TKL_MOTOR_CMD_STOP_ALL, NULL);
    __ptz_pos_update();
}
#endif

#ifdef TUYA_DP_PTZ_CHECK
void IPC_APP_ptz_check(VOID)
{
    PR_INFO("ptz check \r\n");
    int res;
    TKL_MOTOR_INIT_PARAM_T param;
#ifdef IPC_DUAL
    param.pan_step_max = 1450;
    param.tilt_step_max = 140;
    param.pan_roll_step = param.pan_step_max / 2;
    param.tilt_roll_step = 10;
#else
    param.pan_step_max = 512;
    param.tilt_step_max = 100;
    param.pan_roll_step = param.pan_step_max / 2;
    param.tilt_roll_step = (100 - 70);
#endif

    PR_INFO("pan_step_max:%d, tilt_step_max:%d, tilt_roll_step:%d\r\n", param.pan_step_max, param.tilt_step_max, param.tilt_roll_step);
    res = tkl_motor_init(&param);
    if (res != 0) {
        PR_ERR("tkl_motor_init failed,%d\n", res);
        return;
    }
    __ptz_pos_update();
}
#endif

#ifdef TUYA_DP_TRACK_ENABLE
void IPC_APP_track_enable(BOOL_T track_enable)
{
    PR_INFO("track_enable %d\r\n",track_enable);
}

BOOL_T IPC_APP_get_track_enable(void)
{
    char track_enable = 0;
    //the value you get yourself
    return (BOOL_T)track_enable;
}

#endif

#ifdef TUYA_DP_HUM_FILTER
void IPC_APP_set_human_filter(BOOL_T filter_enable)
{
    int ret = 0;
    PR_INFO("set filter_enable %d\r\n",filter_enable);

    if (filter_enable == true) {
        ret = tkl_vi_detect_start(0, TKL_MEDIA_DETECT_TYPE_HUMAN);
    } else {
        ret = tkl_vi_detect_stop(0, TKL_MEDIA_DETECT_TYPE_HUMAN);
    }

    if (ret == 0) {
        __tuya_app_write_INT("tuya_human_filter", filter_enable);
    }
    return;
}

BOOL_T IPC_APP_get_human_filter(void)
{
    BOOL_T filter_enable  = __tuya_app_read_INT("tuya_human_filter");
    PR_INFO("get filter_enable %d\r\n",filter_enable);
    return filter_enable;
}
#endif

#ifdef TUYA_DP_PATROL_MODE
void IPC_APP_set_patrol_mode(BOOL_T patrol_mode)
{
    PR_INFO("patrol_mode %d\r\n",patrol_mode);
    return;
}

char IPC_APP_get_patrol_mode(void)
{
    char patrol_mode = 0;
    //the value you get yourself
    return patrol_mode;
}

#endif

#ifdef TUYA_DP_PATROL_SWITCH
void IPC_APP_set_patrol_switch(BOOL_T patrol_switch)
{
    PR_INFO("patrol_switch %d\r\n",patrol_switch);
    return;
}

BOOL_T IPC_APP_get_patrol_switch(void)
{
    char patrol_switch = 0;
    //the value you get yourself
    return (BOOL_T)patrol_switch;
}

void IPC_APP_ptz_preset_reset(S_PRESET_CFG_T *preset_cfg)
{
    /*Synchronize data from server*/
    return;
}

#endif

#ifdef TUYA_DP_PATROL_TMODE
void IPC_APP_set_patrol_tmode(BOOL_T patrol_tmode)
{
    PR_INFO("patrol_tmode %d\r\n",patrol_tmode);
    return;
}

char IPC_APP_get_patrol_tmode(void)
{
    char patrol_tmode = 0;
    //the value you get yourself
    return patrol_tmode;
}

#endif

#ifdef TUYA_DP_PATROL_TIME
void IPC_APP_set_patrol_time(cJSON * p_patrol_time)
{
    //set your patrol_time
    /*

    cJSON * pJson = cJSON_Parse((CHAR_T *)p_patrol_time);
    if (NULL == pJson){
        TYWARN("----error---\n");

        return -1;
    }
    cJSON* t_start = cJSON_GetObjectItem(pJson, "t_start");
    cJSON* t_end = cJSON_GetObjectItem(pJson, "t_end");
    if ((NULL == t_start) || (NULL == t_end)){
        TYWARN("----t_start---\n");
        cJSON_Delete(pJson);
        return -1;
    }
    PR_DEBUG("stare%s--end:%s\n", t_start->valuestring,t_end->valuestring);

    */
    return;
}

#endif

#ifdef TUYA_DP_PRESET_SET
void IPC_APP_set_preset(cJSON * p_preset_param)
{
    //preset add ,preset del, preset go
#if 0
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_preset_param);
    if (NULL == pJson){
        PR_ERR("null preset set input");
        return -1;
    }
    cJSON* type = cJSON_GetObjectItem(pJson, "type");
    cJSON* data = cJSON_GetObjectItem(pJson, "data");
    if ((NULL == type) || (NULL == data)){
        PR_ERR("invalid preset set input");
        return -1;
    }


    PR_DEBUG("preset set type: %d", type->valueint);
    //1:add preset point 2:delete preset point 3:call preset point
    if (type->valueint == 1) {
        char pic_buffer[PRESET_PIC_MAX_SIZE] = { 0 };
        int pic_size = sizeof(pic_buffer);  /*Image to be shown*/
        S_PRESET_POSITION preset_pos;
        char respond_add[128] = { 0 };
        /*mpId is 1,2,3,4,5,6，The server will generate a set of its own preset point number information based on the mpid.*/
        preset_pos.mpId = 1;
        preset_pos.ptz.pan = 100; /*horizontal position*/
        preset_pos.ptz.tilt = 60;/*vertical position*/
        cJSON *name = cJSON_GetObjectItem(data, "name");
        int name_len = 0;
        int error_num = 0;

        if (name == NULL) {
            PR_ERR("name is null\n");
            return -1;
        }
        name_len = strlen(name->valuestring);
        name_len = (name_len > 30) ? 30 : name_len;
        memcpy(&preset_pos.name, name->valuestring, (name_len));
        preset_pos.name[name_len] = '\0';
        error_num = tuya_ipc_preset_add(&preset_pos);

        snprintf(respond_add,128,"{\"type\":%d,\"data\":{\"error\":%d}}",type->valueint,error_num);

        tuya_ipc_dp_report(NULL, TUYA_DP_PRESET_SET,PROP_STR,respond_add,1);

        //tuya_ipc_preset_add_pic(pic_buffer,pic_size); /*if you need show pic ,you should set this api*/
    } else if (type->valueint == 2) {
        cJSON *num = cJSON_GetObjectItem(data, "num"); //can delete one or more
        cJSON *sets = cJSON_GetObjectItem(data, "sets");
        char respond_del[128] = { 0 };
        cJSON *del_data;
        int del_num = num->valueint;
        for (i = 0; i < del_num; i++) {
            del_data = cJSON_GetArrayItem(sets, i);
            cJSON *devId = cJSON_GetObjectItem(del_data, "devId");  /*devid is the preset point number registered in the server*/
            cJSON *mpId = cJSON_GetObjectItem(del_data, "mpId");  /*mpid is the preset point number managed on the device*/
            if ((NULL == devId) || (NULL == mpId)) {
                PR_INFO("devid or mpid is error\n");
                return -1;
            }
            del_preset.seq = atoi(mpId->valuestring);

            PR_INFO("%d---%s\n", del_preset.seq, devId->valuestring);

            error_num = (int)time(NULL);

            tuya_ipc_preset_del(devId->valuestring);

            snprintf(respond_add, 128, "{\"type\":%d,\"data\":{\"error\":%d}}", type->valueint, error_num);
        }
    } else if (type->valueint == 3) {
        cJSON *mpId = cJSON_GetObjectItem(data, "mpId");

        preset_seq = atoi(mpId->valuestring);
        //get your seq pos and go there
    }
#endif
    return;
}

#endif

#ifdef TUYA_DP_PATROL_STATE
void IPC_APP_patrol_state(int *patrol_state)
{
    //PR_INFO("patrol_state %d\r\n",atoi(patrol_state));
    //return your patrol_state
    return;
}

#endif


#ifdef TUYA_DP_LINK_MOVE_SET
VOID IPC_APP_set_link_pos(INT_T bind_seq)
{
    /*set the link pos*/
    PR_INFO("IPC_APP_set_bind_pos:%d \r\n", bind_seq);
    /*demo
    step1: get the current position
    step2: save the position to flash
    */
    return;
}
#endif


#ifdef TUYA_DP_LINK_MOVE_ACTION
VOID IPC_APP_set_link_move(INT_T bind_seq)
{
    /*move to the link pos*/
    PR_INFO("IPC_APP_set_bind_move:%d \r\n", bind_seq);
    /*demo
    step1: get the position base seq
    step2: go to the target position
    */
    return;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_BLUB_SWITCH
#ifdef LOW_BAT_IPC
#define WLED_PORT 113
#else
#define WLED_PORT 359
#endif

VOID IPC_APP_blub_init(VOID)
{
#ifdef LOW_BAT_IPC
    system("echo 113 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio113/direction");
#else
    system("echo 359 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio359/direction");
#endif
}

VOID IPC_APP_set_blub_onoff(BOOL_T blub_on_off)
{
    PR_INFO("set blub_on_off:%d \r\n", blub_on_off);
    OPERATE_RET ret = OPRT_OK;
    if (blub_on_off) {
#ifdef LOW_BAT_IPC
        system("echo 1 > /sys/class/gpio/gpio113/value");
#else
        system("echo 1 > /sys/class/gpio/gpio359/value");
#endif
    } else {
#ifdef LOW_BAT_IPC
        system("echo 0 > /sys/class/gpio/gpio113/value");
#else
        system("echo 0 > /sys/class/gpio/gpio359/value");
#endif
    }
    if (ret != OPRT_OK) {
        PR_ERR("error: tkl_gpio_write failed\n");
    } else {
        __tuya_app_write_INT("tuya_blub_on_off", blub_on_off);
    }
}

BOOL_T IPC_APP_get_blub_onoff(VOID)
{
    BOOL_T blub_on_off = __tuya_app_read_INT("tuya_blub_on_off");
    PR_INFO("curr blub_on_off:%d \r\n", blub_on_off);
    return blub_on_off;
}
#endif

#ifdef TUYA_DP_SOUND_DETECT
VOID IPC_APP_set_sound_detect_onoff(BOOL_T sound_detect_on_off)
{
    PR_INFO("set sound_detect_on_off:%d \r\n", sound_detect_on_off);
    int ret;
    if (sound_detect_on_off == true) {
        ret = tkl_ai_detect_start(0, TKL_MEDIA_DETECT_TYPE_DB);
    } else {
        ret = tkl_ai_detect_stop(0, TKL_MEDIA_DETECT_TYPE_DB);
    }

    if (ret == 0) {
        __tuya_app_write_INT("tuya_sound_detect_on_off", sound_detect_on_off);
    }
}

BOOL_T IPC_APP_get_sound_detect_onoff(VOID)
{
    BOOL_T sound_detect_on_off = __tuya_app_read_INT("tuya_sound_detect_on_off");
    PR_INFO("curr sound_detect_on_off:%d \r\n", sound_detect_on_off);
    return sound_detect_on_off;
}
#endif

#ifdef TUYA_DP_SOUND_SENSITIVITY
STATIC CHAR_T s_sound_sensitivity[4] = { 0 };
VOID IPC_APP_set_sound_sensitivity(CHAR_T * p_sensitivity)
{
    PR_INFO("set sound_sensitivity:%s \r\n", p_sensitivity);
    TKL_VI_DETECT_PARAM_T param;
    param.sensitivity = atoi(p_sensitivity);
    tkl_vi_detect_set(0, TKL_MEDIA_DETECT_TYPE_DB, &param);

    __tuya_app_write_STR("tuya_sound_sen", p_sensitivity);
}

CHAR_T *IPC_APP_get_sound_sensitivity(VOID)
{
    __tuya_app_read_STR("tuya_sound_sen", s_sound_sensitivity, 4);
    PR_INFO("curr sound_sensitivity:%s \r\n", s_sound_sensitivity);
    return s_sound_sensitivity;
}
#endif

#ifdef TUYA_DP_MOVE_POINT
STATIC UINT64_T  tmp_move_point_pathId = 0;
STATIC UINT64_T  tmp_move_point_id = 0;
STATIC BOOL_T    tmp_move_point_task_flag = FALSE;
VOID * move_point_async(VOID * args)
{
    PR_DEBUG("move point task run [%llu---%llu]", tmp_move_point_pathId, tmp_move_point_id);
    //todo  客户再此处控制设备运动到对应点位
    INT_T  cnt = 0;
    while(TRUE == tmp_move_point_task_flag && cnt < 10){
        sleep(1);
        cnt++;
    }
    CHAR_T  return_message[128] = {0};
    memset(return_message, 0x00, sizeof(return_message));
    snprintf(return_message, 128, "{\"type\":6,\"data\":{\"pathId\":%llu,\"id\":%llu,\"error\":0}}",tmp_move_point_pathId, tmp_move_point_id);
    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_POINT,PROP_STR,return_message,1);   
    return NULL;    
}
INT_T IPC_APP_set_move_point(CHAR_T * pData)
{
    if (NULL == pData){
        return -1;
    }
    CHAR_T * pJson = ty_cJSON_Parse(pData);
    ty_cJSON *child = ty_cJSON_GetObjectItem(pJson, "type");
    if (NULL == child){
        PR_ERR("no type value");
        return -1;
    } 
    ty_cJSON *child_data = ty_cJSON_GetObjectItem(pJson, "data");
    CHAR_T  return_message[128] = {0};
    PR_DEBUG("get move path type[%d]", child->valueint);
    switch (child->valueint)
    {
    case 6:
    {
        //运动到对应点位
        ty_cJSON * pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        ty_cJSON * pointId_json = ty_cJSON_GetObjectItem(child_data, "id");
        PR_DEBUG("pathId [%llu] run to point id [%llu]", (UINT64_T)pathId_json->valuedouble, (UINT64_T)pointId_json->valuedouble);
        tmp_move_point_pathId = (UINT64_T)pathId_json->valuedouble;
        tmp_move_point_id = (UINT64_T)pointId_json->valuedouble;
        //异步处理
        pthread_t point_move_thread;
        int ret = pthread_create(&point_move_thread, NULL, move_point_async, NULL);
        if(ret != 0){
        PR_ERR("point_move_thread ,create fail! ret:%d\n",ret);
        }else{
            tmp_move_point_task_flag = TRUE;
            pthread_detach(point_move_thread);
        }
        break;
    }
    case 7:
    {
        //结束点位运行
        ty_cJSON * pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        ty_cJSON * pointId_json = ty_cJSON_GetObjectItem(child_data, "id");
        PR_DEBUG("pathId [%llu] stop run to point id [%llu]", (UINT64_T)pathId_json->valuedouble, (UINT64_T)pointId_json->valuedouble);  
        //todo  客户按照实际情况去结束正在执行的点位运行，如果没有点位正在执行，上报错误
        tmp_move_point_task_flag = FALSE;
        CHAR_T  return_message[128] = {0};
        memset(return_message, 0x00, sizeof(return_message));
        snprintf(return_message, 128, "{\"type\":7,\"data\":{\"pathId\":%llu,\"id\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble, pointId_json->valuedouble);
        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_POINT,PROP_STR,return_message,1);   
        break;
    }
    default:
        PR_ERR("not support [%d]", child->valueint);
        break;
    }

    return 0;
}
#endif

#ifdef TUYA_DP_MOVE_PATH

STATIC UINT64_T  tmp_move_path_task_pathId = 0;
STATIC BOOL_T    tmp_move_path_task_flag = FALSE;
VOID * move_path_async(VOID * args)
{
    PR_DEBUG("move path task run [%llu]", tmp_move_path_task_pathId);
    //todo  客户再此处控制设备按照轨迹执行
    INT_T  cnt = 0;
    while(TRUE == tmp_move_path_task_flag && cnt < 10){
        sleep(1);
        cnt++;
    }
    CHAR_T  return_message[128] = {0};
    memset(return_message, 0x00, sizeof(return_message));
    snprintf(return_message, 128, "{\"type\":3,\"data\":{\"pathId\":%llu,\"error\":0}}",tmp_move_path_task_pathId);
    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);  
    return NULL;     
}
INT_T IPC_APP_set_move_path(CHAR_T * pData)
{
    if (NULL == pData){
        return -1;
    }
    PR_DEBUG("##### recv cmd [%s]",pData);
    CHAR_T * pJson = ty_cJSON_Parse(pData);

    ty_cJSON *child = ty_cJSON_GetObjectItem(pJson, "type");
    if (NULL == child){
        PR_ERR("no type value");
        return -1;
    } 
    ty_cJSON *child_data = ty_cJSON_GetObjectItem(pJson, "data");
    CHAR_T  return_message[128] = {0};
    PR_DEBUG("get move path type[%d]", child->valueint);
    switch (child->valueint)
    {
    case 1:
    {
        //添加轨迹
        ty_cJSON *pathName = ty_cJSON_GetObjectItem(child_data, "pathName");
        PR_DEBUG("add path name [%s]", pathName->valuestring);
        UINT64_T pathId = 0;
        INT_T err_num = 0;
        if (OPRT_OK != tuya_ipc_move_path_add(pathName->valuestring, &pathId)){
            PR_ERR("move path add failed");
            err_num = 101;
        }else{
            //stor pathId to db
            CHAR_T tmp_value[4096] = {0};
            if (0 != __tuya_app_read_STR("tuya_move_path", tmp_value, sizeof(tmp_value))){
                PR_ERR("no move path info ");
                ty_cJSON * root_tmp = ty_cJSON_CreateObject();
                ty_cJSON * move_path_list_json = ty_cJSON_AddArrayToObject(root_tmp, "pathList");
                ty_cJSON * move_path_json = ty_cJSON_CreateObject();
                ty_cJSON_AddNumberToObject(move_path_json, "pathId", pathId);
                ty_cJSON_AddStringToObject(move_path_json, "pathName", pathName->valuestring);
                ty_cJSON_AddItemToArray(move_path_list_json, move_path_json);
                CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
                __tuya_app_write_STR("tuya_move_path", json_buff);
                ty_cJSON_Delete(root_tmp);
                ty_cJSON_FreeBuffer(json_buff);
            }else{
                PR_DEBUG("write data");
                //有数据，直接插入新成员
                ty_cJSON * root_tmp = ty_cJSON_Parse(tmp_value);
                ty_cJSON * move_path_list_json = ty_cJSON_GetObjectItemCaseSensitive(root_tmp, "pathList");
                if (ty_cJSON_IsArray(move_path_list_json)){
                    PR_DEBUG("write data step");
                    ty_cJSON * move_path_json = ty_cJSON_CreateObject();
                    ty_cJSON_AddNumberToObject(move_path_json, "pathId", pathId);
                    ty_cJSON_AddStringToObject(move_path_json, "pathName", pathName->valuestring);
                    ty_cJSON_AddItemToArray(move_path_list_json, move_path_json);
                }else{
                    PR_ERR("data error");
                }
                CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
                __tuya_app_write_STR("tuya_move_path", json_buff);
                ty_cJSON_Delete(root_tmp);
                ty_cJSON_FreeBuffer(json_buff);
                PR_DEBUG("write data over");
            }
            err_num = 0;
        }
        memset(return_message, 0x00, sizeof(return_message));
        if (0 == err_num){
            snprintf(return_message, 128, "{\"type\":1,\"data\":{\"pathId\":%llu,\"error\":%d}}",pathId, err_num);
        }else{
            snprintf(return_message, 128, "{\"type\":1,\"data\":{\"error\":%d}}",err_num);
        }
        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        break;
    }
    case 2:
    {
        //删除轨迹
        INT_T err_num = 0;
        //删除轨迹信息
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        PR_DEBUG("delete pathId [%llu]", (UINT64_T)pathId_json->valuedouble);
        if (OPRT_OK != tuya_ipc_move_path_delete((UINT64_T)pathId_json->valuedouble)){
            PR_ERR("move path delete failed");
            err_num = 1001;
        }else{
            err_num = 0;
        }
        //删除本地保存信息
        CHAR_T tmp_value[4096] = {0};
        if (0 != __tuya_app_read_STR("tuya_move_path", tmp_value, sizeof(tmp_value))){
            PR_ERR("no move path info check process");
        }else{
            //有数据，直接插入新成员
            ty_cJSON * root_tmp = ty_cJSON_Parse(tmp_value);
            ty_cJSON * move_path_list_json = ty_cJSON_GetObjectItemCaseSensitive(root_tmp, "pathList");
            if (ty_cJSON_IsArray(move_path_list_json)){
                ty_cJSON * move_path = NULL;
                INT_T index = 0;
                ty_cJSON_ArrayForEach(move_path, move_path_list_json){
                    ty_cJSON * pathId_tmp = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pathId");
                    if (ty_cJSON_IsNumber(pathId_tmp)&& pathId_tmp->valuedouble == pathId_json->valuedouble){
                        ty_cJSON_DeleteItemFromArray(move_path_list_json, index);
                    }
                    index++;
                }

            }else{
                PR_ERR("data error");
            }
            CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
            __tuya_app_write_STR("tuya_move_path", json_buff);
            ty_cJSON_Delete(root_tmp);
            ty_cJSON_FreeBuffer(json_buff);
        }
        memset(return_message, 0x00, sizeof(return_message));
        snprintf(return_message, 128, "{\"type\":2,\"data\":{\"pathId\":%llu,\"error\":%d}}",(UINT64_T)pathId_json->valuedouble, err_num);
        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        break;
    }
    case 3:
    {
        //执行轨迹
        /* code */
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        PR_DEBUG("do pathId [%llu]", (UINT64_T)pathId_json->valuedouble);
        tmp_move_path_task_pathId = (UINT64_T)pathId_json->valuedouble;
        tmp_move_path_task_flag = TRUE;
        //todo  执行运动轨迹，建议异步处理，不要阻塞此接口
        pthread_t point_path_thread;
        int ret = pthread_create(&point_path_thread, NULL, move_path_async, NULL);
        if(ret != 0){
        PR_ERR("point_move_thread ,create fail! ret:%d\n",ret);
        }else{
            pthread_detach(point_path_thread);
        }
        break;
    }
    case 4:
    {
        //结束轨迹运行，和 type 3对应
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        PR_DEBUG("do pathId [%llu]", (UINT64_T)pathId_json->valuedouble);
        //todo  轨迹执行中断
        tmp_move_path_task_flag = FALSE;
        memset(return_message, 0x00, sizeof(return_message));
        snprintf(return_message, 128, "{\"type\":4,\"data\":{\"pathId\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble);
        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        break;
    }
    case 5:
    {
        //轨迹中添加节点
        INT_T err_num = 0;
        TUYA_IPC_MOVE_POINT_T movePoint = {0};
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        ty_cJSON *pointName_json = ty_cJSON_GetObjectItem(child_data, "pointName");
        PR_DEBUG("pathId [%llu] add point [%s]", (UINT64_T)pathId_json->valuedouble, pointName_json->valuestring);
        //获取pathId下面的所有节点信息
        CHAR_T tmp_value[4096] = {0};
        if (0 != __tuya_app_read_STR("tuya_move_path", tmp_value, sizeof(tmp_value))){
            PR_ERR("no move path info check process");
            memset(return_message, 0x00, sizeof(return_message));
            snprintf(return_message, 128, "{\"type\":5,\"data\":{\"pathId\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble);
            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        }else{
            //有数据，匹配到合适的pathId
            ty_cJSON * root_tmp = ty_cJSON_Parse(tmp_value);
            ty_cJSON * move_path_list_json = ty_cJSON_GetObjectItemCaseSensitive(root_tmp, "pathList");
            if (ty_cJSON_IsArray(move_path_list_json)){
                ty_cJSON * move_path = NULL;
                ty_cJSON * pathId_tmp = NULL;
                ty_cJSON * point_list = NULL;
                BOOL_T matchFlag = FALSE;
                ty_cJSON_ArrayForEach(move_path, move_path_list_json){
                    pathId_tmp = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pathId");
                    if (ty_cJSON_IsNumber(pathId_tmp)&& pathId_tmp->valuedouble == pathId_json->valuedouble){
                        PR_DEBUG("match pathId [%llu], travel point info", (UINT64_T)pathId_tmp->valuedouble);
                        point_list = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pointList");
                        ty_cJSON * point_json = NULL;
                        ty_cJSON_ArrayForEach(point_json, point_list){
                            ty_cJSON * id_json_tmp = ty_cJSON_GetObjectItem(point_json, "id");
                            ty_cJSON * mpId_json_tmp = ty_cJSON_GetObjectItem(point_json, "mpId");
                            PR_DEBUG("point info id-mpId[%llu---%d]", (UINT64_T)id_json_tmp->valuedouble, (UINT64_T)mpId_json_tmp->valueint);
                        }
                        matchFlag = TRUE;
                        break;
                    }
                }
                if (FALSE == matchFlag){
                    memset(return_message, 0x00, sizeof(return_message));
                    snprintf(return_message, 128, "{\"type\":5,\"data\":{\"pathId\":%llu,\"error\":102}}",(UINT64_T)pathId_json->valuedouble);
                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                }else{
                    //回复APP中间状态
                    memset(return_message, 0x00, sizeof(return_message));
                    snprintf(return_message, 128, "{\"type\":5,\"data\":{\"pathId\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble);
                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);

                    //获取当前的点位信息，设备自己参考 TUYA_IPC_MOVE_POINT_T 进行填充
                    strncpy(movePoint.name, pointName_json->valuestring, sizeof(movePoint.name));
                    strncpy(movePoint.coords, "chenlb-test-xyz", sizeof(movePoint.coords));
                    srand(time(NULL));
                    movePoint.mpId = rand();
                    //todo 需要自己生成mpId和coord等信息
                    if (OPRT_OK != tuya_ipc_move_point_add(&movePoint)){
                        PR_ERR("point [%s] add failed", movePoint.name);
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":6,\"data\":{\"pathId\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                        break;
                    }else{

                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":6,\"data\":{\"pathId\":%llu,\"id\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble, movePoint.id);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);

                        char snap_addr[100*1024] = {0};
                        int snap_size = 100*1024;
                        IPC_APP_get_snapshot(snap_addr,&snap_size);
                        if (OPRT_OK != tuya_ipc_move_point_add_pic(movePoint.mpId, snap_addr, snap_size)){
                            PR_ERR("add pic faild");
                            memset(return_message, 0x00, sizeof(return_message));
                            snprintf(return_message, 128, "{\"type\":7,\"data\":{\"pathId\":%llu,\"id\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble, movePoint.id);
                            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                            break;
                        }else{
                            PR_DEBUG("point [%s] add succ  [%llu]", movePoint.name, movePoint.id);
                            memset(return_message, 0x00, sizeof(return_message));
                            snprintf(return_message, 128, "{\"type\":7,\"data\":{\"pathId\":%llu,\"id\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble, movePoint.id);
                            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                            err_num = 0;

                            point_list = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pointList");
                            if (NULL == point_list){
                                point_list = ty_cJSON_CreateArray();  
                                ty_cJSON_AddItemToObject(move_path, "pointList", point_list);  
                            }
                            //节点信息保存到配置文件里面
                            ty_cJSON * point_insert_json = ty_cJSON_CreateObject();
                            ty_cJSON_AddNumberToObject(point_insert_json, "id", movePoint.id);
                            ty_cJSON_AddNumberToObject(point_insert_json, "mpId", movePoint.mpId);
                            ty_cJSON_AddStringToObject(point_insert_json, "coords", movePoint.coords);
                            ty_cJSON_AddItemToArray(point_list, point_insert_json);
                            //更新云端轨迹信息
                            TUYA_IPC_MOVE_PATH_T movePathStr = {0};
                            INT_T point_cnt = ty_cJSON_GetArraySize(point_list);
                            PR_DEBUG("point cnt [%d]",point_cnt);
                            movePathStr.pathId = (UINT64_T)pathId_json->valuedouble;
                            ty_cJSON * pathName_json_tmp = ty_cJSON_GetObjectItem(move_path, "pathName");
                            if (NULL == pathName_json_tmp){
                                PR_DEBUG("get pathName failed");
                            }else{
                                PR_DEBUG("get pathName [%s]",pathName_json_tmp->valuestring);
                                strncpy(movePathStr.pathName, pathName_json_tmp->valuestring, sizeof(movePathStr.pathName));
                            }
                            movePathStr.pointCnt = point_cnt;
                            movePathStr.pointList = malloc(sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*point_cnt);
                            if (NULL == movePathStr.pointList){
                                PR_ERR("malloc failed ");
                                memset(return_message, 0x00, sizeof(return_message));
                                snprintf(return_message, 128, "{\"type\":8,\"data\":{\"pathId\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble);
                                tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                                break;
                            }else{
                                INT_T index = 0;
                                for (index = 0 ; index < point_cnt; index++){
                                    PR_DEBUG("step [%d]",index);
                                    sleep(1);
                                    ty_cJSON * point_temp_json = ty_cJSON_GetArrayItem(point_list, index);
                                    if (NULL == point_temp_json){
                                        PR_DEBUG("get item failed");
                                        sleep(1);
                                    }else{
                                        PR_DEBUG("step [%d]",index);
                                        sleep(1);
                                        ty_cJSON * tmp_point_id_json = ty_cJSON_GetObjectItem(point_temp_json, "id");
                                        ty_cJSON * tmp_point_mpId_json = ty_cJSON_GetObjectItem(point_temp_json, "mpId");
                                        if (NULL == tmp_point_id_json || NULL == tmp_point_mpId_json){
                                            PR_DEBUG("get id or mpId failed");
                                        }else{
                                            PR_DEBUG("step [%d] id[%llu]",index, (UINT64_T)tmp_point_id_json->valuedouble);
                                            sleep(1);
                                            movePathStr.pointList[index].id = (UINT64_T)tmp_point_id_json->valuedouble;
                                            PR_DEBUG("step [%d]",index);
                                            sleep(1);
                                            movePathStr.pointList[index].mpId = tmp_point_mpId_json->valueint;
                                        }
                                    }
                                }
                                PR_DEBUG("step ");
                                sleep(1);
                                if (OPRT_OK != tuya_ipc_move_path_update(&movePathStr)){
                                    PR_ERR("path update failed");
                                    memset(return_message, 0x00, sizeof(return_message));
                                    snprintf(return_message, 128, "{\"type\":8,\"data\":{\"pathId\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble);
                                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                                    break;
                                }else{
                                    memset(return_message, 0x00, sizeof(return_message));
                                    snprintf(return_message, 128, "{\"type\":8,\"data\":{\"pathId\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble);
                                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                                }  
                            }

                        }
                    }
                }
            }else{
                PR_ERR("data error");
                memset(return_message, 0x00, sizeof(return_message));
                snprintf(return_message, 128, "{\"type\":5,\"data\":{\"pathId\":%llu,\"error\":101}}",(UINT64_T)pathId_json->valuedouble);
                tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
            }
            CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
            __tuya_app_write_STR("tuya_move_path", json_buff);
            ty_cJSON_Delete(root_tmp);
            ty_cJSON_FreeBuffer(json_buff);
        }        
        break;
    }    
    case 9:
    {
        //删除轨迹中单个节点
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        ty_cJSON *pointId_json = ty_cJSON_GetObjectItem(child_data, "id");
        PR_DEBUG("pathId [%llu] delete point id [%llu]", (UINT64_T)pathId_json->valuedouble, (UINT64_T)pointId_json->valuedouble);
        //获取pathId下面的所有节点信息
        CHAR_T tmp_value[4096] = {0};
        if (0 != __tuya_app_read_STR("tuya_move_path", tmp_value, sizeof(tmp_value))){
            PR_ERR("no move path info check process");
            memset(return_message, 0x00, sizeof(return_message));
            snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":101}",(UINT64_T)pathId_json->valuedouble);
            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        }else{
            //有数据，匹配到合适的pathId
            ty_cJSON * root_tmp = ty_cJSON_Parse(tmp_value);
            ty_cJSON * move_path_list_json = ty_cJSON_GetObjectItemCaseSensitive(root_tmp, "pathList");
            if (ty_cJSON_IsArray(move_path_list_json)){
                ty_cJSON * move_path = NULL;
                ty_cJSON * pathId_tmp = NULL;
                BOOL_T matchFlag = FALSE;
                ty_cJSON_ArrayForEach(move_path, move_path_list_json){
                    pathId_tmp = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pathId");
                    if (ty_cJSON_IsNumber(pathId_tmp)&& pathId_tmp->valuedouble == pathId_json->valuedouble){
                        PR_DEBUG("match pathId [%llu], travel point info", (UINT64_T)pathId_tmp->valuedouble);
                        ty_cJSON * point_list = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pointList");
                        ty_cJSON * point_json = NULL;
                        ty_cJSON_ArrayForEach(point_json, point_list){
                            ty_cJSON * id_json_tmp = ty_cJSON_GetObjectItem(point_json, "id");
                            ty_cJSON * mpId_json_tmp = ty_cJSON_GetObjectItem(point_json, "mpId");
                            PR_DEBUG("point info id-mpId[%llu---%d]", (UINT64_T)id_json_tmp->valuedouble, mpId_json_tmp->valueint);
                        }
                        matchFlag = TRUE;
                        break;
                    }
                }
                if (FALSE == matchFlag){
                    PR_ERR("pathId match failed");
                    memset(return_message, 0x00, sizeof(return_message));
                    snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":102}}",(UINT64_T)pathId_json->valuedouble);
                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                }else{
                    //匹配点位信息，如果觉得本地配置不可靠，可以去云端同步
                    //遍历匹配 point id信息
                    //遍历节点信息
                    BOOL_T matchPointFlag = FALSE;
                    ty_cJSON * point_list_json = ty_cJSON_GetObjectItem(move_path, "pointList");
                    ty_cJSON * move_point = NULL;
                    INT_T point_index_tmp = 0;
                    if (ty_cJSON_IsArray(point_list_json)){
                        ty_cJSON_ArrayForEach(move_point, point_list_json){
                            ty_cJSON * point_Id_tmp = ty_cJSON_GetObjectItem(move_point, "id");
                            if (ty_cJSON_IsNumber(point_Id_tmp)&& point_Id_tmp->valuedouble == pointId_json->valuedouble){
                                PR_DEBUG("match pointId [%llu], travel point info", (UINT64_T)point_Id_tmp->valuedouble);
                                matchPointFlag = TRUE;
                                break;
                            }
                            point_index_tmp++;
                        }
                        if (FALSE == matchPointFlag){
                            PR_ERR("point Id match failed");
                            memset(return_message, 0x00, sizeof(return_message));
                            snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":104}}",(UINT64_T)pathId_json->valuedouble);
                            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                            break;
                        }else{
                            //需要删除节点信息
                            tuya_ipc_move_point_delete((UINT64_T)ty_cJSON_GetObjectItem(move_point, "id")->valuedouble);
                            ty_cJSON_DeleteItemFromArray(point_list_json, point_index_tmp);    
                        }
                    }else{
                        PR_ERR("get point list error");
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":103}}",pathId_json->valuedouble);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                        break;
                    }
                    //更新云端轨迹信息
                    TUYA_IPC_MOVE_PATH_T movePathStr = {0};
                    point_list_json = ty_cJSON_GetObjectItem(move_path, "pointList");
                    INT_T point_cnt = ty_cJSON_GetArraySize(point_list_json);
                    PR_DEBUG("point cnt [%d]",point_cnt);
                    movePathStr.pathId = pathId_json->valuedouble;
                    strncpy(movePathStr.pathName, ty_cJSON_GetObjectItem(move_path, "pathName")->valuestring, sizeof(movePathStr.pathName));
                    movePathStr.pointCnt = point_cnt;
                    if (0 != point_cnt){
                        movePathStr.pointList = malloc(sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*point_cnt);
                    }
                    INT_T index = 0;
                    for (index = 0 ; index < point_cnt; index++){
                        ty_cJSON * point_temp_json = ty_cJSON_GetArrayItem(point_list_json, index);
                        movePathStr.pointList[index].id = ty_cJSON_GetObjectItem(point_temp_json, "id")->valuedouble;
                        movePathStr.pointList[index].mpId = ty_cJSON_GetObjectItem(point_temp_json, "mpId")->valueint;
                    }
                    if (OPRT_OK != tuya_ipc_move_path_update(&movePathStr)){
                        PR_ERR("path update failed");
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":105}}",(UINT64_T)pathId_json->valuedouble);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                        break;
                    }else{
                        //更新成功之后，在删除本地节点
                        ty_cJSON_DeleteItemFromArray(point_list_json, point_index_tmp); 
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":10,\"data\":{\"pathId\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                    }  
                }
            }else{
                PR_ERR("data error");
                memset(return_message, 0x00, sizeof(return_message));
                snprintf(return_message, 128, "{\"type\":9,\"data\":{\"pathId\":%llu,\"error\":106}}",(UINT64_T)pathId_json->valuedouble);
                tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                break;
            }
            CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
            __tuya_app_write_STR("tuya_move_path", json_buff);
            ty_cJSON_Delete(root_tmp);
            ty_cJSON_FreeBuffer(json_buff);
        }      
        break;
    }
    case 11:
    {
        //修改轨迹名称
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        ty_cJSON *pathName_json = ty_cJSON_GetObjectItem(child_data, "pathName");
        PR_DEBUG("pathId [%llu] rename [%s]", (UINT64_T)pathId_json->valuedouble, pathName_json->valuestring);
        //获取pathId下面的所有节点信息
        CHAR_T tmp_value[4096] = {0};
        if (0 != __tuya_app_read_STR("tuya_move_path", tmp_value, sizeof(tmp_value))){
            PR_ERR("no move path info check process");
            memset(return_message, 0x00, sizeof(return_message));
            snprintf(return_message, 128, "{\"type\":11,\"data\":{\"pathId\":%llu,\"pathName\":\"%s\",\"error\":101}",pathId_json->valuedouble,pathName_json->valuestring);
            tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        }else{
            //有数据，匹配到合适的pathId
            ty_cJSON * root_tmp = ty_cJSON_Parse(tmp_value);
            ty_cJSON * move_path_list_json = ty_cJSON_GetObjectItemCaseSensitive(root_tmp, "pathList");
            if (ty_cJSON_IsArray(move_path_list_json)){
                ty_cJSON * move_path = NULL;
                ty_cJSON * pathId_tmp = NULL;
                BOOL_T matchFlag = FALSE;
                ty_cJSON_ArrayForEach(move_path, move_path_list_json){
                    pathId_tmp = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pathId");
                    if (ty_cJSON_IsNumber(pathId_tmp)&& pathId_tmp->valuedouble == pathId_json->valuedouble){
                        PR_DEBUG("match pathId [%llu], travel point info", pathId_tmp->valuedouble);
                        ty_cJSON * point_list = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pointList");
                        ty_cJSON * point_json = NULL;
                        ty_cJSON_ArrayForEach(point_json, point_list){
                            ty_cJSON * id_json_tmp = ty_cJSON_GetObjectItem(point_json, "id");
                            ty_cJSON * mpId_json_tmp = ty_cJSON_GetObjectItem(point_json, "mpId");
                            PR_DEBUG("point info id-mpId[%llu---%d]", id_json_tmp->valuedouble, mpId_json_tmp->valueint);
                        }
                        matchFlag = TRUE;
                        break;
                    }
                }
                if (FALSE == matchFlag){
                    PR_ERR("pathId match failed");
                    memset(return_message, 0x00, sizeof(return_message));
                    snprintf(return_message, 128, "{\"type\":11,\"data\":{\"pathId\":%llu,\"pathName\":\"%s\",\"error\":102}",(UINT64_T)pathId_json->valuedouble,pathName_json->valuestring);
                    tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                    break;
                }else{
                    //匹配点位信息，如果觉得本地配置不可靠，可以去云端同步
                    //修改名称
                    ty_cJSON * path_name_json_tmp = ty_cJSON_GetObjectItemCaseSensitive(move_path, "pathName");
                    strncpy(path_name_json_tmp->valuestring, pathName_json->valuestring, strlen(path_name_json_tmp->valuestring));
                    //更新云端轨迹信息
                    TUYA_IPC_MOVE_PATH_T movePathStr = {0};
                    ty_cJSON * point_list_json = ty_cJSON_GetObjectItem(move_path, "pointList");
                    INT_T point_cnt = ty_cJSON_GetArraySize(point_list_json);
                    PR_DEBUG("point cnt [%d]",point_cnt);
                    movePathStr.pathId = pathId_json->valuedouble;
                    strncpy(movePathStr.pathName, pathName_json->valuestring, sizeof(movePathStr.pathName));
                    PR_DEBUG("update pathName [%s] ", movePathStr.pathName);
                    movePathStr.pointCnt = point_cnt;
                    if (0 != point_cnt){
                        movePathStr.pointList = malloc(sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*point_cnt);
                    }

                    INT_T index = 0;
                    for (index = 0 ; index < point_cnt; index++){
                        ty_cJSON * point_temp_json = ty_cJSON_GetArrayItem(point_list_json, index);
                        movePathStr.pointList[index].id = ty_cJSON_GetObjectItem(point_temp_json, "id")->valuedouble;
                        movePathStr.pointList[index].mpId = ty_cJSON_GetObjectItem(point_temp_json, "mpId")->valueint;
                    }
                    if (OPRT_OK != tuya_ipc_move_path_update(&movePathStr)){
                        PR_ERR("path update failed");
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":11,\"data\":{\"pathId\":%llu,\"pathName\":\"%s\",\"error\":103}}",(UINT64_T)pathId_json->valuedouble,(CHAR_T *)pathName_json->valuestring);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                        break;
                    }else{
                        memset(return_message, 0x00, sizeof(return_message));
                        snprintf(return_message, 128, "{\"type\":11,\"data\":{\"pathId\":%llu,\"pathName\":\"%s\",\"error\":0}}",(UINT64_T)pathId_json->valuedouble,(CHAR_T *)pathName_json->valuestring);
                        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                    }  
                }
            }else{
                PR_ERR("data error");
                memset(return_message, 0x00, sizeof(return_message));
                snprintf(return_message, 128, "{\"type\":11,\"data\":{\"pathId\":%llu,\"pathName\":\"%s\",\"error\":104}}",(UINT64_T)pathId_json->valuedouble,(CHAR_T *)pathName_json->valuestring);
                tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
                break;
            }
            CHAR_T * json_buff = ty_cJSON_PrintUnformatted(root_tmp);
            __tuya_app_write_STR("tuya_move_path", json_buff);
            ty_cJSON_Delete(root_tmp);
            ty_cJSON_FreeBuffer(json_buff);
        }            
        break;   
    }
    case 12:
    {
        //保存路径，主要是通知设备当前路径完成操作，可以在此做一些自定义行为
        ty_cJSON *pathId_json = ty_cJSON_GetObjectItem(child_data, "pathId");
        PR_DEBUG("pathId [%llu] save", (UINT64_T)pathId_json->valuedouble);
        memset(return_message, 0x00, sizeof(return_message));
        snprintf(return_message, 128, "{\"type\":12,\"data\":{\"pathId\":%llu,\"error\":0}}",(UINT64_T)pathId_json->valuedouble);
        tuya_ipc_dp_report(NULL, TUYA_DP_MOVE_PATH,PROP_STR,return_message,1);
        break;
    }
    default:
        PR_ERR("not support [%d]", child->valueint);
        break;
    }   
    
}

#endif

#if defined(WIFI_GW) && (WIFI_GW==1)
#ifdef TUYA_DP_AP_MODE
STATIC CHAR_T response[128] = {0};
#define WIFI_SSID_LEN 32    // tuya sdk definition WIFI SSID MAX LEN
#define WIFI_PASSWD_LEN 64  // tuya sdk definition WIFI PASSWD MAX LEN
CHAR_T * IPC_APP_get_ap_mode(VOID)
{
    CHAR_T ap_ssid[WIFI_SSID_LEN + 1] = {0};
    CHAR_T ap_pw[WIFI_PASSWD_LEN + 1] = {0};
    WF_WK_MD_E cur_mode = WWM_STATION;
    NW_MAC_S mac = {{0}};
    tkl_wifi_get_work_mode(&cur_mode);
    INT_T is_ap = 0;
    if (cur_mode == WWM_SOFTAP) {
        is_ap = 1;
    }
    OPERATE_RET op_ret = tkl_wifi_get_mac(WF_AP, &mac);
    if (OPRT_OK != op_ret) {
        strncpy(ap_ssid, "TUYA_IPC_AP", WIFI_SSID_LEN);
    } else {
        snprintf(ap_ssid, WIFI_SSID_LEN, "TUYA_IPC-%02X%02X", mac.mac[4], mac.mac[5]);
    }
    __tuya_app_read_STR("tuya_ap_passwd", ap_pw, WIFI_PASSWD_LEN);

    snprintf(response, 128, "{\"is_ap\":%d,\"ap_ssid\":\"%s\",\"password\":\"%s\"}",is_ap,ap_ssid,ap_pw);
    return response;
}
#endif
#ifdef TUYA_DP_AP_SWITCH
VOID* __ap_change_thread(void *arg)
{
    WF_WK_MD_E cur_mode = WWM_STATION;
    tkl_wifi_get_work_mode(&cur_mode);
    WF_AP_CFG_IF_S ap_cfg;
    INT_T ap_onoff = 0;
    tal_system_sleep(5000);

    ap_onoff = __tuya_app_read_INT("tuya_ap_on_off");
    memset(&ap_cfg, 0, SIZEOF(WF_AP_CFG_IF_S));
    PR_INFO("%s : ap_onoff:%d\n", __func__, ap_onoff);
    if (ap_onoff) {
        //ap, sta, monitor, do same
        __tuya_app_read_STR("tuya_ap_ssid", (CHAR_T *)ap_cfg.ssid, WIFI_SSID_LEN);
        __tuya_app_read_STR("tuya_ap_passwd", (CHAR_T *)ap_cfg.passwd, WIFI_PASSWD_LEN);

        ap_cfg.s_len = strlen((CHAR_T *)(ap_cfg.ssid));
        ap_cfg.ssid_hidden = 0;
        ap_cfg.p_len = strlen((CHAR_T *)(ap_cfg.passwd));
        if (ap_cfg.p_len == 0) {
            ap_cfg.md = WAAM_OPEN;
        } else {
            ap_cfg.md = WAAM_WPA2_PSK;
        }

        tkl_wifi_start_ap(&ap_cfg);
    } else {
        if (cur_mode == WWM_SOFTAP) {
            tkl_wifi_stop_ap();
        } else {
            //not ap mode,do nothing
        }
    }

    return NULL;
}

VOID change_ap_process()
{
    pthread_t ap_change_thread;
    int ret = pthread_create(&ap_change_thread, NULL, __ap_change_thread, NULL);
    if (ret != 0) {
        PR_ERR("ap_change_thread ,create fail! ret:%d\n", ret);
        return;
    }

    pthread_detach(ap_change_thread);
}

INT_T IPC_APP_set_ap_mode(IN cJSON *p_ap_info)
{
    if (NULL == p_ap_info){
        return 0;
    }

    INT_T ap_onoff = -1;

    PR_INFO("%s %d handle_DP_AP_SWITCH:%s \r\n",__FUNCTION__,__LINE__, (char *)p_ap_info);

    cJSON * pJson = cJSON_Parse((CHAR_T *)p_ap_info);

    if (NULL == pJson){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        return ap_onoff;
    }
    cJSON * tmp = cJSON_GetObjectItem(pJson, "ap_enable");
    if (NULL == tmp){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        cJSON_Delete(pJson);
        return ap_onoff;
    }
    ap_onoff = tmp->valueint;
    __tuya_app_write_INT("tuya_ap_on_off", ap_onoff);

    if (ap_onoff == 1) {
        cJSON *tmpSsid = cJSON_GetObjectItem(pJson, "ap_ssid");
        if (NULL == tmpSsid) {
            PR_ERR("%s %d step error\n", __FUNCTION__, __LINE__);
            cJSON_Delete(pJson);
            return ap_onoff;
        }
        PR_INFO("###[%d] get ssid:%s\n", ap_onoff, tmpSsid->valuestring);

        cJSON *tmpPwd = cJSON_GetObjectItem(pJson, "ap_pwd");
        if (NULL == tmpPwd){
            PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            return ap_onoff;
        }
        PR_INFO("###get pwd:%s\n", tmpPwd->valuestring);

        __tuya_app_write_STR("tuya_ap_ssid", tmpSsid->valuestring);
        __tuya_app_write_STR("tuya_ap_passwd", tmpPwd->valuestring);
    }

    cJSON_Delete(pJson);

    return ap_onoff;
}

CHAR_T * IPC_APP_update_ap_mode(INT_T is_ap)
{
    CHAR_T ap_ssid[WIFI_SSID_LEN + 1] = { 0 };
    CHAR_T ap_pw[WIFI_PASSWD_LEN + 1] = { 0 };
    NW_MAC_S mac = { {0} };
    OPERATE_RET op_ret = tkl_wifi_get_mac(WF_AP, &mac);
    if (OPRT_OK != op_ret) {
        strncpy(ap_ssid, "TUYA_IPC_AP", WIFI_SSID_LEN);
    } else {
        snprintf(ap_ssid, WIFI_SSID_LEN, "TUYA_IPC-%02X%02X", mac.mac[4], mac.mac[5]);
    }
    __tuya_app_read_STR("tuya_ap_passwd", ap_pw, WIFI_PASSWD_LEN);

    snprintf(response, 128, "{\"is_ap\":%d,\"ap_ssid\":\"%s\",\"password\":\"%s\"}",is_ap,ap_ssid,ap_pw);
    return response;
}

INT_T IPC_APP_get_ap_mode_config()
{
    return __tuya_app_read_INT("tuya_ap_on_off");
}

INT_T IPC_APP_switch_ap_mode()
{
    WF_AP_CFG_IF_S ap_cfg = {0};
    //ap, sta, monitor, do same
    __tuya_app_read_STR("tuya_ap_ssid", (CHAR_T*)ap_cfg.ssid, WIFI_SSID_LEN);
    __tuya_app_read_STR("tuya_ap_passwd", (CHAR_T*)ap_cfg.passwd, WIFI_PASSWD_LEN);

    ap_cfg.s_len = strlen((CHAR_T *)(ap_cfg.ssid));
    ap_cfg.ssid_hidden = 0;
    ap_cfg.p_len = strlen((CHAR_T *)(ap_cfg.passwd));
    if (ap_cfg.p_len == 0) {
        ap_cfg.md = WAAM_OPEN;
    } else {
        ap_cfg.md = WAAM_WPA2_PSK;
    }

    return tkl_wifi_start_ap(&ap_cfg);

    return 0;
}
#endif

#ifdef TUYA_DP_AP_TIME_SYNC
STATIC BOOL_T time_sync = FALSE;
VOID IPC_APP_set_time(IN CHAR_T *p_time_info)
{
    if (NULL == p_time_info) {
        return;
    }
    if (time_sync == TRUE) {
        return;
    }
    PR_INFO("%s %d recv time sync from app:%s \r\n", __FUNCTION__, __LINE__, p_time_info);

    tuya_ipc_set_service_time(atoi(p_time_info));
    time_sync = TRUE;
    return;
}
#endif

#ifdef TUYA_DP_AP_TIME_ZONE
STATIC BOOL_T timezone_sync = FALSE;
VOID IPC_APP_set_time_zone(IN CHAR_T *p_time_info)
{
    if (NULL == p_time_info) {
        return;
    }
    PR_INFO("%s %d recv timeZONE sync from app:%s \r\n", __FUNCTION__, __LINE__, p_time_info);
    if (timezone_sync == TRUE) {
        return;
    }

    extern OPERATE_RET tal_time_set_time_zone(IN CONST CHAR_T *time_zone);
    tal_time_set_time_zone(p_time_info);
    
    //TODO: You need call your own timezone setter function here.

    timezone_sync = TRUE;
    return;
}
#endif
#endif

#ifdef TUYA_DP_DEVICE_VOLUME
VOID IPC_APP_set_device_volume(int volume_value)
{
    if (volume_value < 0 || volume_value > 100) {
        return;
    }

    // 鸟类警笛音量
    tkl_ao_set_vol(0, 0, NULL, volume_value);

    __tuya_app_write_INT("tuya_player_volume", volume_value);
}

INT_T IPC_APP_get_device_volume()
{
    int volume_value = 0;
    volume_value = __tuya_app_read_INT("tuya_player_volume");
    PR_INFO("curr volume_value:%d \r\n", volume_value);
    return volume_value;
}
#endif

#ifdef TUYA_DP_IPC_CONTRAST
VOID IPC_APP_set_ipc_contrast(int value)
{
    PR_INFO("set ipc_contrast:%d \r\n", value);
    if (value < 0 || value > 100) {
        return;
    }
    tkl_vi_set_contrast(value);
}

INT_T IPC_APP_get_ipc_contrast()
{
    int value = tkl_vi_get_contrast();
    PR_INFO("get ipc_contrast:%d \r\n", value);
    return value;
}
#endif

#ifdef TUYA_DP_IPC_BRIGHT
VOID IPC_APP_set_ipc_bright(int value)
{
    PR_INFO("set ipc_bright:%d \r\n", value);
    if (value < 0 || value > 100) {
        return;
    }
    tkl_vi_set_brightness(value);
}

INT_T IPC_APP_get_ipc_bright()
{
    int value = tkl_vi_get_brightness();
    PR_INFO("get ipc_bright:%d \r\n", value);
    return value;
}
#endif

#ifdef TUYA_DP_SHOW_MAC
INT_T IPC_APP_get_mac(NW_MAC_S * mac)
{
    OPERATE_RET op_ret = tkl_wifi_get_mac(WF_STATION, mac);
    if (OPRT_OK != op_ret) {
        PR_ERR("wifi get mac failed\n");
        return -1;
    } else {
        return 0;
    }
}
#endif

#ifdef TUYA_DP_FLICKER
VOID IPC_APP_set_flicker(UINT_T value)
{
    PR_INFO("set flicker:%d \r\n", value);
    if (value < 0 || value > 2) {
        return;
    }

    OPERATE_RET op_ret = tkl_vi_set_flicker(value);
    if (op_ret == 0) {
        __tuya_app_write_INT("tuya_flicker", value);
    }
}

UINT_T IPC_APP_get_flicker()
{
    UINT_T flicker_value = __tuya_app_read_INT("tuya_flicker");
    PR_INFO("curr flicker_value:%d \r\n", flicker_value);
    return flicker_value;
}
#endif
