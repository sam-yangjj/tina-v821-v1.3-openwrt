/**
 * @file tuya_ipc_sdk_info.h
 * @brief This is tuya ipc sdk info file
 * @version 1.0
 * @date 2021-11-17
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef _TUYA_IPC_SDK_INFO_H
#define _TUYA_IPC_SDK_INFO_H

#include "ty_cJSON.h"
#include "tuya_cloud_wifi_defs.h"

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum  {
    E_IPC_UNREGISTERED = 0,
    E_IPC_REGISTERED = 1,
    E_IPC_ACTIVEATED = 2    ///< after successful register and active process, device is in this status
} IPC_REGISTER_STATUS_E;

typedef enum {
    WEATHER_HEAVY_RAIN = 101,  //大雨
    WEATHER_THUNDERSTORM,  //雷暴
    WEATHER_SANDSTORM,  //沙尘暴
    WEATHER_LIGHT_SNOW,  //小雪
    WEATHER_SNOW,  //雪
    WEATHER_FROZEN_FOG,  //冻雾
    WEATHER_RAINSTORM,  //暴雨
    WEATHER_SCATTERED_SHOWERS,  //局部阵雨
    WEATHER_DUST,  //浮尘
    WEATHER_LIGHTNING,  //雷电
    WEATHER_LIGHT_SHOWERS,  //小阵雨
    WEATHER_RAIN,  //雨
    WEATHER_SLEET,  //雨夹雪
    WEATHER_DUST_STORM,  //尘卷风
    WEATHER_ICY,  //冰粒
    WEATHER_HEAVY_SANDSTORM,  //强沙尘暴
    WEATHER_SAND,  //扬沙
    WEATHER_SMALL_TO_MEDIUM_RAIN,  //小到中雨
    WEATHER_MOSTLY_SUNNY,  //大部晴朗
    WEATHER_SUNNY,  //晴
    WEATHER_FOG,  //雾
    WEATHER_SHOWERS,  //阵雨
    WEATHER_HEAVY_SHOWERS,  //强阵雨
    WEATHER_HEAVY_SNOW,  //大雪
    WEATHER_EXTREME_RAINSTORM,  //特大暴雨
    WEATHER_BLIZZARD,  //暴雪
    WEATHER_HAIL,  //冰雹
    WEATHER_SMALL_TO_MEDIUM_SNOW,  //小到中雪
    WEATHER_PARTLY_CLOUDY,  //少云
    WEATHER_LIGHT_SNOW_SHOWERS,  //小阵雪
    WEATHER_MEDIUM_SNOW,  //中雪
    WEATHER_OVERCAST,  //阴
    WEATHER_NEEDLE_ICE,  //冰针
    WEATHER_HEAVY_RAINSTORM,  //大暴雨
    WEATHER_THUNDERSHOWER_WITH_HAIL = 136,  //雷阵雨伴有冰雹
    WEATHER_FREEZING_RAIN,  //冻雨
    WEATHER_SNOW_SHOWERS,  //阵雪
    WEATHER_LIGHT_RAIN,  //小雨
    WEATHER_HAZE,  //霾
    WEATHER_MEDIUM_RAIM,  //中雨
    WEATHER_CLOUDY,  //多云
    WEATHER_THUNDERSHOWER,  //雷阵雨
    WEATHER_MEDIUM_TO_BIG_RAIN,  //中到大雨
    WEATHER_BIG_TO_HEAVY_RAIN,  //大到暴雨
    WEATHER_CLEAR,  //晴朗
    WEATHER_CONDITION_MAX
} WEATHER_CONDITION_E;

typedef struct {
    BOOL_T is_valid;
    INT32_T value;
}WEATHER_VALUE_INT_T;

typedef struct {
    BOOL_T is_valid;
    CHAR_T value[32];
}WEATHER_VALUE_STR_T;

typedef struct {
    BOOL_T is_valid;
    FLOAT_T value;
}WEATHER_VALUE_FLOAT_T;

typedef struct {
    WEATHER_CONDITION_E condition_num; ///> weather condition num
    WEATHER_VALUE_INT_T temp;          ///> temp  unit:℃
    WEATHER_VALUE_INT_T real_feel;     ///> body temperatur unit:℃
    WEATHER_VALUE_INT_T humidity;      ///> humidity
    WEATHER_VALUE_INT_T pressure;      ///> pressure unit:millibar
    WEATHER_VALUE_INT_T uvi;           ///> ultraviolet index
    WEATHER_VALUE_STR_T wind_dir;      ///> wind direction. N, NE, E, SE, S, SW, W, NW
    WEATHER_VALUE_FLOAT_T wind_speed;  ///> wind speed unit:m/s
    WEATHER_VALUE_INT_T wind_level;    ///> wind level
    WEATHER_VALUE_INT_T aqi;           ///> air quality index
    WEATHER_VALUE_INT_T temp_high;     ///> temp high unit:℃
    WEATHER_VALUE_INT_T temp_low;      ///> temp low  unit:℃
    WEATHER_VALUE_INT_T pm25;          ///> PM2.5 uint:ug/m3
    WEATHER_VALUE_INT_T pm10;          ///> PM10 uint:ug/m3
    WEATHER_VALUE_INT_T so2;           ///> SO2 uint:ug/m3
    WEATHER_VALUE_INT_T o3;            ///> O3 uint:ug/m3
    WEATHER_VALUE_INT_T no2;           ///> NO2 uint:ug/m3
    WEATHER_VALUE_INT_T co;            ///> CO uint:ug/m3
    WEATHER_VALUE_STR_T sunrise;       ///> sunrise time  "2022-10-20 05:24"
    WEATHER_VALUE_STR_T sunset;        ///> sunset time   "2022-10-20 18:32"

    WEATHER_VALUE_INT_T expiration;    ///> unit:min
    WEATHER_VALUE_STR_T city;          ///> city name
    WEATHER_VALUE_STR_T area;          ///> city area name
} IPC_WEATHER_T;

/**
 * @brief return sdk info, sdk version etc.
 *
 * @param VOID 
 * 
 * @return CHAR_T* 
 */
