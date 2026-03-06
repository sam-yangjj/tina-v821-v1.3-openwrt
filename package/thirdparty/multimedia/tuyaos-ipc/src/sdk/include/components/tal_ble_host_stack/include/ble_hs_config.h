#ifndef _TUYA_BLE_HS_CONFIG_H_
#define _TUYA_BLE_HS_CONFIG_H_

#include "tuya_iot_config.h"

#ifndef TUYA_BLE_HS_BLE_VERSION
#define TUYA_BLE_HS_BLE_VERSION         (50)
#endif

#ifndef TUYA_BLE_HS_THREAD_PRIORITY
#define TUYA_BLE_HS_THREAD_PRIORITY     (4)         // Should be high priority than callout thread
#endif

#ifndef TUYA_BLE_HS_THREAD_STACK_SIZE
#define TUYA_BLE_HS_THREAD_STACK_SIZE   (1024)
#endif

#ifndef TUYA_BLE_HS_CALLOUT_PRIORITY
#define TUYA_BLE_HS_CALLOUT_PRIORITY    (2)
#endif

#ifndef TUYA_BLE_HS_CALLOUT_STACK_SIZE
#define TUYA_BLE_HS_CALLOUT_STACK_SIZE  (1024)
#endif

#ifndef TUYA_BLE_HS_USE_TAL_INTERFACE
#define TUYA_BLE_HS_USE_TAL_INTERFACE   (0)
#endif

/**
 * @brief   For better to understand the host stack, we will list more important informations.
 * */
/******************************************[Tuya] Important Macro Start ******************************************************/
/**< 1, About ble role. */
// For Tuya Ble Peripheral, we need to enable "Broadcaster"(Advertising) + "Observer"(Scan) + "Peripheral"
// For Tuya Ble Central *(within Beacon Central), we need to enable "Broadcaster"(Advertising) + "Observer"(Scan) + "Central"
// For Tuya Ble Peripheral + Central, we need to enable all features.
#ifndef TUYA_BLE_HS_ROLE_BROADCASTER
#define TUYA_BLE_HS_ROLE_BROADCASTER    (1)
#endif

#ifndef TUYA_BLE_HS_ROLE_OBSERVER
#define TUYA_BLE_HS_ROLE_OBSERVER       (1)
#endif

#ifndef TUYA_BLE_HS_ROLE_PERIPHERAL
#define TUYA_BLE_HS_ROLE_PERIPHERAL     (1)
#endif

#ifndef TUYA_BLE_HS_ROLE_CENTRAL
#define TUYA_BLE_HS_ROLE_CENTRAL        (0)
#endif

/**< 2, If you choose Tuya Ble Central, you need to config the max gatt-connections.(Should follow ble controller.) */
#ifndef TUYA_BLE_HS_MAX_CONNECTIONS
#define TUYA_BLE_HS_MAX_CONNECTIONS     (1)
#endif

#ifndef TUYA_BLE_HS_CONNECT_WITHOUT_SCAN
#define TUYA_BLE_HS_CONNECT_WITHOUT_SCAN (1)
#endif

#ifndef TUYA_BLE_HS_CONNECT_CE_LENGTH
#define TUYA_BLE_HS_CONNECT_CE_LENGTH      (32)
#endif

#ifndef TUYA_BLE_HS_CONNECT_CI_LENGTH
#define TUYA_BLE_HS_CONNECT_CI_LENGTH      (48)
#endif

/**< 3, [Important] For different applications, we will use different configurations for saving ram. */
// For Ble HCI Layer(Only Ble&Mesh, Not include ISO Interface), we need to know:
// Full Name -> HCI: Host-Controller-Interface; ACL -> Asynchronous Connection-Less
// HCI Command: Transfer HCI command into controller.
// HCI Event: Get the hci event from controller.
// ACL Transfer: Transfer ACL Packet into controller.
// ACL Receive: Got the ACL Packet from controller.

// #ifndef TUYA_BLE_HS_MEM_USE_STATIC
// #define TUYA_BLE_HS_MEM_USE_STATIC      (0)         // [TimCheng][20211021] Switch buffer into the dynamic memory or static memory
// #endif

/***< Start For HCI Command Buffer Definitions */
#ifndef TUYA_BLE_HCI_CMD_BUF_COUNT
#define TUYA_BLE_HCI_CMD_BUF_COUNT      (2)
#endif

