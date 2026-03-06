#ifndef HTTP_INF_H
#define HTTP_INF_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "unpack_httpc.h"

#define DEF_URL_LEN 1024
typedef int (*http_data_recv)(void * hand,void * buf,unsigned long len);

typedef struct {
    int chunked;
    unsigned long content_len; // if(chunked == FALSE) then valid.
    void * hand;
    http_data_recv recv; // system auto fill
    void * *pri_data; // private data
    long status_code;
    char *p_decode_key;
}http_inf_h_s;

typedef int (*http_inf_cb)(http_inf_h_s *hand);

typedef struct {
    unsigned char total;
    unsigned char cnt;
    char *pos[0];
}http_param_h_s;

typedef struct {
    http_param_h_s *param_h; // param handle
    char *param_in; // param insert pos
    unsigned short head_size; // head_size == "url?" or "url"
    unsigned short buf_len; // bufer len
    char buf[0]; // "url?key=value" + "kev=value statistics"
}http_url_h_s;

typedef int (*http_recv_cb)(char *buf, int size, void *user_data);

typedef struct {
    void * priv_data;
    http_recv_cb gfd_cb;
    unsigned int download_size_this_time; /* 本次循环下载的长度 */
    unsigned int download_size_all; /* 整个下载过程已经下载的长度 */
    unsigned int total_size; /* 文件总长度 */
    unsigned int len; /* len为buffer的长度 */
    unsigned char *buf;
    unsigned int buf_off;
    unsigned int offset_size;
}GET_FILE_S;

/**
 * @brief encode data
 * @param status of upgrade
 * @param buf save encode data
 * @param aes_key form cloud
 * @return success return 0, failed return -errno
 */
extern int ty_make_encode_data(int progress , char *buf,char *aes_key);

/**
 * @brief make full url
 * @param post url of origin
 * @param api_name of post to cloud
 * @param api_version of post to cloud
 * @param aes_key form cloud
 * @param devid of device
 * @param output full url
 * @return success return 0, failed return -errno
 */
extern int ty_make_full_url(char * url,\
                                char * api_name,\
                                char * api_ver,\
                                char * aes_key,\
                                char * devid,\
                                char * result);

/**
 * @brief download file from http server
 * @param http server url address
 * @param user recv data callback fun
 * @param offset of download file
 * @param size of download file
 * @param user private data
 * @return success return 0, failed return -errno
 */
extern int tuya_http_get_file(char *url,\
                            http_recv_cb callback,\
                            long offset_start,\
                            long download_size,\
                            void *pri_data);
/**
 * @brief post msg to http server
 * @param http server url address
 * @param user recv resp callback fun
 * @param post data buffer
 * @param size of data
 * @param header_data of user define
 * @param user private data
 * @return success return 0, failed return -errno
 */
extern int tuya_http_inf_post(char *url,\
                            http_recv_cb callback,\
                            unsigned char *data,\
                            unsigned long len,\
                            char *header_data,\
                            void * pri_data);

#ifdef __cplusplus
}
#endif
#endif

