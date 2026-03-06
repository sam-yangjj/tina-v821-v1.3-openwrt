#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "lp_com.h"

#include "lp_protocol.h"

int lp_protocol_setup_header(lp_protocol_hdr_s* phdr, unsigned char version, unsigned char type,
                               unsigned char flag, unsigned short size)
{
    if(phdr == NULL) {
        return -1;
    }

    phdr->version = version;
    phdr->type    = type;
    phdr->flag    = flag;
    phdr->size    = htons(size);

    return 0;
}

int lp_protocol_parse_header(unsigned char* pdata, int len, lp_protocol_hdr_s* phdr)
{
    if(pdata == NULL || len <= 0 || phdr == NULL) {
        return -1;
    }

    int hdr_len = sizeof(lp_protocol_hdr_s);

    if(len < hdr_len) {
        return -1;
    }

    memcpy(phdr, pdata, hdr_len);

    phdr->size = ntohs(phdr->size);

    return 0;
}

int lp_protocol_heartbeat(char* pdata, int* plen)
{
    lp_protocol_hdr_s hdr;
    int hdr_len = sizeof(lp_protocol_hdr_s);

    if(pdata == NULL || plen == NULL
        || *plen < hdr_len) {
        return -1;
    }

    memset(&hdr, 0, hdr_len);

    lp_protocol_setup_header(&hdr, LP_VERSION_1, LP_TYPE_HEARTBEAT, 0, 0);

    memcpy(pdata, &hdr, hdr_len);
    *plen = hdr_len;

    return 0;
}

int lp_protocol_wakeup(char* pdata, int* plen, unsigned int wk)
{

    lp_protocol_hdr_s hdr;
    int hdr_len = sizeof(lp_protocol_hdr_s);

    if(pdata == NULL || plen == NULL
        || *plen < hdr_len) {
        return -1;
    }

    memset(&hdr, 0, hdr_len);

    lp_protocol_setup_header(&hdr, LP_VERSION_1, LP_TYPE_WAKEUP, 0, 4);

    memcpy(pdata, &hdr, hdr_len);

    pdata[hdr_len]      = (wk >> 24) & 0xFF;
    pdata[hdr_len + 1]  = (wk >> 16) & 0xFF;
    pdata[hdr_len + 2]  = (wk >> 8) & 0xFF;
    pdata[hdr_len + 3]  = (wk) & 0xFF;

    *plen = hdr_len + 4;

    return 0;
}

int lp_protocol_set_auth(unsigned char* pdata, int* plen, lp_protocol_hdr_s* phdr,
                            lp_auth_dev_msg_s* pdevMsg, char* reqdata, int reqlen)
{
    int hdr_len = sizeof(lp_protocol_hdr_s);
    unsigned char iv[16 + 1] = {0};
    unsigned char output[256] = {0};
    int olen = sizeof(output);

    lp_system_random_str(iv, 16);

    lp_crypt_aes_encrypt((unsigned char*)reqdata, reqlen, output, &olen,
                         pdevMsg->key, iv);

    char* tmpd = (char*)malloc(pdevMsg->idLen + 16 + olen + 6);
    if(tmpd == NULL) {
        return -1;
    }

    *(short*)tmpd = htons(16);
    memcpy((unsigned char*)tmpd + 2, iv, 16);

    *(short*)(tmpd + 2 + 16) = htons(pdevMsg->idLen);
    memcpy(tmpd + 2 + 16 + 2, pdevMsg->devId, pdevMsg->idLen);

    *(short*)(tmpd + 2 + 16 + 2 + pdevMsg->idLen) = htons(olen);
    memcpy(tmpd + 2 + 16 + 2 + pdevMsg->idLen + 2, output, olen);

    *plen = hdr_len + pdevMsg->idLen + 16 + olen + 6;
    lp_protocol_setup_header(phdr, LP_VERSION_1, LP_TYPE_AUTH_REQUEST, 1, *plen - hdr_len);
    memcpy(pdata, phdr, hdr_len);
    memcpy(pdata + hdr_len, tmpd, pdevMsg->idLen + 16 + olen + 6);

    free(tmpd);
    tmpd = NULL;

    return 0;
}

int lp_protocol_parse_auth(unsigned char* pdata, int len, lp_protocol_hdr_s* phdr,
                            lp_auth_dev_msg_s* pdevMsg, char* reqdata, int* reqlen)
{
    int hdr_len = sizeof(lp_protocol_hdr_s);
    char devId[64] = {0};
    int devlen = 0;
    int enclen = 0;
    unsigned char iv[16] = {0};

    lp_protocol_parse_header(pdata, hdr_len, phdr);

    short ivlen = ntohs(*(short*)(pdata + hdr_len));

    memcpy(iv, pdata + hdr_len + 2, ivlen);

    devlen = ntohs(*(short*)(pdata + hdr_len + 2 + ivlen));
    memcpy(devId, pdata + hdr_len + 2 + ivlen + 2, devlen);

    enclen = ntohs(*(short*)(pdata + hdr_len + 2 + ivlen + 2 + devlen));

    char* encd = (char*)malloc(enclen + 1);
    if(encd == NULL) {
        return -1;
    }

    memset(encd, 0, enclen + 1);
    memcpy(encd, pdata + hdr_len + 2 + ivlen + 2 + devlen + 2, enclen);

    lp_crypt_aes_decrypt((unsigned char*)encd, enclen, (unsigned char*)reqdata, reqlen,
                         pdevMsg->key, iv);

    free(encd);
    encd = NULL;

    return 0;
}