#ifndef TUYA_BLE_HCI_CMD_SIZE
#define TUYA_BLE_HCI_CMD_SIZE           (72)//(260) // Actually, Except ble5.0 extended-adv, we dont need too large size for sending cmd
#endif

/***< Start For HCI Event Buffer Definitions */
#ifndef TUYA_BLE_HCI_EVT_HI_BUF_COUNT
#define TUYA_BLE_HCI_EVT_HI_BUF_COUNT   (8)
#endif

#ifndef TUYA_BLE_HCI_EVT_LO_BUF_COUNT
#define TUYA_BLE_HCI_EVT_LO_BUF_COUNT   (8)
#endif

#ifndef TUYA_BLE_HCI_EVT_REPORT_LIMITED
#define TUYA_BLE_HCI_EVT_REPORT_LIMITED (8)
#endif

#ifndef TUYA_BLE_HCI_ADV_REPORT_LIMITED
#define TUYA_BLE_HCI_ADV_REPORT_LIMITED (24)//(8)
#endif

#ifndef TUYA_BLE_HCI_ADV_REPORT_FILTER
#define TUYA_BLE_HCI_ADV_REPORT_FILTER  (0)         // [TimCheng][20220610] Filter adv report except tuya-adv+mesh
#endif

#ifndef TUYA_BLE_HCI_PB_ADV_REPORT
#define TUYA_BLE_HCI_PB_ADV_REPORT      (1)         // [TimCheng][20220610] Report PB-ADV adv report
#endif

#ifndef TUYA_BLE_HCI_PB_GATT_REPORT
#define TUYA_BLE_HCI_PB_GATT_REPORT     (0)         // [TimCheng][20220610] Report PB-GATT adv report
#endif

#ifndef TUYA_BLE_HCI_SPECIFIC_REPORT
#define TUYA_BLE_HCI_SPECIFIC_REPORT    (1)         // [TimCheng][20220610] Report Tuya Specific adv report, eg: 0xfd50, 0xa201, 0xa300, 0x07d0
#endif

#ifndef TUYA_BLE_HCI_TY_NAME_REPORT
#define TUYA_BLE_HCI_TY_NAME_REPORT     (1)         // [TimCheng][20220610] Report Tuya Specific Name adv report, eg: ty_prod, ty_mdev
#endif

#ifndef TUYA_BLE_HCI_EVT_BUF_SIZE
#define TUYA_BLE_HCI_EVT_BUF_SIZE       (72)        // [TimCheng] !!!Must Align with "OS_ALIGNMENT", especially in dynamic memory
#endif

/***< Start For HCI ACL Receive Buffer Definitions */
// #ifndef TUYA_BLE_HCI_ACL_BUF_COUNT
// #define TUYA_BLE_HCI_ACL_BUF_COUNT      (5)//(24)   // Pay attention to the count in ble central mode
// #endif

#ifndef TUYA_BLE_HCI_ACL_RX_LIMITED
#define TUYA_BLE_HCI_ACL_RX_LIMITED     (12)         // [20220301] We will set the limited count for receiving acl packet after we will dynamic memory
#endif

#ifndef TUYA_BLE_HCI_ACL_BUF_SIZE
#define TUYA_BLE_HCI_ACL_BUF_SIZE       (255)
#endif

/***< Start For HCI ACL Transfer Buffer Definitions And Gatt/Adv/Server/Client Definitions*/
// #ifndef TUYA_BLE_HS_WHOLE_BLOCK_COUNT
// #define TUYA_BLE_HS_WHOLE_BLOCK_COUNT   (5)//(12)//80//12//48//(80)  // mesh = 80 ble = 12/5. 
// #endif

#ifndef TUYA_BLE_HCI_ACL_TX_LIMITED
#define TUYA_BLE_HCI_ACL_TX_LIMITED     (12)         // [20220301] We will set the limited count for transfering acl packet.
#endif

#ifndef TUYA_BLE_HS_WHOLE_BLOCK_SIZE
#define TUYA_BLE_HS_WHOLE_BLOCK_SIZE    (292)
#endif

#ifndef TUYA_BLE_HCI_ACL_DATA_HDR_SZ
#define TUYA_BLE_HCI_ACL_DATA_HDR_SZ    (4)
#endif

