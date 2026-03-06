#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "mbedtls/md.h"
#include "mbedtls/md5.h"


int lp_crypt_hmac_sha256(unsigned char* pbuffer, int dlen,
                         unsigned char* psecret, int slen,
                         unsigned char* pdigest)
{
    int ret = -1;
    mbedtls_md_context_t sha_ctx;

    mbedtls_md_init(&sha_ctx);

    ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if(ret != 0) {
        printf("mbedtls_md_setup returned -0x%04x\n", -ret);
        mbedtls_md_free(&sha_ctx);
        return -1;
    }

    mbedtls_md_hmac_starts(&sha_ctx, psecret, slen);
    mbedtls_md_hmac_update(&sha_ctx, pbuffer, dlen);
    mbedtls_md_hmac_finish(&sha_ctx, pdigest);

    mbedtls_md_free(&sha_ctx);

    return ret;
}

int lp_crypt_aes_encrypt(unsigned char* pinput, int len, unsigned char* poutput, int* polen,
                         unsigned char* pkey, unsigned char* piv)
{
    int i = 0;
    int remainder = len % 16;
    int nopadlen = len - remainder;
    unsigned char padding[16] = {0};
    mbedtls_aes_context ctx;
    unsigned char iv[16 + 1] = {0};

    memcpy(iv, piv, 16);

    mbedtls_aes_init(&ctx);

    mbedtls_aes_setkey_enc(&ctx, pkey, 128);
    printf("key   is = ");
    for(i =0; i<strlen(pkey);i++)
    {
        printf("%02x",pkey[i]);
    }
    printf("\n");

    printf("iv   is = ");
    for(i =0; i<strlen(iv);i++)
    {
        printf("%02x",iv[i]);
    }
    printf("\n");


    if(nopadlen > 0) {
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, nopadlen, iv, pinput, poutput);
    }

    if(remainder > 0) {
        memcpy(padding, pinput + nopadlen, remainder);
    }

    for(i = 0; i < 16 - remainder; i++) {
        padding[i + remainder] = 16 - remainder;
    }
    printf("padding  nopadlen=%d,remainder=%d is = ",nopadlen,remainder);
    for(i=0;i<16;i++)
        printf("%02x",padding[i]);
    printf("\n");


    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, 16, iv, padding, poutput + nopadlen);

    mbedtls_aes_free(&ctx);

    *polen = len + (16 - remainder);

    printf("ase cbc is =");
    for(i=0;i<*polen;i++)
    {
        printf("%02x",poutput[i]);
    }
    printf("\n");

    return 0;
}

int lp_crypt_aes_decrypt(unsigned char* pinput, int len, unsigned char* poutput, int* polen,
                         unsigned char* pkey, unsigned char* piv)
{
    mbedtls_aes_context ctx;
    unsigned char iv[16 + 1] = {0};

    memcpy(iv, piv, 16);

    mbedtls_aes_init(&ctx);

    mbedtls_aes_setkey_dec(&ctx, pkey, 128);

    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, len, iv, pinput, poutput);

    mbedtls_aes_free(&ctx);

    *polen = len - poutput[len - 1];

    poutput[*polen] = '\0';

    return 0;
}

void lp_system_random_str(unsigned char s[], int num)
{
    int i = 0;
    unsigned char* str = (unsigned char*)"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int len = strlen((char*)str);

    srand((unsigned int)time((time_t *)NULL));

    for(i = 0; i < num; i++) {
        s[i] = str[(rand()%len)];
    }

    return;
}

uint64_t lp_time_utc_ms()
{
    struct timeval now;

    gettimeofday(&now, NULL);

    return ((uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_usec / 1000);
}

void lp_cli_print_hex(unsigned char* pdata, int len)
{
    int i = 0;

    printf("~~~~~~~~~~~~~~~ (%d)\n", len);

    for(i = 0; i < len; i++) {
        printf("%02x ", pdata[i]);

        if((i + 1) % 16 == 0) {
            printf("\n");
        }
    }

    printf("\n");

    return;
}

