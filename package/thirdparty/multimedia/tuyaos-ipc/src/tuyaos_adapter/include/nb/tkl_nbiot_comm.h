/**
 * @file tkl_nbiot_comm.h
 * @brief Common process - common defination
 * 
 * @copyright Copyright(C),2018-2020, 涂鸦科技 www.tuya.com
 * 
 */

#ifndef __TKL_NBIOT_COMM_H__
#define __TKL_NBIOT_COMM_H__


#include <stdbool.h>
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TKL_NBIOT_ISP_CT,
    TKL_NBIOT_ISP_CMCC,
    TKL_NBIOT_ISP_UNICOM,
    TKL_NBIOT_ISP_TUYA,
    TKL_NBIOT_ISP_OTHER
} NBIOT_ISP_T;

typedef VOID_T (*NBIOT_REV_DATA_CB)(UCHAR_T *data, UINT_T data_len);
typedef INT_T (*NBIOT_EVENT_CB)(UINT_T event_id, PVOID_T param);

typedef struct {
    NBIOT_EVENT_CB 	event_cb;               // lwm2m event send callback
    UCHAR_T         evt_id_ready;           // lwm2m ready event 
    UCHAR_T 		evt_id_connected;       // lwm2m connected event 
    UCHAR_T 		evt_id_update_success;  // lwm2m update successful event 
    UCHAR_T 		evt_id_rsp_success;     // lwm2m response successful event 
    
    BOOL_T  		bootstrap_en;           // boot strap enable
    NBIOT_ISP_T 	isp_type;               // connect isp type,refer to NBIOT_ISP_T
    UINT_T   		lifetime;               // life time with the server
    UINT_T			srv_port;               // desired port of lwm2m server
    CHAR_T *        srv_ip;                 // desired ip address of lwm2m server
    CHAR_T *		imei;                   // imei of the client
    CHAR_T *        psk;                    // psk used for dtls
    
    NBIOT_REV_DATA_CB data_recv_cb;         // data reveive callback
}NBIOT_LWM2M_REGISTER_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