CHAR_T* tuya_ipc_get_sdk_info(VOID);

/**
 * @brief get register status on tuya cloud
 * 
 * @param VOID 
 * 
 * @return IPC_REGISTER_STATUS_E 
 */
IPC_REGISTER_STATUS_E tuya_ipc_get_register_status(VOID);

/**
 * @brief get time which sync with tuya server and APP
 * 
 * @param[out] time_utc: utc time
 * @param[out] time_zone: time zone in seconds
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_service_time(OUT TIME_T *time_utc, OUT INT_T *time_zone);

/**
 * @brief check if specified utc time is in daylight saving time
 * 
 * @param[in] time_utc: utc time
 * @param[out] p_is_dls: TRUE/FALSE set into *p_is_dls
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_check_in_dls(IN TIME_T time_utc, OUT BOOL_T *p_is_dls);

/**
 * @brief transfer utc time to local time with timezone
 * 
 * @param[in] in_time: input utc time
 * @param[out] local_time: output local time 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_local_time(IN TIME_T in_time, OUT struct tm *local_time);

/**
 * @brief get utc time in tm format
 * 
 * @param[out] utc_time
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_utc_tm(OUT struct tm *utc_time);

/**
 * @brief get utc time
 * 
 * @param[out] time_utc: utc time
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_utc_time(OUT TIME_T *time_utc);

/**
 * @brief get free memory of system in KB, only standard linux supported
 * 
 * @param[out] free: KB
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_free_ram(OUT ULONG_T *free);

/**
 * @brief Get signatrue key from ipc sdk.
 * 
 * @param[out] dest_key_buf: len >= 17
 * @param[out] len: key length
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_local_key(OUT CHAR_T *dest_key_buf, OUT UINT_T *len);

/**
 * @brief get device id
 * 
 * @param[out] devid: device id
 * @param[out] id_len: device id length
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_device_id(OUT CHAR_T *devid, OUT INT_T *id_len);

/**
 * @brief get low power server info
 * 
 * @param[out] ip: server ip
 * @param[out] port: server port
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_low_power_server(OUT TUYA_IP_ADDR_T *ip, OUT UINT_T *port);

/**
 * @brief get low power server info v2
 * 
 * @param[out] domain: server domain
 * @param[in] domain_len: domain length
 * @param[out] ip: server ip
 * @param[out] port: server port
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_low_power_server_v2(CHAR_T * domain, INT_T domain_len, TUYA_IP_ADDR_T *ip,UINT_T *port);

/**
 * @brief get mqtt connection status, mqtt is used by tuya SDK to commnicate with tuya cloud
 * 
 * @param VOID 
 * 
 * @return 0:offline 1:online 
 */
