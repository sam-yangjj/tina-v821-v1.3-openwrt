#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/oid.h"
#include "mbedtls/aes.h"
#include "mbedtls/aesni.h"
#include "mbedtls/x509_crt.h"
#include "tuya_tls.h"
#include "md5.h"
#include "unpack_httpc.h"

#define TLS_SESSION 1
#define MBEDTLS_CIPHER_MODE_CBC

#define DEBUG_LOG
#ifdef DEBUG_LOG
#define dgb_print(format,...) printf (format, ##__VA_ARGS__)
#else
#define dgb_print(format, ...)
#endif

static void* __tuya_tls_calloc(size_t nmemb, size_t size);
static void __tuya_tls_free(void * p);
static  unsigned char s_iot_dns_cert_der[] = {
    0x30, 0x82, 0x03, 0x0e, 0x30, 0x82, 0x01, 0xf6, 0x02, 0x09, 0x00, 0xc5, 0x3e, 0x9d, 0xe4, 0xdc,
    0x37, 0xae, 0x68, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
    0x05, 0x00, 0x30, 0x48, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
    0x53, 0x31, 0x18, 0x30, 0x16, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0f, 0x48, 0x54, 0x54, 0x50,
    0x20, 0x44, 0x4e, 0x53, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x31, 0x1f, 0x30, 0x1d, 0x06,
    0x03, 0x55, 0x04, 0x0b, 0x0c, 0x16, 0x49, 0x4f, 0x54, 0x20, 0x44, 0x4e, 0x53, 0x20, 0x50, 0x75,
    0x62, 0x6c, 0x69, 0x63, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30, 0x20, 0x17, 0x0d,
    0x31, 0x39, 0x30, 0x35, 0x32, 0x35, 0x30, 0x37, 0x35, 0x35, 0x35, 0x37, 0x5a, 0x18, 0x0f, 0x32,
    0x31, 0x31, 0x39, 0x30, 0x35, 0x30, 0x31, 0x30, 0x37, 0x35, 0x35, 0x35, 0x37, 0x5a, 0x30, 0x48,
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x18, 0x30,
    0x16, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0f, 0x48, 0x54, 0x54, 0x50, 0x20, 0x44, 0x4e, 0x53,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x0c, 0x16, 0x49, 0x4f, 0x54, 0x20, 0x44, 0x4e, 0x53, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09,
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
    0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xe4, 0x98, 0xdd, 0x80, 0x21, 0xb0, 0xee,
    0x24, 0x5d, 0xf5, 0xbd, 0x9f, 0x5a, 0x5f, 0x0a, 0xd8, 0x48, 0x6b, 0x16, 0x46, 0x33, 0x71, 0xdb,
    0x5b, 0x41, 0xb8, 0x77, 0x87, 0xc6, 0x69, 0xc4, 0xf8, 0x08, 0x53, 0x1a, 0xc8, 0xce, 0xa4, 0x39,
    0xa2, 0xa6, 0x3b, 0x5e, 0x98, 0xba, 0x09, 0x0a, 0xef, 0x61, 0x7c, 0x81, 0x99, 0x56, 0x6e, 0xe0,
    0xde, 0x43, 0x2e, 0x18, 0xc5, 0x7d, 0x90, 0xd1, 0x57, 0x19, 0x39, 0x43, 0xa7, 0x04, 0x1c, 0x06,
    0x77, 0x5a, 0x10, 0xc2, 0xb7, 0xa8, 0x27, 0xdc, 0xf3, 0x64, 0x0f, 0x32, 0x70, 0x14, 0x38, 0x05,
    0xe9, 0xb7, 0x8d, 0xf7, 0x27, 0x96, 0x3e, 0x39, 0xf4, 0x1e, 0x48, 0xb9, 0x7b, 0xc2, 0x01, 0xb7,
    0x8b, 0xcb, 0x1f, 0xb4, 0x6b, 0xd1, 0x14, 0x5e, 0x23, 0x89, 0xfb, 0x09, 0x0c, 0x28, 0x84, 0x1b,
    0x37, 0x04, 0xcf, 0x89, 0xfa, 0x85, 0x4c, 0xd4, 0xa9, 0x44, 0xaf, 0x83, 0x5d, 0x1b, 0xe4, 0x21,
    0x5c, 0xa0, 0xd5, 0x07, 0xd1, 0x7f, 0x7a, 0x48, 0xd5, 0x6e, 0xe0, 0x26, 0x99, 0x34, 0x78, 0x5f,
    0xfd, 0x14, 0x78, 0xef, 0x84, 0xe8, 0x35, 0xd0, 0x16, 0x56, 0xa4, 0x01, 0x47, 0x4a, 0xf7, 0x43,
    0x71, 0xe9, 0x0c, 0xfe, 0xbd, 0x0a, 0xd2, 0xbe, 0x9c, 0x2a, 0x6e, 0xdf, 0xde, 0x6a, 0x3f, 0xc8,
    0x86, 0x1d, 0x4e, 0xf2, 0x6c, 0x8d, 0x85, 0xa0, 0x10, 0xdf, 0x2d, 0x22, 0x74, 0x33, 0xd4, 0x9a,
    0xf9, 0x52, 0xfb, 0x3d, 0x0d, 0xf6, 0xda, 0xe0, 0xf1, 0x33, 0xcc, 0x8b, 0x56, 0x21, 0x08, 0x99,
    0x96, 0x42, 0x61, 0x8e, 0xda, 0xed, 0xcc, 0x57, 0x6b, 0xc6, 0x7e, 0xa8, 0x96, 0x98, 0xad, 0x26,
    0x21, 0x14, 0x31, 0xac, 0x78, 0x9b, 0x4b, 0x64, 0xa5, 0x0e, 0x12, 0x19, 0x80, 0x45, 0x90, 0x15,
    0xf5, 0xf6, 0xfe, 0x4b, 0x6c, 0x85, 0x47, 0x7a, 0xbb, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01,
    0x01, 0x00, 0xae, 0x50, 0x2e, 0x46, 0x49, 0xd5, 0xaf, 0x80, 0x26, 0xba, 0x80, 0xbe, 0xc3, 0x06,
    0xf2, 0x4e, 0x91, 0x79, 0xa2, 0x89, 0x9d, 0x93, 0xe8, 0x34, 0x80, 0xb7, 0xea, 0x4d, 0x0c, 0x60,
    0x43, 0x11, 0x33, 0x7a, 0x9d, 0x5d, 0x7f, 0x53, 0x82, 0xd4, 0x55, 0x74, 0xde, 0x83, 0xa0, 0x6b,
    0x52, 0xee, 0x15, 0x42, 0x53, 0xd0, 0x77, 0x99, 0x03, 0xb5, 0x14, 0xae, 0xd4, 0x1f, 0xe8, 0x08,
    0xba, 0x61, 0x09, 0x4d, 0xd4, 0x1a, 0x4e, 0x6f, 0xfa, 0x89, 0xa0, 0x9e, 0xcc, 0x16, 0x04, 0x39,
    0x28, 0xb5, 0xf7, 0xc1, 0xb3, 0x8c, 0xce, 0x59, 0x52, 0x62, 0x9c, 0x94, 0x6c, 0xe3, 0x1e, 0xa3,
    0x39, 0x55, 0xa7, 0xe5, 0x66, 0x18, 0xf5, 0x4d, 0xaf, 0xee, 0xfb, 0x64, 0xf1, 0xfb, 0x56, 0x58,
    0x2c, 0x11, 0x65, 0xc2, 0xce, 0x8f, 0xcc, 0xa3, 0xe2, 0x16, 0x80, 0x79, 0x60, 0x86, 0x52, 0xc6,
    0x21, 0xc9, 0x06, 0x86, 0x43, 0xf2, 0x12, 0xaf, 0x8f, 0xb0, 0xfa, 0x4b, 0x00, 0x20, 0x41, 0x3d,
    0xcc, 0x38, 0x5e, 0x6c, 0xbe, 0xa4, 0xa9, 0xf4, 0x3c, 0xa8, 0x20, 0x5b, 0x44, 0x1c, 0x56, 0xd6,
    0x44, 0x02, 0xc0, 0xcf, 0xe1, 0x9d, 0x11, 0x84, 0x0d, 0xc9, 0x34, 0x00, 0x99, 0xe8, 0xe0, 0xdd,
    0x09, 0xff, 0xf8, 0x7a, 0xa1, 0x46, 0x91, 0x14, 0xc6, 0xb2, 0xbb, 0x31, 0xf9, 0x6a, 0x6d, 0xa9,
    0x1f, 0x81, 0x74, 0xbc, 0x39, 0x4f, 0x58, 0x03, 0x72, 0x39, 0x0b, 0xe2, 0x2c, 0x93, 0x9b, 0xce,
    0xcf, 0x92, 0x19, 0xb0, 0x82, 0x3e, 0x30, 0x90, 0xd3, 0xee, 0x70, 0xea, 0xcb, 0xf0, 0x8e, 0x90,
    0x99, 0x28, 0x26, 0x5b, 0xe0, 0x9e, 0xfc, 0xac, 0xca, 0xa5, 0x65, 0xb7, 0xf4, 0x5a, 0x4b, 0x46,
    0x46, 0xc1, 0x36, 0xb1, 0x98, 0xca, 0x1c, 0xf2, 0x73, 0x88, 0xcc, 0x95, 0x23, 0xf4, 0xd7, 0x94,
    0xa2, 0xb5
};