#ifndef TUYA_BLE_HCI_DATA_HDR_SZ
#define TUYA_BLE_HCI_DATA_HDR_SZ        (4)
#endif

#ifndef TUYA_BLE_HCI_EVT_HDR_SZ
#define TUYA_BLE_HCI_EVT_HDR_SZ         (3)
#endif

/****************************************** [Tuya] Important Macro End ******************************************************/
#ifndef TUYA_BLE_HS_SM_SC
#define TUYA_BLE_HS_SM_SC               (0)//(1)    // [TimCheng] if using mesh, we need to open sm security.
#endif

#ifndef TY_HS_BLE_PUBLIC_DEV_ADDR
#define TY_HS_BLE_PUBLIC_DEV_ADDR       ((uint8_t[6]){0x21, 0x18, 0x00, 0x00, 0x23, 0xBC})
#endif

#ifndef TY_HS_BLE_SVC_GAP_DEVICE_NAME
#define TY_HS_BLE_SVC_GAP_DEVICE_NAME   ("TY_HS_STACK")
#endif

#ifndef TY_HS_BLE_HCI_VS
#define TY_HS_BLE_HCI_VS                (1)  // [Tim][Ramdom Address], Need to warn, ble address belongs to vendor command, not be union
#endif

#ifndef TY_HS_BLE_ATT_PREFERRED_MTU
#define TY_HS_BLE_ATT_PREFERRED_MTU     (247)
#endif

#ifndef TY_HS_BLE_EXT_ADV
#define TY_HS_BLE_EXT_ADV               (0)
#endif

#ifndef TY_HS_BLE_EXT_ADV_MAX_SIZE
#define TY_HS_BLE_EXT_ADV_MAX_SIZE      (31)
#endif

#ifndef TY_HS_BLE_MAX_PERIODIC_SYNCS
#define TY_HS_BLE_MAX_PERIODIC_SYNCS    (0)
#endif

#ifndef TY_HS_BLE_MULTI_ADV_INSTANCES
#define TY_HS_BLE_MULTI_ADV_INSTANCES   (0)
#endif

#ifndef TY_HS_BLE_PERIODIC_ADV
#define TY_HS_BLE_PERIODIC_ADV          (0)
#endif

#ifndef TY_HS_BLE_PERIODIC_ADV_SYNC_TRANSFER
#define TY_HS_BLE_PERIODIC_ADV_SYNC_TRANSFER (0)
#endif

#ifndef TY_HS_BLE_CONTROLLER
#define TY_HS_BLE_CONTROLLER            (0)         // [TimCheng] if we use self-controller, we will open this macro while building
#endif

// #ifndef TY_HS_OS_MEMPOOL_CHECK
// #define TY_HS_OS_MEMPOOL_CHECK          (0)
// #endif

#ifndef TY_HS_OS_MEMPOOL_GUARD
#define TY_HS_OS_MEMPOOL_GUARD          (0)
#endif

#ifndef TY_HS_OS_MEMPOOL_POISON
#define TY_HS_OS_MEMPOOL_POISON         (0)
#endif

#ifndef TY_HS_OS_SYSVIEW
#define TY_HS_OS_SYSVIEW                (0)
#endif

#ifndef TY_HS_OS_SCHEDULING
#define TY_HS_OS_SCHEDULING             (1)
#endif

#ifndef TY_HS_OS_SYSINIT_STAGE
#define TY_HS_OS_SYSINIT_STAGE          (0)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_CALLOUT
#define TY_HS_OS_SYSVIEW_TRACE_CALLOUT  (1)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_EVENTQ
#define TY_HS_OS_SYSVIEW_TRACE_EVENTQ   (1)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_MBUF
#define TY_HS_OS_SYSVIEW_TRACE_MBUF     (0)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_MEMPOOL
#define TY_HS_OS_SYSVIEW_TRACE_MEMPOOL  (0)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_MUTEX
#define TY_HS_OS_SYSVIEW_TRACE_MUTEX    (1)
#endif

#ifndef TY_HS_OS_SYSVIEW_TRACE_SEM
#define TY_HS_OS_SYSVIEW_TRACE_SEM      (1)
#endif

