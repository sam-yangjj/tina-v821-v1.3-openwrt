#include <string.h>
#include <stdlib.h>
#include "mbedtls/ecp.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/asn1.h"
#include "mbedtls/aes.h"
#include "aes_inf.h"

#define OPRT_OK (0)
#define CNTSOF(a)   (sizeof(a)/sizeof(a[0]))
#define TARGET_PLATFORM "a113x"
#define PROJECT_NAME "ai_speaker"

static const char *p_global_str1 = TARGET_PLATFORM;
static char p_global_str2[] = PROJECT_NAME;

int ty_init_kv_encrypted(char *key)
{
    static char static_str3[] = "HHRRQbyemo";

    if (!key) {
        printf("can not genreate key\n");
        return -1;
    }

    char final_str[64] = "L5yNB6mbAem9J75ebEOxoXz4ygTH9bj8GiexUIf4xPOzEJFPImqa";
    unsigned int index1 = 10;
    unsigned int index2 = CNTSOF(static_str3);
    final_str[index1++] = final_str[index2];
    final_str[index1 + index2] = static_str3[index1/4] + p_global_str2[index1/2] + p_global_str1[index1/3];
    index1 += index2;
    index2 += CNTSOF(p_global_str2);
    index2 = index2 % index1;
    memcpy(p_global_str2 + 1, p_global_str1, 3);
    memcpy(final_str + index1, p_global_str2, 5);
    memcpy(final_str + index2, p_global_str2, CNTSOF(static_str3));
    memcpy(key, final_str + index1,16);

    return 0;
}

/* 加密函数 */

/* 尾部填充pkcs7,返回填充后的长度 */
static unsigned int __Add_Pkcs(unsigned char *p, unsigned int len)
{           
    char pkcs[16];
    int i, cz = 16-len%16;
    memset(pkcs, 0, sizeof(pkcs));
    for( i=0; i<cz; i++ ) {
        pkcs[i]=cz;
    }       
    memcpy(p + len, pkcs, cz);
    return (len + cz);
} 

int ty_aes128_ecb_encode_raw( const unsigned char *data, const unsigned int len,\
                                   unsigned char *ec_data, const unsigned char *key)
{   
    if(NULL == data || NULL == key || NULL == ec_data || 0 == len)
        return -1;
    
    if(0 != len % 16)
        return -1;

    mbedtls_aes_context ctx;
    mbedtls_aes_init( &ctx );
    mbedtls_aes_setkey_enc( &ctx, key, 128);

    int index = 0;
    for(index = 0; index < len;index += 16)
        mbedtls_aes_crypt_ecb( &ctx, MBEDTLS_AES_ENCRYPT, data+index, ec_data+index);

    mbedtls_aes_free(&ctx);
            
    return OPRT_OK;
}

int ty_aes128_ecb_encode( const unsigned char *data, const unsigned int len,\
                               unsigned char **ec_data, unsigned int *ec_len,\
                               const unsigned char *key)
{            
    if(NULL == data || NULL == key || NULL == ec_data || NULL == ec_len || 0 == len)
        return -1;
             
    *ec_data = malloc(len + 16);
    if(NULL == *ec_data)
        return -1;
             
    memcpy(*ec_data, data, len);
    unsigned int pkcs7Len = __Add_Pkcs(*ec_data, len);
    *ec_len = pkcs7Len;
             
    int ret = ty_aes128_ecb_encode_raw(*ec_data, pkcs7Len, *ec_data, key);
    if(ret != OPRT_OK) {
        free(*ec_data);
        *ec_data = NULL;
        return ret;
    }
    return OPRT_OK;
}            

/* 解密函数 */

int ty_aes128_ecb_decode_raw(const unsigned char *data, const unsigned int len,\
                                   unsigned char *dec_data, const unsigned char *key)
{   
    if(NULL == data || 0 == len || NULL == key || NULL == dec_data )
        return -1;
    
    if(len % 16 != 0)
        return -1;

    mbedtls_aes_context ctx;
    mbedtls_aes_init( &ctx );
    mbedtls_aes_setkey_dec( &ctx, key, 128);

    int index = 0;
    for(index = 0; index < len;index += 16)
        mbedtls_aes_crypt_ecb( &ctx, MBEDTLS_AES_DECRYPT, data+index, dec_data+index);

    mbedtls_aes_free(&ctx);
   
            
    return OPRT_OK;
 
}

int ty_aes128_ecb_decode(const unsigned char *data, const unsigned int len,\
                               unsigned char **dec_data, unsigned int *dec_len,\
                               const unsigned char *key)
{                      
    if(NULL == data || 0 == len || NULL == key || NULL == dec_data ||NULL == dec_len)
        return -1;
                       
    *dec_len = len;    
    *dec_data = (unsigned char *)malloc(len+1);
    if(NULL == dec_data)
        return -1;
                       
    memcpy(*dec_data, data, len);
    (*dec_data)[len] = 0;
    int ret = ty_aes128_ecb_decode_raw(*dec_data, len, *dec_data, key);
    if(ret != OPRT_OK) {
        free(*dec_data);
        *dec_data = NULL;
        return ret;
    }
    (*dec_data)[len] = 0;
    return OPRT_OK;    
}                      
   
int ty_aes_get_actual_length(const unsigned char *dec_data, const unsigned int dec_data_len)                                                                                                    
{
    if(NULL == dec_data)
        return -1;
       
    unsigned char lastdata_val = dec_data[dec_data_len - 1] ;
    return dec_data_len - lastdata_val;
}