typedef struct {
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config conf_ctx;
    mbedtls_x509_crt cert_ctx;
    long socket_fd;
    long overtime_s;
    pthread_mutex_t mutex; //在tuya业务中，读取不会多线程，但是写入会多线程。
}TUYA_TLS_CTX_S;

typedef struct
{
    unsigned char *p_der;
    unsigned long  der_len;
}TY_TLS_DER_S;

#define MAX_TLS_SESSION_NUM  8
#define TLS_URL_LEN          256
#define DEFAULT_LOAD_CERT TRUE
typedef struct
{
    mbedtls_ssl_session session;
    char hostname[TLS_URL_LEN];
    int used;
}tuya_tls_session_mgr;

typedef struct
{
    TY_TLS_DER_S *p_der_arr;
    unsigned long der_arr_size;
    tuya_tls_event_cb event_cb;
    char *p_uuid;
    char *p_authkey;
    long verify_mode;
    int load_cert;
    tuya_tls_session_mgr tls_session[MAX_TLS_SESSION_NUM];
    pthread_mutex_t tls_session_lock;
}TUYA_TLS_GLOBAL_MGR_S;

static TUYA_TLS_GLOBAL_MGR_S s_global_mgr = {0};

void tuya_tls_register_constant( char *p_uuid,  char *p_authkey)
{
    printf("tls register constant %s %s\n", p_uuid, p_authkey);
    s_global_mgr.p_uuid = p_uuid;
    s_global_mgr.p_authkey = p_authkey;
}

