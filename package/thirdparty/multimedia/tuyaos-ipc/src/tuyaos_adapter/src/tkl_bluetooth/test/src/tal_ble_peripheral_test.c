#ifdef __linux__
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
 #include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#endif

#include "tal_bluetooth.h"
#include "tuya_cloud_types.h"

#define TUYA_BLE_MAX_MTU_SIZE       (247)

STATIC UCHAR_T adv_data_const[31] =
{
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x50, 0xFD,
    0x17,
    0x16,
    0x50, 0xFD,
    0x41, 0x00,       //Frame Control
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

STATIC UCHAR_T scan_rsp_data_const[31] =
{   
    0x17,             // length
    0xFF,
    0xD0,
    0x07,
    0x00, //Encry Mode(8)
    0x00,0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //FLAG
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x03, //24
    0x09,
    0x54, 0x59,
};

STATIC TAL_BLE_PEER_INFO_T ble_peripheral_info;

STATIC UCHAR_T ble_peripheral_connected = 0;

STATIC VOID tuyaos_bluetooth_event_callback(TAL_BLE_EVT_PARAMS_T *p_event)
{
	printf("aaaaa\n");
    switch(p_event->type) {
        // Service Register and Stack init Event
        case TAL_BLE_STACK_INIT:
            if(p_event->ble_event.init == 0) {
                TAL_BLE_DATA_T adv_data;
                TAL_BLE_DATA_T rsp_data;

                adv_data.p_data = adv_data_const;
                adv_data.len = sizeof(adv_data_const);

                rsp_data.p_data = scan_rsp_data_const;
                rsp_data.len = sizeof(scan_rsp_data_const);
                
                tal_ble_advertising_data_set(&adv_data, &rsp_data);
                // After Init Successfully, we will do adv
                tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);
            }
            break;
        
        // Peripheral Connect Event
        case TAL_BLE_EVT_PERIPHERAL_CONNECT:
            if(p_event->ble_event.connect.result == 0) {
                TAL_BLE_DATA_T read_data;
                UCHAR_T read_buffer[512];

                memcpy(&ble_peripheral_info, &p_event->ble_event.connect.peer, sizeof(TAL_BLE_PEER_INFO_T));
                memcpy(read_buffer, adv_data_const, sizeof(adv_data_const));
                memcpy(&read_buffer[sizeof(adv_data_const)], scan_rsp_data_const, sizeof(scan_rsp_data_const));
                
                read_data.p_data = read_buffer;
                read_data.len = sizeof(adv_data_const) + sizeof(scan_rsp_data_const);

                // Verify Read Char
                tal_ble_server_common_read_update(&read_data);

                // Try to request the mtu exchange
                tal_ble_client_exchange_mtu_request(ble_peripheral_info, 247);

                // Try to request connection parameter update
                tal_ble_conn_param_update(ble_peripheral_info, TUYAOS_BLE_DEFAULT_CONN_PARAM);

                ble_peripheral_connected  = 1;
            }else {
                memset(&ble_peripheral_info, 0, sizeof(TAL_BLE_PEER_INFO_T));
            }
            break;
        
        // Disconnect Event
        case TAL_BLE_EVT_DISCONNECT: {
                ble_peripheral_connected = 0;
                tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);
            } break;

        // Data From the Client
        case TAL_BLE_EVT_WRITE_REQ: {
                TAL_BLE_DATA_T send_data;
                UCHAR_T send_buffer[247];

				if(p_event->ble_event.write_report.peer.char_handle[0] == ble_peripheral_info.char_handle[TAL_COMMON_WRITE_CHAR_INDEX]) {
					send_data.p_data = send_buffer;
					memcpy(send_data.p_data, p_event->ble_event.write_report.report.p_data, p_event->ble_event.write_report.report.len);
					send_data.len = p_event->ble_event.write_report.report.len;
					
					printf("Connect Handle(0x%02x), Char Handle(0x%02x)\r\n", p_event->ble_event.write_report.peer.conn_handle,
						p_event->ble_event.write_report.peer.char_handle[0]);

                    // Send the same data into the client
					tal_ble_server_common_send(&send_data);
				}
            } break;
        
        // Connection Parameters Update Report
        case TAL_BLE_EVT_CONN_PARAM_UPDATE: {
                // Show Conn Parameters Info
                
               printf("Conn Param Update: Min = %f ms, Max = %f ms, Latency = %d, Sup = %d ms\n", 
                   p_event->ble_event.conn_param.conn.min_conn_interval * 1.25, 
                   p_event->ble_event.conn_param.conn.max_conn_interval * 1.25,
                   p_event->ble_event.conn_param.conn.latency, 
                   p_event->ble_event.conn_param.conn.conn_sup_timeout * 10);
            } break;
        
        // MTU Exchange Response Event
        case TAL_BLE_EVT_MTU_RSP:{
                // Show MTU Rsponse Data
               printf("Get Response MTU Size = %d", p_event->ble_event.exchange_mtu.mtu);
            } break;
    }
}

int main(void)
{
    tal_ble_bt_init(TAL_BLE_ROLE_PERIPERAL, tuyaos_bluetooth_event_callback);
	printf("11111\n");
    while (true) {
        sleep(1);
    }
}

