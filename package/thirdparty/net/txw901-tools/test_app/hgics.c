/**
  ******************************************************************************
  * @file    hgics.c
  * @author  HUGE-IC Application Team
  * @version V1.0.0
  * @date    2022-05-18
  * @brief   hgic smac driver daemon.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 HUGE-IC</center></h2>
  *
  ******************************************************************************
  */

// #include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "hgic.h"

#include "iwpriv.c"

#define AUTO_RELAY_EN 0
#define BELNC_EN 1

struct hgic_fw_info hgic_fwinfo;

//////////////////////////////////////////////////////////////////////////////////////////////////

// static void hgics_parse_blenc_param(u8 *data, int len)
// {
//     u8 *ptr = data;
// #if 1 //sample code
//     u8 buff[33];
//     u8 cmd[64];
//     char out[128];
//     while (ptr < data + len) {
//         switch (ptr[0]) {
//             case 1: //SSID
//                 memset(buff, 0, sizeof(buff));
//                 memcpy(buff, ptr + 2, ptr[1]);
//                 printf("SET ssid:%s\r\n", buff);
//                 sprintf(cmd, "wpa_cli -i wlan0 set_network 0 ssid '\"%s\"'", buff);
//                 hgic_do_system(cmd, out, 100);
//                 printf("%s: %s\r\n", cmd, out);
//                 //sprintf(cmd, "nvram_set ah_ssid %s", buff);
//                 //system(cmd);
//                 break;
//             case 2: //PassWord
//                 memset(buff, 0, sizeof(buff));
//                 memcpy(buff, ptr + 2, ptr[1]);
//                 printf("SET passwd:%s\r\n", buff);
//                 sprintf(cmd, "wpa_cli -i wlan0 set_network 0 psk '\"%s\"'", buff);
//                 hgic_do_system(cmd, out, 100);
//                 printf("%s: %s\r\n", cmd, out);
//                 //sprintf(cmd, "nvram_set ah_psk %s", buff);
//                 //system(cmd);
//                 break;
//             case 3: //Keymgmt
//                 printf("SET keymgmt %s\r\n", ptr[2] ? "WPA-PSK" : "NONE");
//                 sprintf(cmd, "wpa_cli -i wlan0 set_network 0 key_mgmt %s", ptr[2] ? "WPA-PSK" : "NONE");
//                 hgic_do_system(cmd, out, 100);
//                 printf("%s: %s\r\n", cmd, out);
//                 //sprintf(cmd, "nvram_set ah_key_mgmt %s", ptr[2] ? "WPA-PSK" : "NONE");
//                 //system(cmd);
//                 break;
//             case 4: //auth
//                 printf("AUTH %d\r\n", ptr[2]);
//                 sprintf(cmd, "ifconfig wlan0 %s", ptr[2] ? "up" : "down");
//                 system(cmd);
//                 break;
//             default:
//                 printf("Unsupport ID:%d\r\n", ptr[0]);
//                 break;
//         }
//         ptr += (ptr[1] + 2);
//     }
//     hgic_blenc_release();
//     hgic_iwpriv_blenc_start("wlan0", 0, 38);
//     strcpy(cmd, "wpa_cli -i wlan0 save_config");
//     printf("%s: %s\r\n", cmd,out);
//     hgic_do_system(cmd, out, 100);
//     //system("smac.sh"); //restart hostapd or wpa_supplicant
// #endif
// }

// /*泰芯微信小程序配网数据 | Weixin applet data*/
// static void hgics_recv_blenc_data(uint8 *data, uint32 len)
// {
//     uint8 *ncdata = NULL;
//     uint32 data_len = 0;

//     //hgic_dump_hex("BLE DATA:\r\n", data, len, 1);
//     if (hgic_blenc_parse_data(data, len)) {
//         data_len = hgic_blenc_get_data(&ncdata);
//         if (data_len && ncdata) {
//             hgics_parse_blenc_param(ncdata, data_len);
//         }
//     }
// }

// /* customer protocol
//    data: BLE PDU data
//    len:  data length
// */
// static void hgics_recv_customer_ble_data(u8 *data, int len)
// {
// }

// static void hgics_proc_fwevent(u8 *event_data, u32 event_len)
// {
//     u32   data_len = 0;
//     u32   evt_id   = 0;
//     char *data     = NULL;
//     struct hgic_ctrl_hdr *evt = (struct hgic_ctrl_hdr *)event_data;

