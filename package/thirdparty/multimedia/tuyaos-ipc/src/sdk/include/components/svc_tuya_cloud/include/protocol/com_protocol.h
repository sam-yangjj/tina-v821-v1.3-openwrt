/**
* @file com_protocol.h
* @brief Common process - common protocol
* @version 0.1
* @date 2017-04-12
*
* @copyright Copyright 2015-2021 Tuya Inc. All Rights Reserved.
*
*/

#ifndef _COM_PROTOCOL_H
#define _COM_PROTOCOL_H
#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FR_TYPE_AP_ENCRYPTION                   (0x11)
#define FR_TYPE_ENCRYPTION                      (0x13)
#define FRM_AP_CFG_WF_V40                       (0x14)
#define WIFI_ERR_APP_TO_DEV                     (0x15)
#define FRM_AP_CFG_GET_DEV_INFO                 (0x16)
#define FRM_AP_CFG_SET_DEV_SCHEMA               (0x17)
#define FRM_AP_CFG_SET_TIME                     (0x18)
#define FRM_AP_CFG_SET_ACTIVE_INFO              (0x19)
#define FRM_LAN_OTA_START                       (0x1A)
#define FRM_LAN_OTA_DATA                        (0x1B)
#define FRM_LAN_OTA_FINISH                      (0x1C)
#define FRM_LAN_RESET                           (0x1D)
#define FRM_AP_CFG_EXT_CMD                      (0x1E)
#define FRM_AP_CFG_4G                           (0x1F)

#pragma pack(1)
/**
 * @brief LAN protocol frame head format (from app)
 */
typedef struct {
    UINT_T head; // 0x55aa
    uint8_t version;/*version:hight 4bit, reserved: low 4bit*/
    uint8_t reserved;
    UINT_T fr_num;  // frame sequence number
    UINT_T fr_type; // frame type
    UINT_T len;     // data length
    BYTE_T data[0]; // the data
} LAN_PRO_HEAD_APP_S;

/**
 * @brief LAN protocol frame tail format (from device)
 */
typedef struct {
    UINT_T crc;  // CRC of data
    UINT_T tail; // 0xaa55
} LAN_PRO_TAIL_S;

/**
 * @brief LAN protocol frame head format (from device)
 */
typedef struct {
    UINT_T head;    // 0x55aa
    UINT_T fr_num;  // frame sequence number
    UINT_T fr_type; // frame type
    UINT_T len;     // data length
    UINT_T ret_code;// return code
    BYTE_T data[0]; // the data
} LAN_PRO_HEAD_GW_S;
#pragma pack()

#define LPV35_FRAME_HEAD_SIZE        4
#define LPV35_FRAME_VERSION_SIZE     1
#define LPV35_FRAME_RESERVE_SIZE     1
#define LPV35_FRAME_SEQUENCE_SIZE    4
#define LPV35_FRAME_TYPE_SIZE        4
#define LPV35_FRAME_DATALEN_SIZE     4
#define LPV35_FRAME_NONCE_SIZE       12
#define LPV35_FRAME_TAG_SIZE         16
#define LPV35_FRAME_TAIL_SIZE        4

#define LPV35_FRAME_MINI_SIZE (LPV35_FRAME_HEAD_SIZE + LPV35_FRAME_VERSION_SIZE + LPV35_FRAME_RESERVE_SIZE + \
    LPV35_FRAME_SEQUENCE_SIZE + LPV35_FRAME_TYPE_SIZE + LPV35_FRAME_DATALEN_SIZE + LPV35_FRAME_NONCE_SIZE + \
    LPV35_FRAME_TAG_SIZE + LPV35_FRAME_TAIL_SIZE)
#pragma pack(1)
typedef struct {
    uint8_t version: 4;
    uint8_t reserved: 4;
    uint8_t reserved2;
    uint32_t sequence;
    uint32_t type;
    uint32_t length;
} lpv35_fixed_head_t;

typedef lpv35_fixed_head_t lpv35_additional_data_t;

typedef struct {
    uint32_t ret_code;
    uint8_t data[0];
} lpv35_plaintext_data_t;
#pragma pack()