void tuya_tls_set_ssl_verify(  long verify_mode)
{
    s_global_mgr.verify_mode = verify_mode;
}

long tuya_tls_register_x509_crt_der(void *p_ctx, unsigned char *p_der, unsigned long der_len)
{
    mbedtls_x509_crt *p_cert_ctx = (mbedtls_x509_crt *)p_ctx;
    return mbedtls_x509_crt_parse( p_cert_ctx, (const unsigned char *) p_der, der_len );
}

static void __tuya_tls_log( void *ctx, int level, const char *file, int line, const char *str )
{
    switch(level)
    {
        case 1:
        {
            printf("%s %d:%s\n", "", line, str);
            break;
        }
        case 2:
        {
            printf("%s %d:%s\n", "", line, str);
            break;
        }
        case 3:
        default:
        {
            printf("%s %d:%s\n", "", line, str);
            break;
        }
    }
}

static void* __tuya_tls_calloc(size_t nmemb, size_t size)
{
    size_t mem_size = nmemb * size;
    void *ptr = malloc(mem_size);
    if(ptr != NULL)
    {
        memset(ptr, 0, mem_size);
    }
    return ptr;
}

static void __tuya_tls_free(void * p)
{
    free(p);
}

int tuya_hal_get_random_data(unsigned char* dst, int size, unsigned char range)
{
    if(range == 0)
        range = 0xFF;

    int index = 0;
    for(index = 0; index < size; index ++)
        dst[index] = rand() % range;

    return OPRT_OK;
}

