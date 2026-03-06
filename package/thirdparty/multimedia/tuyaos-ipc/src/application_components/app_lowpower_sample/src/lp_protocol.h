#ifndef __LP_PROTOCOL_H__
#define __LP_PROTOCOL_H__

#include "lp_auth.h"


#ifdef __cplusplus
extern "C" {
#endif

#define LP_VERSION_1    1
#define LP_FLAG         1


typedef enum LP_TYPE_ {
    LP_TYPE_AUTH_REQUEST	= 0,
    LP_TYPE_AUTH_RESPONSE	= 1,
    LP_TYPE_HEARTBEAT	    = 2,
    LP_TYPE_WAKEUP          = 3,
} LP_TYPE_E;

#pragma pack(1)

typedef struct lp_protocol_hdr_ {
    unsigned char   version;
    unsigned char   type;
    unsigned char   flag;
    unsigned short  size;
} lp_protocol_hdr_s;

#pragma pack()

int lp_protocol_setup_header(lp_protocol_hdr_s* phdr, unsigned char version, unsigned char type,
                               unsigned char flag, unsigned short size);


int lp_protocol_parse_header(unsigned char* pdata, int len, lp_protocol_hdr_s* phdr);


int lp_protocol_heartbeat(char* pdata, int* plen);


int lp_protocol_wakeup(char* pdata, int* plen, unsigned int wk);


int lp_protocol_set_auth(unsigned char* pdata, int* plen, lp_protocol_hdr_s* phdr,
                            lp_auth_dev_msg_s* pdevMsg, char* reqdata, int reqlen);


int lp_protocol_parse_auth(unsigned char* pdata, int len, lp_protocol_hdr_s* phdr,
                            lp_auth_dev_msg_s* pdevMsg, char* reqdata, int* reqlen);

#ifdef __cplusplus
}
#endif

#endif
