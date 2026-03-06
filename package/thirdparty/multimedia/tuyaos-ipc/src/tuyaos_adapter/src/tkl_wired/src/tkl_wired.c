/**
 * @file tkl_wired.c
 * @brief the default weak implements of wired driver, this implement only used when OS=linux
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 * 
 */
#include "tuya_cloud_types.h"
#include "tkl_wired.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/sockios.h>
#include <errno.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <linux/types.h>


#define WIRED_NAME "eth0"
#define ROUTE_PATH "/proc/net/route"

#define IPV6_ADDR_ANY		    0x0000U
#define IPV6_ADDR_UNICAST      	0x0001U	
#define IPV6_ADDR_MULTICAST    	0x0002U	
#define IPV6_ADDR_LOOPBACK	    0x0010U
#define IPV6_ADDR_LINKLOCAL	    0x0020U
#define IPV6_ADDR_SITELOCAL	    0x0040U
#define IPV6_ADDR_COMPATv4	    0x0080U
#define IPV6_ADDR_SCOPE_MASK	0x00f0U
#define IPV6_ADDR_MAPPED	    0x1000U

#define IPV6_ADDR_MC_SCOPE(a)	((a)->s6_addr[1] & 0x0f)	/* nonstandard */
#define __IPV6_ADDR_SCOPE_INVALID	-1
#define IPV6_ADDR_SCOPE_NODELOCAL	0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL	0x02
#define IPV6_ADDR_SCOPE_SITELOCAL	0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL	0x08
#define IPV6_ADDR_SCOPE_GLOBAL		0x0e

#define IPV6_ADDR_SCOPE_TYPE(scope) ((scope) << 16)

static char iface_name[IFNAMSIZ];

static inline unsigned int ipv6_addr_scope2type(unsigned int scope)
{
	switch (scope) {
	case IPV6_ADDR_SCOPE_NODELOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_NODELOCAL) |
			IPV6_ADDR_LOOPBACK);
	case IPV6_ADDR_SCOPE_LINKLOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL) |
			IPV6_ADDR_LINKLOCAL);
	case IPV6_ADDR_SCOPE_SITELOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_SITELOCAL) |
			IPV6_ADDR_SITELOCAL);
	}
	return IPV6_ADDR_SCOPE_TYPE(scope);
}

