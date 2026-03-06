#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "mbedtls/base64.h"

#include "lp_com.h"
#include "lp_protocol.h"
#include "lp_auth.h"


static lp_auth_dev_msg_s g_devMsg;

int lp_cli_connect_server(TUYA_IP_ADDR_T hostaddr, int port)
{
    int sock = -1;
    int flags = 0;
    int ret = 0;
    struct timeval tout;
    struct sockaddr_in addr;
    int domain = hostaddr.type == TY_AF_INET ? AF_INET : AF_INET6;
    sock = socket(domain, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("sock error\n");
        return -1;
    }

    if (hostaddr.type == TY_AF_INET) {
        struct sockaddr_in addr4;
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family        = AF_INET;
        addr4.sin_addr.s_addr   = htonl(hostaddr.u_addr.ip4);
        addr4.sin_port          = htons(port);
        ret = connect(sock, (struct sockaddr*)&addr4, sizeof(addr4));
    } else {
        struct sockaddr_in6 addr6;
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family       = AF_INET6;
        memcpy(&addr6.sin6_addr, hostaddr.u_addr.ip6, sizeof(hostaddr.u_addr.ip6));
        addr6.sin6_port         = htons(port);
        ret = connect(sock, (struct sockaddr*)&addr6, sizeof(addr6));
    }

    if(ret < 0) {
        printf("connect error!=%s\n",strerror(errno));
        close(sock);
        sock = -1;
        return -1;
    }

    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    tout.tv_sec     = 8;
    tout.tv_usec    = 0;

    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tout, sizeof(tout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tout, sizeof(tout));

    return sock;
}

int lp_cli_auth_request(int sock, lp_auth_dev_msg_s* pdevMsg,
                        lp_auth_info_s* preqauth)
{
    unsigned char data[512] = {0};
    int len = sizeof(data);
    char reqd[512] = {0};
    int reql = sizeof(reqd);
    lp_protocol_hdr_s hdr;

    memset(&hdr, 0, sizeof(lp_protocol_hdr_s));
    memset(preqauth, 0, sizeof(lp_auth_info_s));

    snprintf(preqauth->time, sizeof(preqauth->time), "%lld", lp_time_utc_ms());
    lp_system_random_str((unsigned char*)preqauth->random, LP_AUTH_RANDOM_MAX_LEN);

    lp_auth_request(pdevMsg, 1, 1, preqauth, reqd, &reql);
    lp_protocol_set_auth(data, &len, &hdr, pdevMsg, reqd, reql);

    printf("auth data is\n { ",data);
    int i =0;
    for(i=0;i<len;i++)
    {
        printf("0x%x ",data[i]);
    }
    printf("}\n",data);

    ssize_t sz = send(sock, data, len,0);
    if(sz != len) {
        printf("sz: %ld, errno: %d\n", sz, errno);
        return -1;
    }

    return 0;
}
#define MAXBUF 1024
signed int lp_cli_recv(int sock, void *buf, unsigned int size)
{
    int count = 0;
    signed int ret = 0;
    unsigned int rd_size = 0;

    while(rd_size < size) {
        ret = recv(sock, buf + rd_size, size - rd_size, 0);
        if(ret <= 0) {
            if(EWOULDBLOCK == errno || EINTR == errno || EAGAIN == errno) {
                //printf("recv is error =%d\n",errno);
                usleep(500 * 1000);
                if(count++ > 10) {
                    break;
                }

                continue;
            }

            break;
        }

        rd_size += ret;
    }
    return rd_size;
}

int lp_cli_auth_response(int sock, lp_auth_dev_msg_s* pdevMsg, lp_auth_response_s* prsp)
{
    char message[1024] = {0};
    lp_protocol_hdr_s hdr;
    char reqdata[256] = {0};
    int reqlen = sizeof(reqdata);

    memset(&hdr, 0, sizeof(lp_protocol_hdr_s));
    memset(prsp, 0, sizeof(lp_auth_response_s));

    int rlen = lp_cli_recv(sock, message, sizeof(message) - 1);
    if(rlen <= 0) {
        printf("recv auth response error, errno: %s\n", strerror(errno));
        return -1;
    }

    lp_protocol_parse_auth((unsigned char*)message, rlen, &hdr,
                            pdevMsg, reqdata, &reqlen);

    if(hdr.version != LP_VERSION_1) {
        printf("version not match!\n");
        return -1;
    }

    if(lp_auth_parse_response(reqdata, strlen(reqdata), prsp) != 0) {
        printf("parse response error\n");
        return -1;
    }

    return 0;
}

int lp_cli_init_dev_info(lp_auth_dev_msg_s* pmsg, char* pdevId, int idLen,
                        char* pkey, int keyLen)
{
    unsigned char aesdata[128] = {0};
    int len = sizeof(aesdata);
	unsigned char key[16] = {0x23, 0xac, 0x7b, 0x15, 0x0d, 0x89, 0x34, 0x92,
           					 0xf1, 0x19, 0x33, 0xde, 0xc8, 0x6a, 0x10, 0x55};
	unsigned char iv[16] = {0x1e, 0x25, 0x77, 0xb8, 0x66, 0xc1, 0x10, 0x33,
          					0x93, 0x69, 0xcb, 0xa8, 0x2c, 0x54, 0xe5, 0xab};

	memset(pmsg, 0, sizeof(lp_auth_dev_msg_s));

	memcpy(pmsg->key, pkey, keyLen);

    pmsg->keyLen = keyLen;
	pmsg->idLen  = sizeof(pmsg->devId);
    printf("origin devid is =%s \n",pdevId);
	lp_crypt_aes_encrypt(pdevId, idLen, aesdata, &len, key, iv);
	int i=0;
    printf("ecny devid is = ");
    for(i =0; i<len;i++)
    {
        printf("%02x",aesdata[i]);
    }
    printf("\n");
	if(mbedtls_base64_encode(pmsg->devId, pmsg->idLen,
						 (size_t*)&pmsg->idLen, aesdata, len) != 0) {
        printf("base64 encode error\n");
        return -1;
    }

	printf("%s\n",pmsg->devId);

	return 0;
}

int lp_cli_auth(int sock, lp_auth_dev_msg_s* pdevMsg)
{
    int ret = -1;
    lp_auth_info_s reqauth;
    lp_auth_response_s rsp;

    printf("begin___\n");

    memset(&reqauth, 0, sizeof(lp_auth_info_s));
    memset(&rsp, 0, sizeof(lp_auth_response_s));

    do {
        if(lp_cli_auth_request(sock, pdevMsg, &reqauth) < 0) {
            break;
        }

        if(lp_cli_auth_response(sock, pdevMsg, &rsp) < 0) {
            break;
        }

        if(lp_auth_check_response(pdevMsg, &reqauth, &rsp)) {
            ret = 0;
        }
    } while(0);

    printf("end___, auth status: %d\n", ret);

    return ret;
}

int lp_cli_get_sock()
{
    return g_devMsg.sock;
}

int lp_cli_wakeup(char* pdata, unsigned int* plen, int wk)
{
    return lp_protocol_wakeup(pdata, plen, wk);
}

int lp_cli_heartbeat(char* pdata, unsigned int* plen)
{
    return lp_protocol_heartbeat(pdata, plen);
}
int lp_cli_wake_up_data_seed_computer(char *key,unsigned int len)
{
    if(key==NULL)
    {
        return -1;
    }
    return hash_crc32i_total(key,len);
}



