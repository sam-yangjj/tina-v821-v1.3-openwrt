#ifndef __TKL_HCI_SILICON_RCP_H__
#define __TKL_HCI_SILICON_RCP_H__


#ifdef __cplusplus
    extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// For Log TimeStamp
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "tuya_cloud_types.h"

/***************************************** Follow Silicon Gecko SDK **************************************************/
enum gecko_msg_types
{
    gecko_msg_type_cmd=0x00,
    gecko_msg_type_rsp=0x00,
    gecko_msg_type_evt=0x80
};

enum gecko_dev_types
{
    gecko_dev_type_gecko   =0x20
};

#define gecko_cmd_dfu_reset_id                  (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00000000)
#define gecko_cmd_dfu_flash_set_address_id      (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01000000)
#define gecko_cmd_dfu_flash_upload_id           (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02000000)
#define gecko_cmd_dfu_flash_upload_finish_id    (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03000000)
#define gecko_cmd_system_hello_id               (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00010000)
#define gecko_cmd_system_reset_id               (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01010000)
#define gecko_cmd_user_message_to_target_id     (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00ff0000)


#define gecko_cmd_mesh_prov_init_id             (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00150000)
#define gecko_cmd_mesh_node_get_seq_remaining_id    (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f140000)
#define gecko_cmd_mesh_node_get_ivupdate_state_id   (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d140000)
#define gecko_cmd_mesh_test_get_element_seqnum_id   (((uint32_t)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1e220000)

/********************************************************************************************************************/

typedef enum {
    SILICON_STACK_MODE          = 0x00,
    SILICON_CHECK_MODE          = 0x01,
    SILICON_UPGRADE_MODE        = 0x02,
    SILICON_SILIENCE            = 0x03,
}TKL_HCI_SILICON_MODE_E;

#define TKL_HCI_SILICON_READ_COUNT              50  
#define TKL_HCI_SILICON_READ_TIMEOUT            10      // Unit: ms 
#define TKL_HCI_SILICON_READ_SIZE               128  
#define TKL_HCI_SILICON_WRITE_PAYLOAD_MAX       256

#define BGLIB_MSG_HEADER_LEN                    (4)
#define BGLIB_MSG_LEN(HDR)                      ((((HDR)&0x7)<<8)|(((HDR)&0xff00)>>8))

#define TKL_HCI_SILICON_COMMAND_VERSION_READ    (0xA0)

#define TKL_HCI_VENDOR_COMMAND_PORT_SET         (0x0200)
#define TKL_HCI_VENDOR_COMMAND_VERSION_READ     (0x0201)
#define TKL_HCI_VENDOR_COMMAND_RCP_EXIST        (0x0202)
#define TKL_HCI_VENDOR_COMMAND_RCP_BOOT_ENTER   (0x0203)
#define TKL_HCI_VENDOR_COMMAND_NCP_EXIST        (0x0204)
#define TKL_HCI_VENDOR_COMMAND_NCP_BOOT_ENTER   (0x0205)
#define TKL_HCI_VENDOR_COMMAND_BOOT_CHECK       (0x0206)
#define TKL_HCI_VENDOR_COMMAND_FW_UPLOAD        (0x0207)
#define TKL_HCI_VENDOR_COMMAND_UPLOAD_FINISH    (0x0208)
#define TKL_HCI_VENDOR_COMMAND_BOOT_EXIT        (0x0209)
#define TKL_HCI_VENDOR_COMMAND_HW_RESET         (0x020A)

typedef VOID(*TKL_HCI_SILICON_CALLBACK)(VOID);

extern TKL_HCI_SILICON_CALLBACK tkl_hci_hw_reset_callback;

CHAR_T *tkl_hci_silicon_get_time(VOID);

VOID tkl_hci_silicon_dump_printf(CHAR_T* prmote, UCHAR_T *pdata, UINT_T len);

#define TKL_HCI_SILICON_DEBUG(fmt, ...)                 printf("[%s] %s(%d):" fmt "\n", tkl_hci_silicon_get_time(), __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define TKL_HCI_SILICON_DUMP(prmote, pdata, len)        tkl_hci_silicon_dump_printf(prmote, (UCHAR_T *)pdata, len) 

