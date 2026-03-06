/*
 * tuya_ipc_lowpower_api.c
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stddef.h>
#include "lp_auth.h"
#include "tuya_cloud_types.h"

static lp_auth_dev_msg_s g_devMsg={-1,"",0,"",0};
static unsigned int low_power_wake_up_date_seed = 0;

int tuya_ipc_low_power_server_connect(TUYA_IP_ADDR_T serverIp,signed int port,char* pdevId, int idLen, char* pkey, int keyLen)
{
    //
    if(pdevId == NULL || pkey == NULL || idLen == 0 || keyLen == 0)
    {
        return -2;
    }
    //TODO 重复连接处理
    if(g_devMsg.sock >= 0)
    {
        printf("already conncect server,not connect again\n");
        return 0;
    }

    if(lp_cli_init_dev_info(&g_devMsg, pdevId, idLen, pkey, keyLen) < 0)
    {
        printf("init dev info error\n");

        return -1;
    }

    g_devMsg.sock = lp_cli_connect_server(serverIp, port);
    if(g_devMsg.sock < 0)
    {
        printf("connect  lowpower server is error\n");
        memset(&g_devMsg,0,sizeof(g_devMsg));
        return -1;
    }

    if(lp_cli_auth(g_devMsg.sock, &g_devMsg) < 0)
    {
        printf("auth lowpower is error");
        close(g_devMsg.sock);
        memset(&g_devMsg,0,sizeof(g_devMsg));
        return -1;
    }
    low_power_wake_up_date_seed = lp_cli_wake_up_data_seed_computer(pkey, keyLen);

    printf("connect lowpower is success fd=%d,seed is =%d", g_devMsg.sock,low_power_wake_up_date_seed);
    return 0;
}

void tuya_ipc_low_power_server_close()
{
    if(g_devMsg.sock < 0)
    {
        return ;
    }
    close(g_devMsg.sock);
    return;
}
int tuya_ipc_low_power_socket_fd_get()
{
    return g_devMsg.sock;
}

int tuya_ipc_low_power_wakeup_data_get( char* pdata,  unsigned int* plen)
{
    return lp_protocol_wakeup(pdata,plen,low_power_wake_up_date_seed);
}

int tuya_ipc_low_power_heart_beat_get( char * pdata, unsigned int *plen)
{
    return lp_protocol_heartbeat(pdata,plen);
}