typedef struct {
    uint32_t sequence;
    uint32_t type;
    uint8_t* data;
    uint32_t data_len;
} lpv35_frame_object_t;

#define APP_KEY_LEN 16

/**
 * @brief parse data
 *
 * @param[in] cmd refer to DP_CMD_TYPE_E
 * @param[in] data origin data
 * @param[in] len data lemgth
 * @param[out] out_data parse data out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET parse_data_with_cmd(IN CONST DP_CMD_TYPE_E cmd, IN BYTE_T *data, IN CONST INT_T len, OUT CHAR_T **out_data);

/**
 * @brief pack data
 *
 * @param[in] cmd refer to DP_CMD_TYPE_E
 * @param[in] src origin data
 * @param[in] pro pro
 * @param[in] num num
 * @param[out] out pack out data
 * @param[out] out_len pack data length
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET pack_data_with_cmd(IN CONST DP_CMD_TYPE_E cmd, IN CONST CHAR_T *src, \
                               IN CONST UINT_T pro, IN CONST UINT_T num, \
                               OUT BYTE_T **out, OUT UINT_T *out_len);

/***********************************************************
*  Function: parse_data_with_cmd
*  Input: cmd data len
*  Output: out_data
*  Return: OPERATE_RET
***********************************************************/

/**
 * @brief parse protocol data
 *
 * @param[in] cmd refer to DP_CMD_TYPE_E
 * @param[in] pv verison
 * @param[in] data origin data
 * @param[in] len data length
 * @param[in] key parse key
 * @param[out] out_data parse out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_parse_protocol_data(IN CONST DP_CMD_TYPE_E cmd, IN CONST CHAR_T *pv, IN BYTE_T *data, \
                                     IN CONST INT_T len, IN CONST CHAR_T *key, OUT CHAR_T **out_data);

/**
 * @brief pack protocol data
 *
 * @param[in] cmd refer to DP_CMD_TYPE_E
 * @param[in] pv verison
 * @param[in] src origin data
 * @param[in] pro pro
 * @param[in] num num
 * @param[in] key pack key
 * @param[out] out pack out
 * @param[out] out_len pack out length
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_pack_protocol_data(IN CONST DP_CMD_TYPE_E cmd, IN CONST CHAR_T *pv, IN CONST CHAR_T *src, \
                                    IN CONST UINT_T pro, IN CONST UINT_T num, IN UINT_T from, IN BYTE_T *key, \
                                    OUT CHAR_T **out, OUT UINT_T *out_len);

/**
 * @brief Decrypt LAN data
 *
 * @param[in] data encry data
 * @param[in] len encry data length
 * @param[in] key decrypt key
 * @param[out] out_data origin data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_svc_lan_decrypt_data(IN BYTE_T *data, IN CONST INT_T len,
                                      IN CONST BYTE_T *key, OUT CHAR_T **out_data);

/**
 * @brief add head and tail in lpv35 frame
 *
 * @param[in] key encrypt key
 * @param[in] key_len encrypt key len
 * @param[in] input raw data of lpv35 frame
 * @param[out] output out frame data
 * @param[out] olen out frame data len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET lpv35_frame_serialize(CONST BYTE_T* key, INT_T key_len,
                                      CONST lpv35_frame_object_t* input,
                                      BYTE_T* output, INT_T* olen);

/**
 * @brief lpv35 frame parse
 *
 * @param[in] key decrypt key
 * @param[in] key_len decrypt key len
 * @param[in] input lpv35 frame
 * @param[in] ilen lpv35 frame len
 * @param[out] output decrypt raw lpv35 data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET lpv35_frame_parse(CONST BYTE_T* key, INT_T key_len,
                              CONST BYTE_T* input, INT_T ilen,
                              lpv35_frame_object_t* output);

/**
 * @brief get lpv35 frame buffer size
 *
 * @param[in] frame_obj lpv35 frame object
 *
 * @return lpv35 frame buffer size
 */
INT_T lpv35_frame_buffer_size_get(lpv35_frame_object_t* frame_obj);

#ifdef __cplusplus
}
#endif
#endif