static int __tuya_tls_random(void *p_rng, unsigned char *output, size_t output_len)
{
    tuya_hal_get_random_data(output, output_len, 0);
    return 0;
}

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = (unsigned char*)v; while( n-- ) *p++ = 0;
}

// TODO:
#if 0
void mbedtls_aes_init( mbedtls_aes_context *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_aes_context ) );
}

void mbedtls_aes_free( mbedtls_aes_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_zeroize( ctx, sizeof( mbedtls_aes_context ) );
}
#endif

static int __tuya_tls_socket_send_cb( void *ctx, const unsigned char *buf, size_t len )
{
    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)ctx;
    return unw_send(p_tls_mgr->socket_fd, buf, len);
}

int unw_get_nonblock(const int fd)
{
    if( fd < 0 ) {
        return -1;
    }

    if((fcntl(fd, F_GETFL, 0) & O_NONBLOCK) != O_NONBLOCK) {
        return 0;
    }

    if(errno == EAGAIN || errno == EWOULDBLOCK) {
        return 1;
    }

    return 0 ;
}

static int __tuya_tls_socket_recv_cb( void *ctx, unsigned char *buf, size_t len )
{
    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)ctx;

    long non_block = unw_get_nonblock(p_tls_mgr->socket_fd);
    if(non_block == 0) {
        unw_set_block(p_tls_mgr->socket_fd, FALSE);
    }

    fd_set readfds;
    memset(&readfds, 0, sizeof(fd_set));
    FD_SET(p_tls_mgr->socket_fd, &readfds);
    long activefds_cnt = unw_select(p_tls_mgr->socket_fd + 1, &readfds, NULL, NULL, p_tls_mgr->overtime_s * 1000);
    if(activefds_cnt <= 0)
    {
        unw_set_block(p_tls_mgr->socket_fd, 1 - non_block);
        printf("select fail.%ld\n", activefds_cnt);
        return -100+ activefds_cnt;
    }

    long rv = unw_recv(p_tls_mgr->socket_fd, buf, len);
    unw_set_block(p_tls_mgr->socket_fd, 1 - non_block);

    return rv;
}


static int tuya_tls_ciphersuite_list_ECC_PSK[] = {
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
    0
};
static int tuya_tls_ciphersuite_list_PSK_ONLY[] = {
    MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256,
    0
};

void tuya_tls_set_load_cert(int load)
{
    printf("set cert loaded:%d\n", load);
    s_global_mgr.load_cert = load;
}

void tuya_tls_save_session(const mbedtls_ssl_context *p_ssl_ctx, const char *hostname)
{
    long i;
    long unused_index = -1;
    pthread_mutex_lock(&s_global_mgr.tls_session_lock);
    for (i = 0; i < MAX_TLS_SESSION_NUM; i++)
    {
        //相同域名，更新session
        if(s_global_mgr.tls_session[i].used && (0 == strncmp(hostname,s_global_mgr.tls_session[i].hostname,TLS_URL_LEN)))
        {
            mbedtls_ssl_get_session(p_ssl_ctx,&s_global_mgr.tls_session[i].session);
            pthread_mutex_unlock(&s_global_mgr.tls_session_lock);
            return;
        }
        if(!s_global_mgr.tls_session[i].used)
        {
            unused_index = i;
        }
    }
    //新域名，存储session
    if(unused_index >= 0)
    {
        s_global_mgr.tls_session[unused_index].used = TRUE;
        mbedtls_ssl_get_session(p_ssl_ctx,&s_global_mgr.tls_session[unused_index].session);
        memcpy(s_global_mgr.tls_session[unused_index].hostname, hostname, TLS_URL_LEN);
    }
    pthread_mutex_unlock(&s_global_mgr.tls_session_lock);
}

