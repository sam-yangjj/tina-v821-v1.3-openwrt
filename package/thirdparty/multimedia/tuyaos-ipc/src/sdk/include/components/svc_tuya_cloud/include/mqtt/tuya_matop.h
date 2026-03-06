#ifndef _TUYA_MATOP_H_
#define _TUYA_MATOP_H_

#include "tuya_cloud_types.h"
#include "tuya_svc_mqtt_client.h"
#include "ty_cJSON.h"

#if defined(ENABLE_MATOP) && (ENABLE_MATOP==1)

typedef struct {
    BOOL_T success;         // response status
    ty_cJSON *result;       // json formatted result if json parse success, need release manually
    UINT_T t;               // uni-timestamp
    VOID *user_data;        // custom user data
    UINT8_T *raw_data;      // raw data reulst, if json parse failed
    SIZE_T raw_data_len;    // raw data length
} mqtt_atop_response_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  matop async callback, need release response->result manually
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
typedef OPERATE_RET(*MQTT_ATOP_RESPONSE_CB)(mqtt_atop_response_t *response, VOID *user_data);

/**
 * @brief init matop
 *
 */
VOID tuya_matop_init();

/**
 * @brief is tuya matop ready
 *
 * @return TRUE ready, FALSE not ready
 */
BOOL_T tuya_matop_is_ready();

/**
 * @brief tuya_matop_request_async
 *
 * @param api_name API name
 * @param api_ver API version
 * @param data Post data
 * @param data_other Other parameters
 * @param timeout timeout in millisecond
 * @param notify_cb callback
 * @param user_data private user data for callback
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_matop_request_async(CONST CHAR_T *api_name,
                                     CONST CHAR_T *api_ver,
                                     CONST CHAR_T *data,
                                     CONST CHAR_T *data_other,
                                     UINT_T timeout,
                                     MQTT_ATOP_RESPONSE_CB notify_cb,
                                     VOID *user_data);

/**
 * @brief tuya_matop_request_sync
 *
 * @param api_name API name
 * @param api_ver API version
 * @param data Post data
 * @param data_other Other parameters
 * @param timeout timeout in millisecond
 * @param notify_cb callback
 * @param user_data private user data for callback
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_matop_request_sync(CONST CHAR_T *api_name,
                                    CONST CHAR_T *api_ver,
                                    CONST CHAR_T *data,
                                    CONST CHAR_T *data_other,
                                    UINT_T timeout,
                                    MQTT_ATOP_RESPONSE_CB notify_cb,
                                    VOID *user_data);

/**
 * @brief tuya_matop_request_sync_simple
 *
 * @param api_name API name
 * @param api_ver API version
 * @param data Post data
 * @param data_other Other parameters
 * @param timeout timeout in millisecond
 * @param pp_result Post result point, need call ty_cJSON_Delete manually
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_matop_request_sync_simple(CONST CHAR_T *api_name,
                                           CONST CHAR_T *api_ver,
                                           CONST CHAR_T *data,
                                           CONST CHAR_T *data_other,
                                           UINT_T timeout,
                                           ty_cJSON **pp_result);

#if 0
/**
 * @brief tuya_matop_request_sync legacy wapper, instead of iot_httpc_common_post
 *
 * @param api_name API name
 * @param api_ver API version
 * @param uuid not used
 * @param devid not used
 * @param post_data Post data
 * @param p_head_other
 * @param pp_result Post result point, need call ty_cJSON_Delete manually
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
#define matop_service_request_legacy(api_name, api_ver, uuid, devid, post_data, p_head_other, pp_result) \
    ((void)uuid, (void)devid, \
    tuya_matop_request_sync_simple(api_name, api_ver, post_data, p_head_other, 0, pp_result))

/**
 * @brief tuya_matop_request_sync simple wapper, instead of iot_httpc_common_post_simple
 *
 * @param api_name API name
 * @param api_ver API version
 * @param post_data Post data
 * @param p_head_other
 * @param pp_result Post result point, need call ty_cJSON_Delete manually
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
#define matop_service_request_legacy_simple(api_name, api_ver, post_data, p_head_other, pp_result) \
    tuya_matop_request_sync_simple(api_name, api_ver, post_data, p_head_other, 0, pp_result)


/**
 * @brief matop file download in range
 *
 * @param url url
 * @param range_start range start
 * @param range_end range end
 * @param timeout timeout in millisecond
 * @param data_other Other parameters
 * @param notify_cb callback
 * @param user_data callback private user data
 * @return download length on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET matop_service_file_download_range(CONST CHAR_T* url,
                                              UINT_T range_start,
                                              UINT_T range_end,
                                              UINT_T timeout,
                                              CONST CHAR_T *data_other,
                                              MQTT_ATOP_RESPONSE_CB notify_cb,
                                              VOID* user_data);

/**
 * @brief matop file download in simple mode
 *
 * @param url url
 * @param file_offset file offset
 * @param dl_size download size this time
 * @param timeout timeout in millisecond
 * @param data_other Other parameters
 * @param buf buffer for download
 * @param buf_len buffer length
 * @return download length on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET matop_service_file_download_simple(CONST CHAR_T* url,
                                               UINT_T file_offset,
                                               UINT_T dl_size,
                                               UINT_T timeout,
                                               CONST CHAR_T *data_other,
                                               BYTE_T *buf,
                                               UINT_T buf_len);
#endif
#ifdef __cplusplus
}
#endif
#endif
#endif