INT_T tuya_ipc_get_mqtt_status(VOID);

/**
 * @brief set mqtt connection status,  called by status cb. useless for customers.
 * 
 * @param stat: mqtt status, notified by mqtt event. 0:offline, 1:online
 * 
 * @return 0:success 
 */
OPERATE_RET tuya_ipc_set_mqtt_status(INT_T stat);
/**
 * @brief set time of tuya SDK
 * 
 * @param[in] new_time_utc: new utc time
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_set_service_time(IN TIME_T new_time_utc);

/**
 * @brief get SDK time, if failed will force update from cloud
 * 
 * @param[out] time_utc: utc time
 * @param[out] time_zone: time zone in seconds
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_get_service_time_force(OUT TIME_T *time_utc, OUT INT_T *time_zone);


typedef VOID (*ALGO_MODEL_NOTIFY_CB)(IN CONST INT_T download_result, IN PVOID_T pri_data);
typedef OPERATE_RET (*GET_ALGO_MODEL_DATA_CB)(IN CONST UINT_T total_len,IN CONST UINT_T offset,
                                             IN CONST BYTE_T *data,IN CONST UINT_T len,
                                             OUT UINT_T *remain_len,IN PVOID_T priv_data);
/**
 * @brief get algorithm model information
 *
 * @param[IN] model_id: algorithm model id; allow to NULL;
 * @param[IN] model_ver: algorithm model version; allow to NULL;
 * @param[OUT] result: algorithm model result from cloud;
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_get_algorithm_model_info(IN CHAR_T * model_id,IN CHAR_T *model_ver,OUT ty_cJSON ** result);
/**
 * @brief get algorithm model file
 *
 * @param[IN] url: file url;
 * @param[IN] total_file_size: file size;
 * @param[IN] file_hmac: file hmac;
 * @param[IN] algo_model_data_cb: file data callback;
 * @param[IN] algo_model_notify_cb: file dowlowd end callback;
 * @param[IN] pri_data: user custome data;
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_download_algorithm_model_start(IN CHAR_T * url, IN UINT_T total_file_size,IN CHAR_T file_hmac[64],
                                                    IN GET_ALGO_MODEL_DATA_CB algo_model_data_cb,
                                                    IN ALGO_MODEL_NOTIFY_CB algo_model_notify_cb,
                                                    IN PVOID_T pri_data);

/**
 * @brief get dev name (rename from APP)
 *
 * @param[OUT] dev_name: dev name;
 * @param[IN] dev_name_len: dev_name buff size;
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_get_dev_name_info(OUT CHAR_T * dev_name, IN INT_T dev_name_len);

/**
 * @brief get dns mode for ipv4 or ipv6
 *
 * @param[OUT] dns_mode: dns mode define by iot
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_get_dns_mode(OUT DNS_MODE_E * p_dns_mode);


/**
 * @brief get realtime weather
 *
 * @param[OUT] weather: weather info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_get_realtime_weather(OUT IPC_WEATHER_T *weather);

/**
 * @brief get future weather
 *
 * @param[OUT] weather: weather info.array's dimension is the same as the number of forecast days.
 * @param[INT] predict_days: forecast days
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_get_future_weather(OUT IPC_WEATHER_T *weather, IN INT_T predict_days);

/**
 * @brief get language package from tuya cloud
 * @param[in] pid product id
 * @param[in] lang language type
 * @param[out] result json encoded data
 * @return error code
 */
OPERATE_RET tuya_ipc_language_get_packet_by_pid(IN CONST CHAR_T *pid, IN CONST CHAR_T *lang, OUT ty_cJSON **result);

/**
 * @brief free json encoded data
 * @note this function should be called after tuya_ipc_language_get_packet_by_pid
 * @param[in] result json encoded data
 * @return error code
 */
OPERATE_RET tuya_ipc_language_packet_free(IN ty_cJSON *result);

/**
 * @brief get p2p ready status
 *
 * @return TRUE if p2p is ready
 */
BOOL_T tuya_ipc_get_p2p_ready(VOID);

#ifdef __cplusplus
}
#endif
    
#endif
    

