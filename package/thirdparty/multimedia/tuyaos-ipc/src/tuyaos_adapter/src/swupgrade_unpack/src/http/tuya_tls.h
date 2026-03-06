#ifndef TUYA_TLS_H
#define TUYA_TLS_H

/* mbedtls 仅仅用于通道加密通讯，不再用于通道建立。*/
//#include "httpc.h"
#include <pthread.h>
#include <stdlib.h>

typedef void * tuya_tls_hander;

typedef int (*tuya_tls_send_cb)( void *p_custom_net_ctx, const unsigned char *buf, size_t len);
typedef int (*tuya_tls_recv_cb)( void *p_custom_net_ctx, unsigned char *buf, size_t len);

//void tuya_tls_load_der_info(void);

void tuya_tls_register_constant( char *p_uuid,  char *p_authkey);

/*
VERIFY_NONE       0
VERIFY_OPTIONAL   1
VERIFY_REQUIRED   2
*/
void tuya_tls_set_ssl_verify(  long verify_mode);
long tuya_tls_register_x509_crt_der(void *p_ctx, unsigned char *p_der, unsigned long der_len);

void tuya_tls_set_load_cert(int load);


typedef enum
{
    TY_TLS_CERT_EXPIRED,
}TY_TLS_EVENT_E;
typedef void (*tuya_tls_event_cb)(TY_TLS_EVENT_E event, void *p_args);
int tuya_tls_init( tuya_tls_event_cb event_cb);

int tuya_tls_connect( tuya_tls_hander *p_tls_handler,  char *hostname, long port_num,  long sethostname,
                              void *p_custom_net_ctx,  tuya_tls_send_cb send_cb,  tuya_tls_recv_cb recv_cb,
                              long socket_fd,  long overtime_s);
long tuya_tls_write( tuya_tls_hander tls_handler, unsigned char *buf, unsigned long len);
long tuya_tls_read( tuya_tls_hander tls_handler, unsigned char *buf, unsigned long len);
int tuya_tls_disconnect( tuya_tls_hander tls_handler);

#endif//TUYA_TLS_H