static inline int __ipv6_addr_type(const struct in6_addr *addr)
{
    __be32 st;
    st = addr->s6_addr32[0];

    /* Consider all addresses with the first three bits different of
       000 and 111 as unicasts.
       */
    if ((st & htonl(0xE0000000)) != htonl(0x00000000) && (st & htonl(0xE0000000)) != htonl(0xE0000000))
        return (IPV6_ADDR_UNICAST |
                IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));

    if ((st & htonl(0xFF000000)) == htonl(0xFF000000)) {
        /* multicast */
        /* addr-select 3.1 */
        return (IPV6_ADDR_MULTICAST |
                ipv6_addr_scope2type(IPV6_ADDR_MC_SCOPE(addr)));
    }

    if ((st & htonl(0xFFC00000)) == htonl(0xFE800000))
        return (IPV6_ADDR_LINKLOCAL | IPV6_ADDR_UNICAST |
                IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL));		/* addr-select 3.1 */
    if ((st & htonl(0xFFC00000)) == htonl(0xFEC00000))
        return (IPV6_ADDR_SITELOCAL | IPV6_ADDR_UNICAST |
                IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_SITELOCAL));		/* addr-select 3.1 */
    if ((st & htonl(0xFE000000)) == htonl(0xFC000000))
        return (IPV6_ADDR_UNICAST |
                IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));			/* RFC 4193 */

    if ((addr->s6_addr32[0] | addr->s6_addr32[1]) == 0) {
        if (addr->s6_addr32[2] == 0) {
            if (addr->s6_addr32[3] == 0)
                return IPV6_ADDR_ANY;

            if (addr->s6_addr32[3] == htonl(0x00000001))
                return (IPV6_ADDR_LOOPBACK | IPV6_ADDR_UNICAST |
                        IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL));	/* addr-select 3.4 */

            return (IPV6_ADDR_COMPATv4 | IPV6_ADDR_UNICAST |
                    IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.3 */
        }

        if (addr->s6_addr32[2] == htonl(0x0000ffff))
            return (IPV6_ADDR_MAPPED |
                    IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.3 */
    }

    return (IPV6_ADDR_UNICAST |
            IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.4 */
}

static inline int ipv6_addr_scope( const struct in6_addr *addr )
{
	return __ipv6_addr_type(addr) & IPV6_ADDR_SCOPE_MASK;
}

static char *get_iface_name() 
{
    if ( strlen( iface_name ) ) {
        return iface_name;
    }
    return WIRED_NAME;
}

/**
 * @brief  get the link status of wired link
 *
 * @param[out]  status: the wired link status is up or not
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_get_status(TKL_WIRED_STAT_E *status)
{

#ifdef SSD20X 
    FILE *fp = NULL;
    char buf[8] = {0};
    int netlink = 0;
#else 
    int sock = -1;
    struct ifreq ifr;
    int ret = OPRT_COM_ERROR;
#endif

    if ( !status ){
        printf( "%s %d: arg error\n", __func__, __LINE__ );
        return OPRT_COM_ERROR;
    }

#ifdef SSD20X
    if ( ( fp = fopen( "/proc/ty_netlink", "r" ) ) == NULL ) {
        printf( "open /proc/ty_netlink failed\n" );
        return OPRT_COM_ERROR;
    }

    while( fgets( buf, sizeof( buf ), fp ) ) {
        sscanf( buf, "%d", &netlink );

        if ( netlink == 1 ) {
            *status = TKL_WIRED_LINK_UP;
            break;
        } else {
            *status = TKL_WIRED_LINK_DOWN;
            break;
        }
    }

    fclose(fp);
    return OPRT_OK;

#else
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf( "%s %d: socket error=%d,%s\n", __func__, __LINE__, errno, strerror( errno ) );
        return OPRT_SOCK_ERR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);

    if ( ioctl( sock, SIOCGIFFLAGS, &ifr )  < 0 ) {
        printf( "%s %d: failed to get iterface info=%d,%s\n", __func__, __LINE__, errno, strerror( errno ) );
        ret = OPRT_COM_ERROR;
        goto exit_end;

    }

    if ( ifr.ifr_flags & IFF_RUNNING ) {
        *status = TKL_WIRED_LINK_UP;

    }
    else {
        *status = TKL_WIRED_LINK_DOWN;
    }
    ret = OPRT_OK;

exit_end:
    if ( sock ) {
        close( sock );
    }
    return ret;
#endif
}


TKL_WIRED_STATUS_CHANGE_CB status_cb;
static int link_status_exit;

static void *link_status_thread(void *arg)
{
    TKL_WIRED_STAT_E status;
    int old_status = -1;

    /* Circulate detection of network cable plugging and
       unplugging status, and report status when changing
       */
    link_status_exit = 0;

    pthread_detach(pthread_self());

    while(!link_status_exit) {
        tkl_wired_get_status(&status);
        if (status == old_status) {
            sleep(1);
            continue;
        }
        old_status = status;

        status_cb(status);
        sleep(1);
    }

    return NULL;
}

/**
 * @brief  set the status change callback
 *
 * @param[in]   cb: the callback when link status changed
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_set_status_cb(TKL_WIRED_STATUS_CHANGE_CB cb)
{
    pthread_t thread;

    status_cb = cb;

    return pthread_create(&thread, NULL, link_status_thread, NULL);
}

/**
 * @brief  unset the status change callback
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_unset_status_cb(void)
{
    link_status_exit = 1;
    return 0;
}

static OPERATE_RET wired_get_gateway_ipv4(NW_IP_S *ip)
{
    FILE *fp = NULL;
    char *str = NULL;
    char buf[256] = {0};
    unsigned int dest = 0;
    struct in_addr addr;
    int get_gw = 0;

    /* Open file */
    if ((fp = fopen(ROUTE_PATH, "r")) == NULL) {
        printf("open %s failed, %d, %s\n", ROUTE_PATH, errno, strerror( errno ));
        return OPRT_COM_ERROR;
    }

    /* Read one line at a time, perform string processing */
    while(fgets(buf, sizeof(buf), fp)) {
        str = strstr(buf, get_iface_name());
        if (str == NULL)
            continue;

        sscanf(str, "%*s%x %x", &dest, &addr.s_addr);
        if ((dest == 0) && (addr.s_addr != 0)) {
            strncpy(ip->addr.ip4.gw, inet_ntoa(addr), sizeof(ip->addr.ip4.gw));
            get_gw = 1;
            break;
        }
    }

    if (get_gw == 0)
        printf("not find gw addr\n");

    fclose(fp);
    return OPRT_OK;
}

