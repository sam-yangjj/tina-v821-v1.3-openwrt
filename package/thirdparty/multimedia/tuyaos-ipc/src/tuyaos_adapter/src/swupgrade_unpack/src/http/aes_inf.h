#ifndef __AES_INF_H__
#define __AES_INF_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
typedef unsigned char BYTE_T;
typedef BYTE_T *PBYTE_T;
typedef char CHAR_T;
typedef char *PCHAR_T;
typedef unsigned int UINT_T;
typedef unsigned int *PUINT_T;
typedef signed int INT_T;
typedef int *PINT_T;
typedef int BOOL_T;
typedef BOOL_T *PBOOL_T;
**/

int ty_init_kv_encrypted(char *key);

int ty_aes_get_actual_length(const unsigned char *dec_data, const unsigned int dec_data_len);

int ty_aes128_ecb_encode( const unsigned char *data, const unsigned int len,\
                               unsigned char **ec_data, unsigned int *ec_len,\
                               const unsigned char *key);
int ty_aes128_ecb_decode(const unsigned char *data, const unsigned int len,\
                               unsigned char **dec_data, unsigned int *dec_len,\
                               const unsigned char *key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