//     data     = (char *)(evt + 1);
//     data_len = event_len - sizeof(struct hgic_ctrl_hdr);
//     evt_id   = HDR_EVTID(evt);

//     switch (evt_id) {
//         case HGIC_EVENT_BLENC_DATA: //process BLE Advertising data. BLE work mode 1 or 2.
// #if 1       
//             /*微信小程序配网测试 | for Weixin applet*/            
//             hgics_recv_blenc_data(data, data_len); //ble work mode 1
// #else 
//             /*自定义广播配网 | customer protocol*/
//             hgics_recv_customer_ble_data(data, data_len); //ble work mode 2
// #endif
//             break;

//         case HGIC_EVENT_HWSCAN_RESULT:
//             #if AUTO_RELAY_EN
//             hgics_relay_check_hwscan_result(data, data_len);
//             #endif
//             break;

//         case HGIC_EVENT_FW_INIT_DONE:
//             // if(hgic_fwinfo.version){ //maybe wifi chip has been reset.
//             //     printf("firmware init done, restart hostapd or wpa_supplicant!!!!\r\n");
//             //     system("/mnt/extsd/smac.sh");
//             // }
//             break;
//         default:
//             printf("ignore event %d\r\n", evt_id);
//             break;
//     }
// }


int main(int argc, char *argv[])
{
    int i = 0;
    int ret = 0;
    int fd  = -1;
    int wifi_mode = 0;
    struct hgic_hdr *hdr;
    char *ssid = NULL;
    char *buff = NULL;

    HGIC = "hgics"; //for iwpriv.c

    buff = malloc(4096);
    if (buff == NULL) {
        printf("malloc fail\r\n");
        return -1;
    }

//     for (i = 1; i < argc; i++) {
//         if (strcmp(argv[i], "sta") == 0) {
//             wifi_mode = 0;
//         } else if (strcmp(argv[i], "ap") == 0) {
//             wifi_mode = 1;
//         } else if (strcmp(argv[i], "apsta") == 0) {
//             wifi_mode = 2;
//         } else {
//             ssid = argv[i];
//         }
//     }

//     hgic_blenc_init(); /* 支持微信小程序配网测试 | Just for Weixin applet*/

// #if AUTO_RELAY_EN //sample code
//     hgics_relay_init(ssid, wifi_mode);
//     if (wifi_mode == 2) {
//         system("brctl delif br0 eth2");
//         system("brctl addif br0 wlan0");
//         system("brctl addif br0 wlan1");
//         system("ifconfig br0 192.168.1.30");
//         system("ifconfig eth2 10.10.10.30");
//     }
// #endif

__open:
    fd = open("/proc/hgics/fwevnt", O_RDONLY);
    if (fd == -1) {
        sleep(1); //sleep 1s and try again.
        goto __open;
    }

    //get firmware version
    hgic_iwpriv_get_fwinfo("wlan0", &hgic_fwinfo);
    printf("firmware version: v%d.%d.%d.%d\r\n", 
                (hgic_fwinfo.version >> 24) & 0xff, (hgic_fwinfo.version >> 16) & 0xff,
                (hgic_fwinfo.version >> 8) & 0xff, (hgic_fwinfo.version & 0xff));

#if 1 //BLE demo code for work mode 3.
    do {
        /* 
           广播数据 和 扫描响应数据 的填充格式：[长度,类型,数据...] 
           类型定义列表 请查阅文档《BLE_Assigned_Numbers.pdf》， 章节：2.3 Common Data Types.
           根据应用需求，可以修改或新增填充数据。

           Advertising_Data and Scan_Response Format: [length,Type,Data...] 
           Document: BLE_Assigned_Numbers.pdf, Section 2.3,  Common Data Types.
           You can modify the data or add more data in adv_data and scan_resp.
        */
        
        /* 扫描响应数据 Scan_Response Data */
        u8 scan_resp[] = {
                          /*Length  Type   Data */
                            0x04,  0x09,  'S','S','S', /* the name is "SSS" */
                          /* type 0x09: Complete Local Name. document: Core_Specification_Supplement.pdf, Part A, Section 1.2 */
                          /* 类型 0x09: 设备名称. 查阅文档：Core_Specification_Supplement.pdf, Part A, Section 1.2               */

                          /*Length  Type   Data */
                            0x19,  0xFF,  0xD0,0x07,0x01,0x03,0x00,0x00,0x0C,0x00,0x88,0xD1,0xC4,0x89,0x2B,0x56,0x7D,0xE5,0x65,0xAC,0xA1,0x3F,0x09,0x1C,0x43,0x92
                          /* type 0xFF: Manufacturer Specific Data. */
                          /* 类型 0xFF: 厂家自定义数据. */
         };

        /* 广播数据 Advertising_Data */
        u8 adv_data[] = {
                        /*Length  Type   Data */
                          0x02,   0x01, 0x06,
                        /* type 0x01: Flags. document: Core_Specification_Supplement.pdf, Part A, Section 1.3 */
                        /* 类型 0x01: 广播标识，各个bit的含义请查阅文档：Core_Specification_Supplement.pdf, Part A, Section 1.3 */
                        /////////////////////////////////////////////////////////////////////////////////////

                        /*Length  Type   Data */
                          0x03,   0x02, 0x01,0xA2, /* uuid 0x2A01 */
                        /* type 0x02: Incomplete List of 16­bit Service Class UUIDs. document: Core_Specification_Supplement.pdf, Part A, Section 1.1 */
                        /* 类型 0x02: 16bit UUID列表：对应到ATT Table里面定义的UUID. 查阅文档: Core_Specification_Supplement.pdf, Part A, Section 1.1 */
                        /////////////////////////////////////////////////////////////////////////////////////

                        /*Length  Type   Data */ 
                          0x14,   0x16, 0x01,0xA2,0x01,0x6B,0x65,0x79,0x79,0x66,0x67,0x35,0x79,0x33,0x34,0x79,0x71,0x78,0x71,0x67,0x64
                        /* type 0x16: Service Data - 16 bit UUID. document: Core_Specification_Supplement.pdf, Part A, Section 1.11 */
                        /* 类型 0x16:   16bit UUID的服务数据. 查阅文档: Core_Specification_Supplement.pdf, Part A, Section 1.11 */
                        /////////////////////////////////////////////////////////////////////////////////////
          };

        /*设置广播数据 |       Config the Advertising Data*/
        hgic_iwpriv_blenc_set_advdata("wlan0", adv_data, sizeof(adv_data));

        /*设置扫描响应数据 |         Config the Scan Response Data*/
        hgic_iwpriv_blenc_set_scanresp("wlan0", scan_resp, sizeof(scan_resp));

        /* 打开广播功能 |       Enable Advertising Function */
        hgic_iwpriv_blenc_start_adv("wlan0", 1);

        /* 启动进入BLE模式3 |       Start BLE with work mode 3*/
        hgic_iwpriv_blenc_start("wlan0", 3, 38); //start BLE MODE 3

        /* 停止退出BLE模式 | Stop BLE */        
        //hgic_iwpriv_blenc_start("wlan0", 0, 38); //stop BLE
    } while (0);
#endif

    /* start read event from driver and process the event data.*/
    hdr = (struct hgic_hdr *)buff;
    while (1) {
        /* If there is no event data when calling the read API, the driver will block the current thread for 100ms.
           you can see it in the function hgics_fwevent_read. */
        ret = read(fd, buff, 4096);

        if (ret > 0) {
            switch (hdr->type) {
                case HGIC_HDR_TYPE_EVENT:
                case HGIC_HDR_TYPE_EVENT2:
                    // hgics_proc_fwevent(buff, ret); //process the events from firmware.
                    break;
                case HGIC_HDR_TYPE_BLUETOOTH:      //process the bluetooth data from firmware. [ble work mode 3]
                    #if BELNC_EN
                    hgic_proc_bt_data(buff, ret);
                    #endif
                    break;
                default:
                    printf("unknown hdr type:%d\r\n", hdr->type);
                    break;
            }
        } else if (ret == 0) { //no data and timeout.
            // #if AUTO_RELAY_EN
            // if(wifi_mode == 2){
            //     hgics_relay_check_status();
            // }
            // #endif

            // #if 1 //BLE Notify test code
            // hgic_gatt_notify_test();
            // #endif
        } else { // read error.
            printf("read error, ret=%d\r\n", ret);
            close(fd);
            goto __open;
        }
    }

    close(fd);
    free(buff);
    return 0;
}