#if defined(ENABLE_LOG) && (ENABLE_LOG==1)
#include "tal_log.h"
#define TKL_HCI_LOG_DEBUG(fmt, ...)                 TAL_PR_DEBUG(fmt, ##__VA_ARGS__)
#define TKL_HCI_LOG_TRACE(fmt, ...)                 TAL_PR_TRACE(fmt, ##__VA_ARGS__)

#define TKL_HCI_DUMP_LOG_DEBUG(head, data, len)     TAL_PR_HEXDUMP_DEBUG(head,data, len)
#define TKL_HCI_DUMP_LOG_TRACE(head, data, len)     TAL_PR_HEXDUMP_TRACE(head,data, len)
#else
#define TKL_HCI_LOG_DEBUG(fmt, ...)                 TKL_HCI_SILICON_DEBUG(fmt, ##__VA_ARGS__)
#define TKL_HCI_LOG_TRACE(fmt, ...)                 

#define TKL_HCI_DUMP_LOG_DEBUG(head, data, len)     TKL_HCI_SILICON_DUMP(head,data, len)
#define TKL_HCI_DUMP_LOG_TRACE(head, data, len)     
#endif

#if ((defined TAL_HCI_CONFIG_BAUDRATE) && (TAL_HCI_CONFIG_BAUDRATE >= 115200))
#define TKL_HCI_RCP_BAUDRATE        TAL_HCI_CONFIG_BAUDRATE
#else
#define TKL_HCI_RCP_BAUDRATE        921600
#endif

#if ((defined TAL_HCI_CONFIG_FLOW_CONTROL) && (TAL_HCI_CONFIG_FLOW_CONTROL == 1))
#define TKL_HCI_RCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_NONE
#elif ((defined TAL_HCI_CONFIG_FLOW_CONTROL) && (TAL_HCI_CONFIG_FLOW_CONTROL == 2))
#define TKL_HCI_RCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_RTSCTS
#elif ((defined TAL_HCI_CONFIG_FLOW_CONTROL) && (TAL_HCI_CONFIG_FLOW_CONTROL == 3))
#define TKL_HCI_RCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_XONXOFF
#elif ((defined TAL_HCI_CONFIG_FLOW_CONTROL) && (TAL_HCI_CONFIG_FLOW_CONTROL == 4))
#define TKL_HCI_RCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_DTRDSR
#else
#define TKL_HCI_RCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_RTSCTS
#endif

#ifndef TKL_HCI_RCP_PARITY
#define TKL_HCI_RCP_PARITY          TUYA_UART_PARITY_TYPE_NONE
#endif

#ifndef SILICON_NCP_BAUDRATE
#define SILICON_NCP_BAUDRATE        115200
#endif

#ifndef SILICON_NCP_FLOW_CONTROL
#define SILICON_NCP_FLOW_CONTROL    TUYA_UART_FLOWCTRL_NONE
#endif

OPERATE_RET tkl_hci_silicon_uart_init(UINT_T port_id, UINT_T baudrate, TUYA_UART_FLOWCTRL_TYPE_E flowctrl);

VOID tkl_hci_silicon_uart_flush(UINT_T port_id);

OPERATE_RET tkl_hci_silicon_uart_deinit(UINT_T port_id);

VOID tkl_hci_silicon_set_mode(TKL_HCI_SILICON_MODE_E mode);

TKL_HCI_SILICON_MODE_E tkl_hci_silicon_get_mode(VOID);

OPERATE_RET tkl_hci_silicon_fw_upload(UINT_T port_id, UCHAR_T *fw_data, USHORT_T fw_data_len);

OPERATE_RET tkl_hci_silicon_fw_upload_finish(UINT_T port_id);

OPERATE_RET tkl_hci_silicon_upgrade_reset(UINT_T port_id, UCHAR_T dfu_mode);

OPERATE_RET tkl_hci_silicon_set_address(UINT_T port_id, UINT_T address);

OPERATE_RET tkl_hci_silicon_rcp_fw_check(UINT_T port_id, UCHAR_T rcp_version[3]);

OPERATE_RET tkl_hci_silicon_rcp_boot_enter(UINT_T port_id);

OPERATE_RET tkl_hci_silicon_ncp_fw_check(UINT_T port_id, UCHAR_T ncp_version[3]);

OPERATE_RET tkl_hci_silicon_ncp_boot_enter(UINT_T port_id);

#ifdef __cplusplus
}
#endif

#endif