void tuya_tls_set_session(mbedtls_ssl_context *p_ssl_ctx, const char *hostname)
{
    long i;
    pthread_mutex_lock(&s_global_mgr.tls_session_lock);
    for (i = 0; i < MAX_TLS_SESSION_NUM; i++)
    {
        if(s_global_mgr.tls_session[i].used && (0 == strncmp(hostname,s_global_mgr.tls_session[i].hostname,TLS_URL_LEN)))
        {
            mbedtls_ssl_set_session(p_ssl_ctx,&s_global_mgr.tls_session[i].session);
        }
    }
    pthread_mutex_unlock(&s_global_mgr.tls_session_lock);
}

void tuya_tls_clear_session(const char *hostname)
{
    long i;
    pthread_mutex_lock(&s_global_mgr.tls_session_lock);
    for (i = 0; i < MAX_TLS_SESSION_NUM; i++)
    {
        if(s_global_mgr.tls_session[i].used && (0 == strncmp(hostname,s_global_mgr.tls_session[i].hostname,TLS_URL_LEN)))
        {
            s_global_mgr.tls_session[i].used = FALSE;
        }
    }
    pthread_mutex_unlock(&s_global_mgr.tls_session_lock);
}

int tuya_tls_init( tuya_tls_event_cb event_cb)
{
    s_global_mgr.event_cb = event_cb;
    s_global_mgr.verify_mode = MBEDTLS_SSL_VERIFY_REQUIRED;
    s_global_mgr.load_cert = DEFAULT_LOAD_CERT;

    pthread_mutex_init(&s_global_mgr.tls_session_lock,NULL);
    return OPRT_OK;
}

