#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "tkl_wired.h"
#include "tuya_cloud_types.h"


void showusage(char *argv)
{
    printf("%s get sta      :获取连接状态\n", argv);
    printf("%s get ip type  :获取ip地址, type is 0:ipv4, 1:ipv6, 2:ipv6_ll\n", argv);
    printf("%s get mac      :获取mac地址\n", argv);
    printf("%s set mac      :设置mac地址 xx:xx:xx:xx:xx:xx\n", argv);
    printf("%s cb           :回调函数测试\n", argv);
}

void cb(TKL_WIRED_STAT_E status)
{
    if (status == TKL_WIRED_LINK_DOWN)
        printf("linuk status: down\n");
    else if (status == TKL_WIRED_LINK_UP)
        printf("linuk status: up\n");
}

int main(int argc,char **argv)
{
	OPERATE_RET ret;

    if ( argc == 3 && !strcmp(argv[1], "get") && !strcmp(argv[2], "sta")) {
        TKL_WIRED_STAT_E status;

        ret = tkl_wired_get_status(&status);
        if (ret == 0)
            printf("status:%d\n", status);

        return ret;
    }

    if ( argc == 4 && !strcmp(argv[1], "get") && !strcmp(argv[2], "ip") ) {
        NW_IP_S ip;
        NW_IP_TYPE type = atoi( argv[3] );
        ret = tkl_wired_get_ip(type, &ip);
        if (ret == 0) {
            if ( type == NW_IPV4 ) {
                printf("ipv4:%s mask:%s gw:%s\n", ip.addr.ip4.ip, ip.addr.ip4.mask, ip.addr.ip4.gw);
            }
            else if ( type == NW_IPV6 || type == NW_IPV6_LL ) {
                printf("ipv6:%s, islinklocal=%d\n", ip.addr.ip6.ip, ip.addr.ip6.islinklocal);
            }
        }

        return ret;
    }

    if ( argc == 3 && !strcmp(argv[1], "get") && !strcmp(argv[2], "mac")) {
        NW_MAC_S mac;

        ret = tkl_wired_get_mac(&mac);
        if (ret == 0) {
            printf("mac %02x:%02x:%02x:%02x:%02x:%02x\n", mac.mac[0],
                mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);
        }

        return ret;
    }

    if ( argc == 4 && !strcmp(argv[1], "set") && !strcmp(argv[2], "mac")) {
        int i;
        NW_MAC_S set_mac;
        sscanf( argv[3], "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &set_mac.mac[0], &set_mac.mac[1], &set_mac.mac[2], &set_mac.mac[3], &set_mac.mac[4], &set_mac.mac[5] );
        printf("input mac %02x:%02x:%02x:%02x:%02x:%02x\n", set_mac.mac[0], set_mac.mac[1], set_mac.mac[2], set_mac.mac[3], set_mac.mac[4], set_mac.mac[5]);
        ret = tkl_wired_set_mac(&set_mac);
        if (ret)
            printf("tkl_wired_set_mac failed, ret: %d\n", ret);

        return ret;
    }

    if ( argc == 2 && !strcmp(argv[1], "cb") ) {
        ret = tkl_wired_set_status_cb(cb);
        sleep(60);
        return ret;
    }

    showusage(argv[0]);

    return 0;
}