#ifndef TY_HS_OS_TASK_RUN_TIME_CPUTIME
#define TY_HS_OS_TASK_RUN_TIME_CPUTIME  (0)
#endif

#ifndef TY_HS_BLE_ISO
#define TY_HS_BLE_ISO                   (0)
#endif

#ifndef TY_HS_BLE_ISO_TEST
#define TY_HS_BLE_ISO_TEST              (0)
#endif

#ifndef TY_HS_BLE_WHITELIST
#define TY_HS_BLE_WHITELIST             (1)
#endif

#ifndef TY_HS_BLE_LP_CLOCK
#define TY_HS_BLE_LP_CLOCK              (1)
#endif

#ifndef TY_HS_BLE_NUM_COMP_PKT_RATE
#define TY_HS_BLE_NUM_COMP_PKT_RATE     ((2 * OS_TICKS_PER_SEC))
#endif

#ifndef TY_HS_BLE_ATT_SVR_FIND_INFO
#define TY_HS_BLE_ATT_SVR_FIND_INFO     (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_FIND_TYPE
#define TY_HS_BLE_ATT_SVR_FIND_TYPE     (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_INDICATE
#define TY_HS_BLE_ATT_SVR_INDICATE      (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_MAX_PREP_ENTRIES
#define TY_HS_BLE_ATT_SVR_MAX_PREP_ENTRIES (32)
#endif

#ifndef TY_HS_BLE_ATT_SVR_NOTIFY
#define TY_HS_BLE_ATT_SVR_NOTIFY        (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_QUEUED_WRITE
#define TY_HS_BLE_ATT_SVR_QUEUED_WRITE  (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_QUEUED_WRITE_TMO
#define TY_HS_BLE_ATT_SVR_QUEUED_WRITE_TMO (30000)
#endif

#ifndef TY_HS_BLE_ATT_SVR_READ
#define TY_HS_BLE_ATT_SVR_READ          (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_READ_BLOB
#define TY_HS_BLE_ATT_SVR_READ_BLOB     (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_READ_GROUP_TYPE
#define TY_HS_BLE_ATT_SVR_READ_GROUP_TYPE (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_READ_MULT
#define TY_HS_BLE_ATT_SVR_READ_MULT     (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_READ_TYPE
#define TY_HS_BLE_ATT_SVR_READ_TYPE     (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_SIGNED_WRITE
#define TY_HS_BLE_ATT_SVR_SIGNED_WRITE  (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_WRITE
#define TY_HS_BLE_ATT_SVR_WRITE         (1)
#endif

#ifndef TY_HS_BLE_ATT_SVR_WRITE_NO_RSP
#define TY_HS_BLE_ATT_SVR_WRITE_NO_RSP  (1)
#endif

#ifndef TY_HS_BLE_GAP_MAX_PENDING_CONN_PARAM_UPDATE
#define TY_HS_BLE_GAP_MAX_PENDING_CONN_PARAM_UPDATE (1)
#endif

#ifndef TY_HS_BLE_GATT_DISC_ALL_CHRS
#define TY_HS_BLE_GATT_DISC_ALL_CHRS    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_DISC_ALL_DSCS
#define TY_HS_BLE_GATT_DISC_ALL_DSCS    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_DISC_ALL_SVCS
#define TY_HS_BLE_GATT_DISC_ALL_SVCS    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_DISC_CHR_UUID
#define TY_HS_BLE_GATT_DISC_CHR_UUID    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_DISC_SVC_UUID
#define TY_HS_BLE_GATT_DISC_SVC_UUID    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_FIND_INC_SVCS
#define TY_HS_BLE_GATT_FIND_INC_SVCS    (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_INDICATE
#define TY_HS_BLE_GATT_INDICATE         (1)
#endif

#ifndef TY_HS_BLE_GATT_MAX_PROCS
#define TY_HS_BLE_GATT_MAX_PROCS        (4)
#endif

#ifndef TY_HS_BLE_GATT_NOTIFY
#define TY_HS_BLE_GATT_NOTIFY           (1)
#endif

#ifndef TY_HS_BLE_GATT_READ
#define TY_HS_BLE_GATT_READ             (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_READ_LONG
#define TY_HS_BLE_GATT_READ_LONG        (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_READ_MAX_ATTRS
#define TY_HS_BLE_GATT_READ_MAX_ATTRS   (8)
#endif