/**
 * @brief  get the ip address of the wired link
 * 
 * @param[in]   ip: the ip address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
static OPERATE_RET wired_get_ipv4(NW_IP_S *ip)
{
    int sock = -1;
    struct ifreq ifr;
    struct sockaddr_in *sin;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf( "%s %d: socket create fail\n", __func__, __LINE__ );
        return OPRT_SOCK_ERR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);
    sin = (struct sockaddr_in *)&(ifr.ifr_addr);

    /* get ip addr */
    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
        //printf( "%s %d: %s ip ioctl error=%d,%s\n", __func__, __LINE__, ifr.ifr_name, errno, strerror( errno ) );
        goto com_error;
    }
    strncpy(ip->addr.ip4.ip, inet_ntoa(sin->sin_addr), sizeof(ip->addr.ip4.ip));

    /* get net mask */
    if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0) {
        //printf( "%s %d: %s mask ioctl error=%d,%s\n", __func__, __LINE__, ifr.ifr_name, errno, strerror( errno ) );
        goto com_error;
    }
    strncpy(ip->addr.ip4.mask, inet_ntoa(sin->sin_addr), sizeof(ip->addr.ip4.mask));

    /* get gateway */
    if (wired_get_gateway_ipv4(ip) < 0)
        goto com_error;

    close(sock);
    return OPRT_OK;

com_error:
    close(sock);
    return OPRT_COM_ERROR;
}

static OPERATE_RET wired_get_ipv6(NW_IP_TYPE type, NW_IP_S *ip) 
{
    int family = 0;
    int scope = 0;
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;
    char *addr = NULL;
    int size = 0;
    int ret = -1;
    char *ifname = get_iface_name();

    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            if (strncmp(ifname, ifa->ifa_name, IFNAMSIZ))
                continue;

            family = ifa->ifa_addr->sa_family;

            switch (family) {
                case AF_INET:
                    continue;
                case AF_INET6:
                    addr = ip->addr.ip6.ip;
                    size = sizeof( ip->addr.ip6.ip );

                    scope = ipv6_addr_scope((const struct in6_addr *)&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr );
                    if ( type == NW_IPV6 && (scope == IPV6_ADDR_UNICAST ||  scope == IPV6_ADDR_ANY ) ) {
                        inet_ntop(family, (char *)ifa->ifa_addr + offsetof(struct sockaddr_in6, sin6_addr), addr, size);
                        printf("%s: %s, scope=0x%04x\n", ifa->ifa_name, addr, scope);
                        ret = 0;
                        break;
                    }
                    else if ( type == NW_IPV6_LL && scope == IPV6_ADDR_LINKLOCAL ) {
                        inet_ntop(family, (char *)ifa->ifa_addr + offsetof(struct sockaddr_in6, sin6_addr), addr, size);
                        printf("%s: %s, scope=0x%04x\n", ifa->ifa_name, addr, scope);
                        ip->addr.ip6.islinklocal = 1;
                        ret = 0;
                        break;
                    }

                default:
                    continue;
            }

        }
        freeifaddrs(ifaddr);
    }
    else {
        return -1;
    }

    return ret;
}


