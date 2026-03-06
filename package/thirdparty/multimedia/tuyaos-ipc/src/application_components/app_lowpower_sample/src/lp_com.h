#ifndef __LP_COM_H__
#define __LP_COM_H__


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int lp_crypt_hmac_sha256(unsigned char* pbuffer, int dlen,
                         unsigned char* psecret, int slen,
                         unsigned char* pdigest);

int lp_crypt_aes_encrypt(unsigned char* pinput, int len, unsigned char* poutput, int* polen,
                         unsigned char* pkey, unsigned char* piv);

int lp_crypt_aes_decrypt(unsigned char* pinput, int len, unsigned char* poutput, int* polen,
                         unsigned char* pkey, unsigned char* piv);

void lp_system_random_str(unsigned char s[], int num);

uint64_t lp_time_utc_ms();

void lp_cli_print_hex(unsigned char* pdata, int len);

#ifdef __cplusplus
}
#endif

#endif