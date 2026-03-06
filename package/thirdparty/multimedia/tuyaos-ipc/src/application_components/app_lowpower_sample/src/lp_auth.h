#ifndef __LP_AUTH_H__
#define __LP_AUTH_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LP_AUTH_RANDOM_MAX_LEN 32

typedef struct lp_auth_info_ {
	char time[20];
	char random[LP_AUTH_RANDOM_MAX_LEN + 1];
} lp_auth_info_s;


typedef struct lp_auth_response_ {
	int 			interval;
	char 			random[LP_AUTH_RANDOM_MAX_LEN + 1];
	lp_auth_info_s 	authInfo;
	char 			sign[128];
} lp_auth_response_s;


typedef struct lp_auth_dev_msg_ {
	int 			sock;
	unsigned char 	key[32];
	int 			keyLen;
	unsigned char 	devId[64];
	int 			idLen;
} lp_auth_dev_msg_s;


int lp_auth_sign(unsigned char* psecret, int klen, char* devId, lp_auth_info_s *pauth,
					unsigned char* pb64d, size_t* psz);


int lp_auth_parse_auth(char* pauth, lp_auth_info_s* painfo);


int lp_auth_request(lp_auth_dev_msg_s* pdev, int type, int method,
					lp_auth_info_s *pauth, char* pbuf, int* psize);


int lp_auth_parse_request(char* pdata, int len, lp_auth_info_s* preqauth);


int lp_auth_response(lp_auth_dev_msg_s* pdev, lp_auth_info_s *pauth,
					 char* pbuf, int* psize);


int lp_auth_parse_response(char* pdata, int len, lp_auth_response_s* prsp);


int lp_auth_check_response(lp_auth_dev_msg_s* pdev, lp_auth_info_s *preq, lp_auth_response_s* prsp);


#ifdef __cplusplus
}
#endif


#endif