#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "lp_com.h"
#include "lp_auth.h"

int lp_auth_parse_auth(char* pauth, lp_auth_info_s* painfo)
{
	char *strv = pauth;
	char* token = strsep(&strv, ",");

	if(pauth == NULL || painfo == NULL) {
		return -1;
	}

	while(token != NULL) {
		char* ptr = NULL;

		if((ptr = strstr(token, "time=")) != NULL) {
			strncpy(painfo->time, ptr + strlen("time="), strlen(token) - strlen("time="));
		} else if ((ptr = strstr(token, "random=")) != NULL) {
			strncpy(painfo->random, ptr + strlen("random="), strlen(token) - strlen("random="));
		}

		token = strsep(&strv, ",");
	} 

	return 0;
}

int lp_auth_sign(unsigned char* psecret, int klen, char* devId,
				lp_auth_info_s *pauth, unsigned char* pb64d, size_t* psz)
{
	unsigned char digest[32] = {0};
	unsigned char temp[256] = {0};

	snprintf((char*)temp, sizeof(temp), "%s:%s:%s", devId, pauth->time, pauth->random);
    printf("sig=%s\n",temp);
    printf("hmac key is %s\n",psecret);

	if(lp_crypt_hmac_sha256(temp, strlen((char*)temp), psecret, klen, digest) < 0) {
		return -1;
	}

    size_t dlen = (32 + 3) / 3 * 4 + 1;

    if(*psz < dlen) {
    	return -1;
    }

    mbedtls_base64_encode(pb64d, *psz, psz, digest, 32);
    printf("sig bs64=%s\n",pb64d);

	return 0;
}

int lp_auth_request(lp_auth_dev_msg_s* pdev, int type, int method,
					lp_auth_info_s *pauth, char* pbuf, int* psize)
{
	int ret = -1;
	//ty_cJSON* proot = NULL;
	char auth[128] = {0};
    unsigned char b64d[128] = {0};
	size_t sz = sizeof(b64d);

    lp_auth_sign(pdev->key, pdev->keyLen, (char*)pdev->devId, pauth, b64d, &sz);

    snprintf(auth, sizeof(auth), "time=%s,random=%s", pauth->time, pauth->random);

    char sendSig[182]={0};
    snprintf(sendSig,182,"{\"type\":%d,\"method\":%d,\"authorization\":\"%s\",\"signature\":\"%s\"}",type,method,auth,b64d);

	printf("send json is new  %s\n",sendSig);
	int len = strlen(sendSig);


	if(*psize > len && len > 0) {
		memcpy(pbuf, sendSig, len);
		*psize = len;
		ret = 0;
	}

	return ret;
}

int lp_auth_parse_response(char* pdata, int len, lp_auth_response_s* prsp)
{
    printf("rev is response is %s\n",pdata);


    char * errPtr = strstr(pdata,"err");
    if(errPtr ==NULL)
    {
        return -1;
    }
    int errvalue=-1;

    char colonCh=':';
    //
    char * position=strchr(errPtr,colonCh);
    sscanf(position+1,"%d",&errvalue);
    printf("err number is %d\n",errvalue);
    if(errvalue != 0)
    {
        printf("~~~~~~~~~~~~~ err: %d\n", errvalue);
        return -1;
    }
    //
    char * interValPtr = strstr(pdata,"interval");
    if(interValPtr == NULL)
    {
        return -1;
    }

    position=strchr(interValPtr,colonCh);
    sscanf(position+1,"%d",&(prsp->interval));
    printf("interval is %d\n",prsp->interval);
    //
    char * randomPtr = strstr(pdata,"random");
    if(randomPtr == NULL)
    {
        return -1;
    }
    char commaCh='\"';
    position=strchr(randomPtr,colonCh);
    if(position == NULL)
    {
        return -1;
    }

    char * startPosition=strchr(position,commaCh);
    if(startPosition == NULL)
    {
        return -1;
    }

    char * endPosition = strchr(startPosition+1,commaCh);
    if(endPosition == NULL)
    {
        return -1;
    }
    int valueLen = (int)(endPosition - startPosition)-1;
    strncpy(prsp->random,startPosition+1,valueLen);
    printf("rev random len is =%d,value %s\n",valueLen,prsp->random);

    //
    char * authPtr = strstr(pdata,"authorization");
    if(authPtr == NULL)
    {
        return -1;
    }
    position=strchr(authPtr,colonCh);
    if(position == NULL)
    {
        return -1;
    }
    startPosition=strchr(position,commaCh);
    if(startPosition == NULL)
    {
        return -1;
    }
    endPosition = strchr(startPosition+1,commaCh);
    if(endPosition == NULL)
    {
        return -1;
    }
    valueLen = (int)(endPosition - startPosition)-1;
    char authBuf[64]={0};
    strncpy(authBuf,startPosition+1,valueLen);
    printf("rev authorization len is =%d,value %s\n",valueLen,authBuf);
    lp_auth_parse_auth(authBuf, &prsp->authInfo);

    //
    char * sigPtr = strstr(pdata,"signature");
    if(sigPtr == NULL)
    {
        return -1;
    }
    position=strchr(sigPtr,colonCh);
    if(position == NULL)
    {
        return -1;
    }
    startPosition=strchr(position,commaCh);
    if(startPosition == NULL)
    {
        return -1;
    }
    endPosition = strchr(startPosition+1,commaCh);
    if(endPosition == NULL)
    {
        return -1;
    }
    valueLen = (int)(endPosition - startPosition)-1;
    strncpy(prsp->sign, startPosition+1, valueLen);
    printf("rev sig len is =%d,value %s\n",valueLen,prsp->sign);

	return 0;
}

int lp_auth_check_response(lp_auth_dev_msg_s* pdev, lp_auth_info_s *preq, lp_auth_response_s* prsp)
{
	unsigned char b64d[128] = {0};
	size_t sz = sizeof(b64d);

	if(memcmp(preq->random, prsp->random, strlen(prsp->random)) != 0) {
		printf("random not same: %s:%s\n", preq->random, prsp->random);
		return 0;
	}

    lp_auth_sign(pdev->key, pdev->keyLen, (char*)pdev->devId, &prsp->authInfo, b64d, &sz);

    if(memcmp(prsp->sign, b64d, sz) != 0) {
    	printf("sign error\n");
    	return 0;
    }

	return 1;
}