#ifndef TY_HS_BLE_GATT_READ_MULT
#define TY_HS_BLE_GATT_READ_MULT        (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_READ_UUID
#define TY_HS_BLE_GATT_READ_UUID        (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_RESUME_RATE
#define TY_HS_BLE_GATT_RESUME_RATE      (1000)
#endif

#ifndef TY_HS_BLE_GATT_SIGNED_WRITE
#define TY_HS_BLE_GATT_SIGNED_WRITE     (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_WRITE
#define TY_HS_BLE_GATT_WRITE            (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_WRITE_LONG
#define TY_HS_BLE_GATT_WRITE_LONG       (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_WRITE_MAX_ATTRS
#define TY_HS_BLE_GATT_WRITE_MAX_ATTRS  (4)
#endif

#ifndef TY_HS_BLE_GATT_WRITE_NO_RSP
#define TY_HS_BLE_GATT_WRITE_NO_RSP     (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_GATT_WRITE_RELIABLE
#define TY_HS_BLE_GATT_WRITE_RELIABLE   (TUYA_BLE_HS_ROLE_CENTRAL)
#endif

#ifndef TY_HS_BLE_HS_AUTO_START
#define TY_HS_BLE_HS_AUTO_START         (1)
#endif

#ifndef TY_HS_BLE_HS_DEBUG
#define TY_HS_BLE_HS_DEBUG              (0)
#endif

#ifndef TY_HS_BLE_HS_FLOW_CTRL
#define TY_HS_BLE_HS_FLOW_CTRL          (0)
#endif

#ifndef TY_HS_BLE_HS_FLOW_CTRL_ITVL
#define TY_HS_BLE_HS_FLOW_CTRL_ITVL     (1000)
#endif

#ifndef TY_HS_BLE_HS_FLOW_CTRL_THRESH
#define TY_HS_BLE_HS_FLOW_CTRL_THRESH   (2)
#endif

#ifndef TY_HS_BLE_HS_FLOW_CTRL_TX_ON_DISCONNECT
#define TY_HS_BLE_HS_FLOW_CTRL_TX_ON_DISCONNECT (0)
#endif

#ifndef TY_HS_BLE_HS_REQUIRE_OS
#define TY_HS_BLE_HS_REQUIRE_OS         (1)
#endif

#ifndef TY_HS_BLE_HS_STOP_ON_SHUTDOWN
#define TY_HS_BLE_HS_STOP_ON_SHUTDOWN   (1)
#endif

#ifndef TY_HS_BLE_HS_STOP_ON_SHUTDOWN_TIMEOUT
#define TY_HS_BLE_HS_STOP_ON_SHUTDOWN_TIMEOUT (2000)
#endif

#ifndef TY_HS_BLE_HS_SYSINIT_STAGE
#define TY_HS_BLE_HS_SYSINIT_STAGE      (200)
#endif

#ifndef TY_HS_BLE_L2CAP_COC_MAX_NUM
#define TY_HS_BLE_L2CAP_COC_MAX_NUM     (0)
#endif

#ifndef TY_HS_BLE_L2CAP_COC_MPS
#define TY_HS_BLE_L2CAP_COC_MPS         (TUYA_BLE_HS_WHOLE_BLOCK_SIZE-8)
#endif

#ifndef TY_HS_BLE_L2CAP_ENHANCED_COC
#define TY_HS_BLE_L2CAP_ENHANCED_COC    (0)
#endif

#ifndef TY_HS_BLE_L2CAP_JOIN_RX_FRAGS
#define TY_HS_BLE_L2CAP_JOIN_RX_FRAGS   (1)
#endif

#ifndef TY_HS_BLE_L2CAP_MAX_CHANS
#define TY_HS_BLE_L2CAP_MAX_CHANS       (3*TUYA_BLE_HS_MAX_CONNECTIONS)
#endif

#ifndef TY_HS_BLE_L2CAP_RX_FRAG_TIMEOUT
#define TY_HS_BLE_L2CAP_RX_FRAG_TIMEOUT (30000)
#endif

#ifndef TY_HS_BLE_L2CAP_SIG_MAX_PROCS
#define TY_HS_BLE_L2CAP_SIG_MAX_PROCS   (1)
#endif