/**
 * @brief  get the ip address of the wired link
 * 
 * @param[in]   ip: the ip address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_get_ip(NW_IP_S *ip)
{   
    OPERATE_RET ret = OPRT_COM_ERROR;

    if (ip == NULL) {
        printf( "%s %d: invalid param\n", __func__, __LINE__ );
        return OPRT_INVALID_PARM;
    }

    if ( TY_AF_INET == ip->type ) {
        ret = wired_get_ipv4( ip );
    }
    else if ( TY_AF_INET6 == ip->type ) {
        ret = wired_get_ipv6( NW_IPV6, ip );
    }
    else {
        printf( "%s %d: invalid param type=%d\n", __func__, __LINE__, ip->type );
    }
    return ret;
}

/**
 * @brief  get the ipv6 address of the wired link
 * 
 * @param[in]   type: the ip type
 * @param[in]   ip: the ip address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_wired_get_ipv6(NW_IP_TYPE type, NW_IP_S *ip)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief  get the mac address of the wired link
 * 
 * @param[in]   mac: the mac address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_get_mac(NW_MAC_S *mac)
{
    int sock = -1;
    struct ifreq ifr;
    struct sockaddr *addr;

    if (mac == NULL) {
        printf( "%s %d: invalid param\n", __func__, __LINE__ );
        return OPRT_INVALID_PARM;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf( "%s %d: socket create fail\n", __func__, __LINE__ );
        return OPRT_SOCK_ERR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);
    addr = (struct sockaddr *)&ifr.ifr_hwaddr;
    addr->sa_family = 1;

    /* get mac addr */
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        printf( "%s %d: get mac, error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
        close(sock);
        return OPRT_COM_ERROR;
    }

    memcpy(mac->mac, addr->sa_data, MAC_ADDR_LEN);

    close(sock);
    return OPRT_OK;
}

/**
 * @brief  set the mac address of the wired link
 * 
 * @param[in]   mac: the mac address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_set_mac(CONST NW_MAC_S *mac)
{
    int sock = -1;
    struct ifreq ifr;
    struct sockaddr *addr;
    int state = 0;

    if (mac == NULL) {
        printf( "%s %d: invalid param\n", __func__, __LINE__ );
        return OPRT_INVALID_PARM;
    }

    printf("%s %d: %s %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__, get_iface_name(), mac->mac[0], mac->mac[1], mac->mac[2], mac->mac[3], mac->mac[4], mac->mac[5] );

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf( "%s %d: create sock error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
        return OPRT_SOCK_ERR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);

    if ( ioctl( sock, SIOCGIFFLAGS, &ifr ) < 0 ) {
        printf( "%s %d: get flag error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
        close(sock);
        return OPRT_COM_ERROR;

    }

    if ( ifr.ifr_ifru.ifru_flags & IFF_UP ) {
        printf( "%s %d: disable interface \n", __func__, __LINE__ );
        state = 1;
        ifr.ifr_ifru.ifru_flags &= ( ~IFF_UP );
        if ( ioctl( sock, SIOCSIFFLAGS, &ifr ) <  0 ) {
            printf( "%s %d: set flag error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
            close(sock);
            return OPRT_COM_ERROR;
        }
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);
    addr = (struct sockaddr *)&ifr.ifr_hwaddr;
    addr->sa_family = 1;
    memcpy(addr->sa_data, mac->mac, MAC_ADDR_LEN);

    /* set mac addr */
    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0) {
        printf( "%s %d: set mac, error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
        close(sock);
        return OPRT_COM_ERROR;
    }

    if ( state ) {
        printf( "%s %d: enable interface \n", __func__, __LINE__ );
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, get_iface_name(), sizeof(ifr.ifr_name) - 1);
        ifr.ifr_ifru.ifru_flags |= IFF_UP;
        if ( ioctl( sock, SIOCSIFFLAGS, &ifr ) <  0 ) {
            printf( "%s %d: get flag error=%d,%s\n", __func__, __LINE__,  errno, strerror( errno ) );
            close(sock);
            return OPRT_COM_ERROR;
        }
    }

    close(sock);
    return OPRT_OK;
}


TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wired_set_iface_name(CONST CHAR_T *name)
{
    int len = 0;
    if ( !name ) {
        printf( "%s %d: invalid param\n", __func__, __LINE__ );
        return OPRT_COM_ERROR;
    }

    len = sizeof( iface_name );
    if ( strlen( name ) > len - 1 ) {
        printf( "%s %d: name is too long, max is %d\n", __func__, __LINE__ , len - 1 );
        return OPRT_COM_ERROR;
    }

    memset( iface_name, 0x0, len );
    snprintf( iface_name, len, "%s", name );
    //printf( "%s %d: set iface name %s \n", __func__, __LINE__, iface_name );
    return OPRT_OK;
}

