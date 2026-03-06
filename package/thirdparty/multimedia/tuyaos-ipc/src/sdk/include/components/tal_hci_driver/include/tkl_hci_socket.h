#ifndef __TKL_HCI_SOCKET_H__
#define __TKL_HCI_SOCKET_H__


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


CHAR_T *tkl_hci_socket_get_time(VOID);

VOID tkl_hci_socket_dump_printf(CHAR_T* prmote, UCHAR_T *pdata, UINT_T len);

#define TKL_HCI_SOCKET_DEBUG(fmt, ...)                 printf("[%s] %s(%d):" fmt "\n", tkl_hci_socket_get_time(), __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define TKL_HCI_SOCKET_DUMP(prmote, pdata, len)        tkl_hci_socket_dump_printf(prmote, (UCHAR_T *)pdata, len) 

#if defined(ENABLE_LOG) && (ENABLE_LOG==1)
#include "tal_log.h"
#define TKL_HCI_LOG_DEBUG(fmt, ...)                 TAL_PR_DEBUG(fmt, ##__VA_ARGS__)
#define TKL_HCI_LOG_TRACE(fmt, ...)                 TAL_PR_TRACE(fmt, ##__VA_ARGS__)

#define TKL_HCI_DUMP_LOG_DEBUG(head, data, len)     TAL_PR_HEXDUMP_DEBUG(head,data, len)
#define TKL_HCI_DUMP_LOG_TRACE(head, data, len)     TAL_PR_HEXDUMP_TRACE(head,data, len)
#else
#define TKL_HCI_LOG_DEBUG(fmt, ...)                 TKL_HCI_SOCKET_DEBUG(fmt, ##__VA_ARGS__)
#define TKL_HCI_LOG_TRACE(fmt, ...)                 

#define TKL_HCI_DUMP_LOG_DEBUG(head, data, len)     TKL_HCI_SOCKET_DUMP(head,data, len)
#define TKL_HCI_DUMP_LOG_TRACE(head, data, len)     
#endif


#ifdef __cplusplus
}
#endif

#endif