#ifndef TY_HS_BLE_RPA_TIMEOUT
#define TY_HS_BLE_RPA_TIMEOUT           (300)
#endif

#ifndef TY_HS_BLE_SM_BONDING
#define TY_HS_BLE_SM_BONDING            (0)
#endif

#ifndef TY_HS_BLE_SM_IO_CAP
#define TY_HS_BLE_SM_IO_CAP             (BLE_HS_IO_NO_INPUT_OUTPUT)
#endif

#ifndef TY_HS_BLE_SM_KEYPRESS
#define TY_HS_BLE_SM_KEYPRESS           (0)
#endif

#ifndef TY_HS_BLE_SM_LEGACY
#define TY_HS_BLE_SM_LEGACY             (0)
#endif

#ifndef TY_HS_BLE_SM_MAX_PROCS
#define TY_HS_BLE_SM_MAX_PROCS          (1)
#endif

#ifndef TY_HS_BLE_SM_MITM
#define TY_HS_BLE_SM_MITM               (0)
#endif

#ifndef TY_HS_BLE_SM_OOB_DATA_FLAG
#define TY_HS_BLE_SM_OOB_DATA_FLAG      (0)
#endif

#ifndef TY_HS_BLE_SM_OUR_KEY_DIST
#define TY_HS_BLE_SM_OUR_KEY_DIST       (0)
#endif

#ifndef TY_HS_BLE_SM_SC_DEBUG_KEYS
#define TY_HS_BLE_SM_SC_DEBUG_KEYS      (0)
#endif

#ifndef TY_HS_BLE_SM_THEIR_KEY_DIST
#define TY_HS_BLE_SM_THEIR_KEY_DIST     (0)
#endif

#ifndef TY_HS_BLE_STORE_MAX_BONDS
#define TY_HS_BLE_STORE_MAX_BONDS       (3)
#endif

#ifndef TY_HS_BLE_STORE_MAX_CCCDS
#define TY_HS_BLE_STORE_MAX_CCCDS       (8)
#endif

#ifndef TY_HS_BLE_SVC_GAP_APPEARANCE
#define TY_HS_BLE_SVC_GAP_APPEARANCE    (0)
#endif

#ifndef TY_HS_BLE_SVC_GAP_APPEARANCE_WRITE_PERM
#define TY_HS_BLE_SVC_GAP_APPEARANCE_WRITE_PERM         (-1)
#endif

#ifndef TY_HS_BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION
#define TY_HS_BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION    (-1)
#endif

#ifndef TY_HS_BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH
#define TY_HS_BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH        (31)
#endif

#ifndef TY_HS_BLE_SVC_GAP_DEVICE_NAME_WRITE_PERM
#define TY_HS_BLE_SVC_GAP_DEVICE_NAME_WRITE_PERM        (-1)
#endif

#ifndef TY_HS_BLE_SVC_GAP_PPCP_MAX_CONN_INTERVAL
#define TY_HS_BLE_SVC_GAP_PPCP_MAX_CONN_INTERVAL        (0)
#endif

#ifndef TY_HS_BLE_SVC_GAP_PPCP_MIN_CONN_INTERVAL
#define TY_HS_BLE_SVC_GAP_PPCP_MIN_CONN_INTERVAL        (0)
#endif

#ifndef TY_HS_BLE_SVC_GAP_PPCP_SLAVE_LATENCY
#define TY_HS_BLE_SVC_GAP_PPCP_SLAVE_LATENCY            (0)
#endif

#ifndef TY_HS_BLE_SVC_GAP_PPCP_SUPERVISION_TMO
#define TY_HS_BLE_SVC_GAP_PPCP_SUPERVISION_TMO          (0)
#endif

#ifndef TY_HS_BLE_SVC_GAP_SYSINIT_STAGE
#define TY_HS_BLE_SVC_GAP_SYSINIT_STAGE                 (301)
#endif

#ifndef TY_HS_BLE_SVC_GATT_SYSINIT_STAGE
#define TY_HS_BLE_SVC_GATT_SYSINIT_STAGE                (302)
#endif

#ifndef TY_HS_BLE_TRANS_RAM_SYSINIT_STAGE
#define TY_HS_BLE_TRANS_RAM_SYSINIT_STAGE               (100)
#endif

#endif