int tuya_tls_connect( tuya_tls_hander *p_tls_handler,  char *hostname, long port_num,  long sethostname,
                              void *p_custom_net_ctx,  tuya_tls_send_cb send_cb,  tuya_tls_recv_cb recv_cb,
                              long socket_fd,  long overtime_s)
{
    int ret = 0;
    s_global_mgr.load_cert = DEFAULT_LOAD_CERT;
    int enable_cert = s_global_mgr.load_cert;

    if(NULL == p_tls_handler)
    {
        printf("NULL Input\n");
        return OPRT_VALID_PARM;
    }

    if(p_custom_net_ctx == NULL)
    {
        printf("custom net ctx must be set\n");
        return OPRT_COM_ERROR;
    }

    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)__tuya_tls_calloc(1, sizeof(TUYA_TLS_CTX_S));
    if(p_tls_mgr == NULL)
    {
        printf("malloc fails\n");
        return OPRT_MALLOC_FAILED;
    }
    pthread_mutex_init(&p_tls_mgr->mutex,NULL);
    *p_tls_handler = p_tls_mgr;
    dgb_print("TUYA_TLS Begin Connect %s:%d  CustomNet:%s Enable_Cert:%s Default:%s \n",
             (hostname ? hostname : ""), port_num, (p_custom_net_ctx != NULL ? "TRUE":"FALSE"),
             (enable_cert ? "TRUE":"FALSE"),  (DEFAULT_LOAD_CERT ? "TRUE":"FALSE") );

    mbedtls_ssl_context *p_ssl_ctx = &(p_tls_mgr->ssl_ctx);
    mbedtls_ssl_config *p_conf_ctx = &(p_tls_mgr->conf_ctx);

    mbedtls_ssl_init( p_ssl_ctx );
    mbedtls_ssl_config_init( p_conf_ctx );
    mbedtls_ssl_conf_dbg(p_conf_ctx, __tuya_tls_log, NULL );
    mbedtls_ssl_conf_rng(p_conf_ctx, __tuya_tls_random, NULL);

    ret = mbedtls_ssl_config_defaults( p_conf_ctx, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT );
    if(ret != 0)
    {
        printf("mbedtls_ssl_config_defaults Fail. %x %d\n", ret, ret);
        return ret;
    }
    mbedtls_ssl_conf_authmode( p_conf_ctx, MBEDTLS_SSL_VERIFY_NONE);

    if(enable_cert == TRUE) {
        dgb_print("mbedtls_x509_crt_init  start\n");
        mbedtls_x509_crt *p_cert_ctx = &(p_tls_mgr->cert_ctx);
        mbedtls_x509_crt_init( p_cert_ctx );

        ret = mbedtls_x509_crt_parse( p_cert_ctx, s_iot_dns_cert_der, sizeof(s_iot_dns_cert_der) );
        if( ret != 0 )
        {
            printf("iot_dns cert parse fail. ret: %x\n", ret);
            return ret;
        }
        else
            dgb_print("iot_dns  ok\n");


        mbedtls_ssl_conf_ca_chain( p_conf_ctx, p_cert_ctx, NULL );

        if(sethostname) {
            ret = mbedtls_ssl_set_hostname( p_ssl_ctx, hostname );
            if(ret != 0 )
            {
                printf("mbedtls_ssl_set_hostname Fail. 0x%x\n", ret);
                return ret;
            }
            else
                dgb_print("mbedtls_ssl_set_hostname ok\n");
        }

        mbedtls_ssl_conf_ciphersuites(p_conf_ctx, tuya_tls_ciphersuite_list_ECC_PSK);
    }
    //mbedtls_ssl_conf_max_frag_len(p_conf_ctx, MBEDTLS_SSL_MAX_FRAG_LEN_1024);
    ret = mbedtls_ssl_setup( p_ssl_ctx, p_conf_ctx );
    if(ret != 0)
    {
        printf("mbedtls_ssl_setup Fail. 0x%x\n", ret);
        return ret;
    }

    if(socket_fd >= 0)
    {
        p_tls_mgr->socket_fd = socket_fd;
        p_tls_mgr->overtime_s = overtime_s;
        unw_set_timeout(p_tls_mgr->socket_fd, overtime_s * 1000, TRANS_SEND);
        mbedtls_ssl_set_bio(p_ssl_ctx, p_tls_mgr, __tuya_tls_socket_send_cb, __tuya_tls_socket_recv_cb, NULL);
        dgb_print("socket fd is set. set to inner send/recv to handshake \n");
    }else
    {/* 业务上保证socket_fd始终有效 */
        dgb_print(" TUYA SDK,should not enter here!!!!! \n");
        mbedtls_ssl_set_bio(p_ssl_ctx, p_custom_net_ctx, send_cb, recv_cb, NULL);
    }

    //tuya_tls_set_session(p_ssl_ctx,hostname);

    while( ( ret = mbedtls_ssl_handshake(p_ssl_ctx) ) != 0 )
    {
        unsigned long handshake_flags = mbedtls_ssl_get_verify_result( p_ssl_ctx );
        dgb_print("tls handshake ret:%d %d \n", ret, handshake_flags);
        if(ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
        {
            printf("tls handshake fail:%d .require new certs.\n", ret);
            if(s_global_mgr.event_cb != NULL)
                s_global_mgr.event_cb(TY_TLS_CERT_EXPIRED, NULL);
        }


        if( ret != MBEDTLS_ERR_SSL_WANT_READ  && \
            ret != MBEDTLS_ERR_SSL_WANT_WRITE \
            && ret != (MBEDTLS_ERR_X509_UNKNOWN_SIG_ALG + MBEDTLS_ERR_OID_NOT_FOUND))
        {
            //tuya_tls_clear_session(hostname);

            dgb_print( "mbedtls_ssl_handshake returned %x %d \n", ret, ret);
            return ret;
        }
    }
    //tuya_tls_save_session(p_ssl_ctx,hostname);
    dgb_print("handshake finish for %s. set send/recv to user set\n",hostname);

    mbedtls_ssl_set_bio(p_ssl_ctx, p_custom_net_ctx, send_cb, recv_cb, NULL);
    unsigned long flags = 0;
    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( p_ssl_ctx ) ) != 0 )
    {
        dgb_print("mbedtls_ssl_get_verify_result flag %x\n", flags);
    }

    dgb_print("TUYA_TLS Success Connect %s:%d  CustomNet:%s Suit:%s\n", (hostname ? hostname : ""),
            port_num, (p_custom_net_ctx != NULL ? "TRUE":"FALSE"),
            mbedtls_ssl_get_ciphersuite(p_ssl_ctx));

    return OPRT_OK;
}

