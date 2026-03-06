/**
 * @file tkl_passwd.c
 * @brief the implements of ty os passwd change
 * @version 0.1
 * @date 2019-08-15
 *
 * @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
 *
 */

#define  _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <shadow.h>
#include <sys/types.h>
#include <pwd.h>
#include <crypt.h>
#include <stdlib.h>
#include <time.h>
#include "sys_log.h"
#include "tkl_passwd.h"

#ifndef TAG
#define TAG "tkl_passwd"
#endif

#define MAX_PW_SALT_LEN (3 + 16 + 1)
#define SHADOW_PATH "/etc/shadow"
#define MAX_PWD_STOR (100)

/* static const uint8_t ascii64[] ALIGN1 =
 * "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
 */
static int i64c(int i)
{
    i &= 0x3f;
    if (i == 0)
        return '.';
    if (i == 1)
        return '/';
    if (i < 12)
        return ('0' - 2 + i);
    if (i < 38)
        return ('A' - 12 + i);
    return ('a' - 38 + i);
}

static unsigned long long monotonic_us(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC,&ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec/1000;
}

static int crypt_make_salt(char *p,int cnt )
{
    /* was: x += ... */
    unsigned x = getpid() + monotonic_us();

    do {
        /* x = (x*1664525 + 1013904223) % 2^32 generator is lame
         * (low-order bit is not "random", etc...),
         * but for our purposes it is good enough */
        x = x*1664525 + 1013904223;
        /* BTW, Park and Miller's "minimal standard generator" is
         * x = x*16807 % ((2^31)-1)
         * It has no problem with visibly alternating lowest bit
         * but is also weak in cryptographic sense + needs div,
         * which needs more code (and slower) on many CPUs */
        *p++ = i64c(x >> 16);
        *p++ = i64c(x >> 22);
    } while (--cnt);
    *p = '\0';
    return x;
}


static int crypt_create_salt( TKL_PWD_ALGO_E algo,char salt[MAX_PW_SALT_LEN]) 
{
    int len = 2/2;
    char *salt_ptr = salt;

    if(algo == E_TKL_PWD_ALGO_MD5) {
        len = 8/2;
        *salt_ptr++ = '$';
        *salt_ptr++ = '1';
        *salt_ptr++ = '$';
    }else if(algo == E_TKL_PWD_ALGO_SHA256) {
        len = 16/2;
        *salt_ptr++ = '$';
        *salt_ptr++ = '5';
        *salt_ptr++ = '$';
    }else if(algo == E_TKL_PWD_ALGO_SHA512) {
        len = 16/2;
        *salt_ptr++ = '$';
        *salt_ptr++ = '6';
        *salt_ptr++ = '$';
    }

    crypt_make_salt(salt_ptr, len);

    return 0;
}

OPERATE_RET tkl_passwd_update(TKL_PWD_ALGO_E algo,const char *user, const char *passwd) 
{
    int i;
    FILE *fs;
    struct passwd* cur;
    int total = 0;
    int succeed_flag = 0;
    uid_t ruid,euid,suid;
    char salt[MAX_PW_SALT_LEN];
    OPERATE_RET iret = OPRT_COM_ERROR;
    struct spwd store[MAX_PWD_STOR],*p;

    memset(salt,0,MAX_PW_SALT_LEN);

    if(user == NULL || passwd == NULL) {
        return OPRT_INVALID_PARM;
    }

    if(crypt_create_salt(algo,salt)) {
        LOGE(TAG,"crypt_create_salt fail\n");
        return OPRT_COM_ERROR;
    }

    if(getresuid(&ruid,&euid,&suid) <= -1) {
        LOGE(TAG,"getresuid fail\n");
        return OPRT_COM_ERROR;
    }

    if((fs=fopen(SHADOW_PATH,"r")) == NULL) {
        LOGE(TAG,"%s open fail\n",SHADOW_PATH);
        return OPRT_COM_ERROR;
    }

    memset(store,0,sizeof(store));

    cur =  getpwuid(getuid());

    if(ruid && (cur == NULL || strcmp(cur->pw_name,user))) {
        LOGE(TAG,"ilegal passwd update\n");
        return OPRT_COM_ERROR;
    }
     
    while((p=fgetspent(fs))!=NULL && total < MAX_PWD_STOR) {

        store[total] = *p;

	store[total].sp_namp = malloc(strlen(p->sp_namp)+1);

	if(store[total].sp_namp == NULL) {
	     LOGE(TAG,"malloc fail\n");
	     goto err;
	}

	if(strcmp(p->sp_namp,user) == 0) {
	    char *crypt_passwd;

	    crypt_passwd = crypt(passwd,salt);
            store[total].sp_pwdp = malloc(strlen(crypt_passwd)+1);

	    if(store[total].sp_pwdp == NULL) {
                free(store[total].sp_namp);
	        LOGE(TAG,"malloc fail\n");
                goto err;
	    }

	    strcpy(store[total].sp_pwdp,crypt_passwd);
            succeed_flag = 1;

        }else{
            store[total].sp_pwdp = malloc(strlen(p->sp_pwdp)+1);
            if(store[total].sp_pwdp == NULL) {
      	        free(store[total].sp_namp);
	        LOGE(TAG,"malloc fail\n");
                goto err;
	    }

	    strcpy(store[total].sp_pwdp,p->sp_pwdp);
	}

        strcpy(store[total].sp_namp,p->sp_namp);

        total++;		
    }

    if(succeed_flag) {

        if((fs=fopen(SHADOW_PATH,"w")) == NULL) {
            LOGE(TAG,"%s open fail\n",SHADOW_PATH);
            iret =  OPRT_COM_ERROR;
	    goto err1;
        }

        for(i=0;i<total;i++) {
            if(putspent(&store[i],fs)) {
		LOGE(TAG,"putspent fail\n");
                fclose(fs);
		remove(SHADOW_PATH);
		goto err1;
	    }
	}

	iret = OPRT_OK;
    }

err: 
    fclose(fs);
err1:
    for(i=0;i<total;i++) {
        free(store[i].sp_pwdp);
        free(store[i].sp_namp);
    }

    return iret; 
}