long tuya_tls_write( tuya_tls_hander tls_handler, unsigned char *buf, unsigned long len)
{
    int ret = -1;
    int mu_ret = OPRT_OK;
    size_t written_len = 0;
    if((tls_handler == NULL) || (buf == NULL) || (len == 0))
    {
        printf("Input Invalid\n");
        return OPRT_VALID_PARM;
    }
    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)tls_handler;
    mbedtls_ssl_context *p_ssl_ctx = &(p_tls_mgr->ssl_ctx);

    mu_ret = pthread_mutex_lock(&p_tls_mgr->mutex);
    if(OPRT_OK != mu_ret) {
        printf("MutexLock err %d\n",mu_ret);
        return mu_ret;
    }
    while (written_len < len)
    {
        ret = mbedtls_ssl_write(p_ssl_ctx, (buf + written_len), (len - written_len));
        if (ret > 0)
        {
            written_len += ret;
            continue;
        }

        if( (ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE) )
        {
            continue;
        }

        dgb_print( "mbedtls_ssl_write returned %d\n", ret);
        mu_ret = pthread_mutex_unlock(&p_tls_mgr->mutex);
        if(OPRT_OK != mu_ret) {
            printf("MutexUnLock err %d\n",mu_ret);
            return mu_ret;
        }
        return ret;
    }

    mu_ret = pthread_mutex_unlock(&p_tls_mgr->mutex);
    if(OPRT_OK != mu_ret) {
        printf("MutexUnLock err %d\n",mu_ret);
        return mu_ret;
    }
    return written_len;
}

long tuya_tls_read( tuya_tls_hander tls_handler, unsigned char *buf, unsigned long len)
{
    if((tls_handler == NULL) || (buf == NULL) || (len == 0))
    {
        printf("Input Invalid\n");
        return OPRT_VALID_PARM;
    }
    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)tls_handler;
    mbedtls_ssl_context *p_ssl_ctx = &(p_tls_mgr->ssl_ctx);

    pthread_mutex_lock(&p_tls_mgr->mutex);
    long value = mbedtls_ssl_read(p_ssl_ctx, buf, len);
    pthread_mutex_unlock(&p_tls_mgr->mutex);

    return value;
}

int tuya_tls_disconnect( tuya_tls_hander tls_handler)
{
    if(tls_handler == NULL)
    {
        printf("Input Invalid\n");
        return OPRT_VALID_PARM;
    }

    TUYA_TLS_CTX_S *p_tls_mgr = (TUYA_TLS_CTX_S *)tls_handler;
    mbedtls_ssl_context *p_ssl_ctx = &(p_tls_mgr->ssl_ctx);
    mbedtls_ssl_config *p_conf_ctx = &(p_tls_mgr->conf_ctx);
    // TODO:
    // mbedtls_x509_crt_free( p_conf_ctx->ca_chain );
    mbedtls_ssl_free( p_ssl_ctx );
    mbedtls_ssl_config_free( p_conf_ctx );

    pthread_mutex_destroy(&p_tls_mgr->mutex);

    __tuya_tls_free(p_tls_mgr);

    dgb_print("TUYA_TLS Disconnect Success\n");
    return OPRT_OK;
}
