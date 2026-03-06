#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "http_inf.h"
#include "unpack_httpc.h"
#include "md5.h"
#include "../notifier.h"

#define DEBUG_LOG
#ifdef DEBUG_LOG
#define dgb_print(format,...) printf (format, ##__VA_ARGS__)
#else
#define dgb_print(format, ...)
#endif

extern int ty_aes128_ecb_encode( const unsigned char *data, const unsigned int len,\
                                       unsigned char **ec_data, unsigned int *ec_len,\
                                                          const unsigned char *key);

static int http_recv_data(void * hand,void * buf,unsigned long len)
{
    return http_read_content((http_session_t)hand, buf, len);
}

static int tuya_http_inf_com_hanle(  http_req_t *req,\
                                          char *url,\
                                          http_hdr_field_sel_t field_flags,\
                                          http_inf_cb callback,
                                          char *p_decode_key,
                                         void * *pri_data)
{
    if(NULL == req || NULL == url)
        return OPRT_VALID_PARM;

    if( ((field_flags & HRD_ADD_HTTP_RAW) && (NULL == pri_data)) ||
        (((field_flags & HRD_ADD_HTTP_RAW) == 0) && (NULL == callback)) )
        return OPRT_VALID_PARM;

    http_session_t hnd;
    int ret = ty_http_open_session(&hnd, url, 0, 0);
    if (ret != 0) {
        printf("Open session failed: %s (%d)\n", url, ret);
        return ret;
    }

    ret = ty_http_prepare_req(hnd, req,field_flags);
    if (ret != 0) {
        ty_http_close_session(&hnd);
        printf("Prepare request failed: %d\n", ret);
        return OPRT_HTTP_PR_REQ_ERROR;
    }
    int send_content = 1;
    if(field_flags & HRD_ADD_HTTP_RAW)
        send_content = 0;

    ret = ty_http_send_request(hnd, req, send_content);
    if (ret != 0) {
        ty_http_close_session(&hnd);
        printf("Send request failed: %d\n", ret);
        return OPRT_HTTP_SD_REQ_ERROR;
    }

    if(field_flags & HRD_ADD_HTTP_RAW) {
        http_inf_h_s *p_inf = (http_inf_h_s *)pri_data;
        p_inf->hand = (void *)hnd;
        return OPRT_OK;
    }

    http_resp_t *resp;
    ret = ty_http_get_response_hdr(hnd, &resp);
    if (ret != 0) {
        ty_http_close_session(&hnd);
        printf("Get resp header failed: %d\n", ret);
        return OPRT_HTTP_GET_RESP_ERROR;
    }
    if(field_flags & HRD_ADD_DOWNLOAD_RANGE)
    {
        if( (resp->content_length == 0) || \
            ((resp->status_code != 200) && (resp->status_code != 206)) ||
            ((resp->p_accept_ranges != NULL) && (strcmp(resp->p_accept_ranges, "bytes") != 0))
          )
        {
            printf("Server Maybe Not support download in range. %u %d %s\n",
                   resp->content_length, resp->status_code, resp->p_accept_ranges);
            ty_http_close_session(&hnd);
            return OPRT_HTTPS_NO_SUPPORT_RANGE;
        }
    }

    http_inf_h_s inf_h;

    memset(&inf_h,0,sizeof(inf_h));
    inf_h.chunked = resp->chunked;
    inf_h.content_len = resp->content_length;
    inf_h.hand = (void *)hnd;
    inf_h.recv = http_recv_data;
    inf_h.pri_data = pri_data;
    inf_h.status_code = resp->status_code;
    inf_h.p_decode_key = (char *)p_decode_key;
    int op_ret = callback(&inf_h);
    ty_http_close_session(&hnd);

    return op_ret;
}

/***********************************************************
*  Function: http_inf_client_get
*  Input: url callback
*  Output: pri_data
*  Return: OPERATE_RET
***********************************************************/
int http_inf_client_get(  char *url,\
                                  http_inf_cb callback,\
                                 void * *pri_data)
{
    if(NULL == url || \
       NULL == callback) {
        return OPRT_VALID_PARM;
    }

    http_req_t req = {
        .type = HTTP_GET,
        .resource = url,
        .version = HTTP_VER_1_1,
    };

    int op_ret = OPRT_OK;
    op_ret = tuya_http_inf_com_hanle(&req,url,STANDARD_HDR_FLAGS|HDR_ADD_CONN_KEEP_ALIVE,callback, NULL, pri_data);

    return op_ret;
}

 static int ty_http_inf_get_file(char *url,\
                                   http_inf_cb callback,\
                                   void * *pri_data, \
                                   unsigned long file_offset,\
                                   unsigned long file_size)
{
    int op_ret;

    if(NULL == url || \
       NULL == callback) {
        return OPRT_VALID_PARM;
    }

    http_req_t req = {
        .type = HTTP_GET,
        .resource = url,
        .version = HTTP_VER_1_1,
        .download_offset = file_offset,
        .download_size = file_size,
    };

    if(file_size != 0) {
        dgb_print("DEBUG: Get File In Range Mode [%d %d]", file_offset, file_size);
        op_ret = tuya_http_inf_com_hanle(&req,url,STANDARD_HDR_FLAGS|HDR_ADD_CONN_KEEP_ALIVE|HRD_ADD_DOWNLOAD_RANGE,callback,NULL, pri_data);
    }else {
        dgb_print("Get File In Full Mode", file_offset, file_size);
        op_ret = tuya_http_inf_com_hanle(&req,url,STANDARD_HDR_FLAGS|HDR_ADD_CONN_KEEP_ALIVE,callback,NULL, pri_data);
    }
    return op_ret;
}

static int ty_http_inf_post_msg(  char *url,\
                                http_inf_cb callback,\
                                unsigned char *data,\
                                unsigned long len,\
                                char *p_decode_key,\
                                char *header_data,\
                                void * *pri_data)

{
    int op_ret;
    if(NULL == url || NULL == callback)
    {
        return OPRT_VALID_PARM;
    }

    http_req_t req = {
        .type = HTTP_POST,
        .resource = url,
        .version = HTTP_VER_1_1,
        .content = (char *)data,
        .content_len = len,
        .user_header = header_data,
    };
    op_ret = tuya_http_inf_com_hanle(&req, url, STANDARD_HDR_FLAGS|HDR_ADD_CONN_KEEP_ALIVE|HDR_ADD_CONTENT_TYPE_FORM_URLENCODE,
                                  callback, p_decode_key, pri_data);

    return op_ret;
}

http_url_h_s *ty_create_http_url_h(int buf_len, int param_cnt)
{
    short len = 0;
    short total_len = 0;

    if(0 == buf_len) {
        len = DEF_URL_LEN;
    }else {
        len = buf_len;
    }

    total_len = (sizeof(http_url_h_s) + len + \
                                sizeof(http_param_h_s) + sizeof(char *)*param_cnt);

    http_url_h_s *hu_h = malloc(total_len);
    if(NULL == hu_h) {
        return NULL;
    }
    memset(hu_h, 0, total_len);

    hu_h->head_size = 0;
    hu_h->param_in = hu_h->buf;
    hu_h->buf_len = len;

    http_param_h_s *param_h = (http_param_h_s *)((unsigned char *)hu_h + (sizeof(http_url_h_s) + len));
    param_h->cnt = 0;
    param_h->total = param_cnt;

    hu_h->param_h = param_h;

    return hu_h;
}

/***********************************************************
*  Function: ty_del_http_url_h
*  Input: hu_h
*  Output:
*  Return: http_url_h_s *
***********************************************************/
void ty_del_http_url_h(http_url_h_s *hu_h)
{
    free(hu_h);
}

/***********************************************************
*  Function: ty_fill_url_head
*  Input: hu_h:http url handle
*         url_h:url head
*  Output: hu_h
*  Return: int
***********************************************************/
int ty_fill_url_head(http_url_h_s *hu_h, char *url_h)
{
    if(NULL == hu_h || \
       NULL == url_h) {
        return OPRT_INVALID_PARM;
    }
    hu_h->head_size = strlen(url_h);

    strncpy(hu_h->param_in,url_h,hu_h->head_size);
    hu_h->param_in += hu_h->head_size;

    return OPRT_OK;
}

/***********************************************************
*  Function: ty_fill_url_param
*  Input: hu_h
*         key
*         value
*  Output: hu_h
*  Return: int
***********************************************************/
int ty_fill_url_param(http_url_h_s *hu_h, char *key, char *value)
{
  if(NULL == hu_h || \
     NULL == key || \
     NULL == value) {
      return OPRT_INVALID_PARM;
  }

  if('?' != hu_h->buf[hu_h->head_size-1]) {
      strncpy(hu_h->param_in,"?",strlen("?"));
      hu_h->param_in += strlen("?");
      hu_h->head_size += strlen("?");
  }

  int ret;
  ret = snprintf(hu_h->param_in,((hu_h->buf+hu_h->buf_len)-hu_h->param_in),\
                 "%s=%s",key,value);
  if(ret != (strlen(hu_h->param_in))) {
      if(ret < 0) {
          return OPRT_COM_ERROR;
      }else {
          return OPRT_BUF_NOT_ENOUGH;
      }
  }

  http_param_h_s *param_h = hu_h->param_h;
  if(param_h->cnt >= param_h->total) {
      return OPRT_URL_PARAM_OUT_LIMIT;
  }

  param_h->pos[param_h->cnt++] = hu_h->param_in;
  hu_h->param_in += ret+1; // "key=value"

  return OPRT_OK;
}

int ty_uni_strcmp   (const char *src,const char *dst)
{
    int ret = 0;

    while(!(ret  =  *(unsigned char *)src - *(unsigned char *)dst) && *dst) {
        ++src,++dst;
    }

    if(ret < 0) {
        ret = -1;
    } else if(ret > 0) {
        ret = 1;
    }

    return (ret);
}

static int ty_make_url(http_url_h_s *hu_h, char *key)
{
    if(NULL == hu_h || NULL == hu_h->param_h || (hu_h->param_h && (0 == hu_h->param_h->cnt)))
    {
        return OPRT_INVALID_PARM;
    }

    http_param_h_s *param_h = hu_h->param_h;
    int i,j,k;
    char *tmp_str;

    // use key to sort
    for(i = 0;i < param_h->cnt;i++) {
        char *tmp;
        tmp = strstr(param_h->pos[i],"=");
        *tmp = 0;
    }

    // sort use choice method
    for(i = 0; i < (param_h->cnt-1);i++) {
        k = i;
        for(j = i+1;j < param_h->cnt;j++) {
            if(ty_uni_strcmp(param_h->pos[k],param_h->pos[j]) > 0) {
                tmp_str = param_h->pos[k];
                param_h->pos[k] = param_h->pos[j];
                param_h->pos[j] = tmp_str;
            }
        }
    }

    // data restore
    for(i = 0;i < param_h->cnt;i++) {
        *(param_h->pos[i] + strlen(param_h->pos[i])) = '=';
    }

    // make md5 sign
    char sign[32+1];
    MD5_CTX md5;
    MD5Init(&md5);
    unsigned char decrypt[16];

    for(i = 0;i < param_h->cnt;i++) {
        MD5Update(&md5,(unsigned char *)param_h->pos[i],strlen(param_h->pos[i]));
        MD5Update(&md5,(unsigned char *)"||",2);
        //printf("%s %d %d||\n",param_h->pos[i],i ,strlen(param_h->pos[i]));
    }

    if(key != NULL){
        MD5Update(&md5,(unsigned char *)key,strlen(key));
        //printf("%s , %d\n",key,strlen(key));
    }
    //PR_DEBUG_RAW("%s\r\n",key);
    MD5Final(&md5,decrypt);

    int offset = 0;
    for(i = 0;i < 16;i++) {
        sprintf(&sign[offset],"%02x",decrypt[i]);
        offset += 2;
    }
    sign[offset] = 0;

    // to sort string
    int len = hu_h->param_in - (hu_h->buf+hu_h->head_size);
    tmp_str = (char *)malloc(len+1); // +1 to save 0
    if(tmp_str == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    offset = 0;
    for(i = 0;i < param_h->cnt;i++) {
        strncpy(tmp_str+offset,param_h->pos[i],strlen(param_h->pos[i]));
        offset += strlen(param_h->pos[i]);
        tmp_str[offset++] = '&';
    }
    tmp_str[offset] = 0;

    // copy to the huh handle
    strcpy((hu_h->buf+hu_h->head_size),tmp_str);
    free(tmp_str);

    // set sign to url
    int op_ret;
    op_ret = ty_fill_url_param(hu_h,"sign",sign);
    if(OPRT_OK != op_ret) {
        return op_ret;
    }

    return op_ret;
}

int ty_make_full_url(char *url,\
                        char *api_name,\
                        char *api_ver,\
                        char *aes_key,\
                        char *devid,\
                        char *result)
{

    http_url_h_s *hu_h = ty_create_http_url_h(0,10);
    if(NULL == hu_h) {
        printf("ty_create_http_url_h error\n");
        return -1;
    }

    int  op_ret = OPRT_OK;

    // 构造header
    op_ret = ty_fill_url_head(hu_h,url);
    if(op_ret != OPRT_OK) {
        printf("set url fail.ret:%d %s\n", op_ret, url);
        goto ERR_HANDLE;
    }

    char time_str[15];
    time_t curr_time = time((time_t *)NULL);
    snprintf(time_str, 15, "%u", curr_time);
    ty_fill_url_param(hu_h, "a", api_name);
    if(devid != NULL)        { ty_fill_url_param(hu_h, "devId", devid); }
    ty_fill_url_param(hu_h,"et","1"); //加密
    ty_fill_url_param(hu_h, "t", time_str);
    if(api_ver != NULL  && strcmp(api_ver, "1.0") != 0)      { ty_fill_url_param(hu_h, "v", api_ver);   }

    //构造整个URL,把header计算出md5，然后放到sign里面
    op_ret = ty_make_url(hu_h, aes_key);
    if(op_ret != OPRT_OK) {
        printf("Make Full URL Fail.\n");
        goto ERR_HANDLE;
    }

    //发送
    dgb_print("post url: %s ;\n", hu_h->buf);
    memcpy(result, hu_h->buf, strlen(hu_h->buf));
    if(op_ret != OPRT_OK) {
        printf("Post Data Return Fail.%d\n", op_ret);
        goto ERR_HANDLE;
    }
ERR_HANDLE:
    ty_del_http_url_h(hu_h);
    return op_ret;
}

void com_hex_str(unsigned char *outtxt, unsigned char *inchar, unsigned int len )
{
    unsigned char hbit,lbit;
    unsigned int i;
    for(i=0;i<len;i++)
    {
    hbit = (*(inchar+i)&0xf0)>>4;
    lbit = *(inchar+i)&0x0f;
    if (hbit>9) outtxt[2*i]='A'+hbit-10;
    else outtxt[2*i]='0'+hbit;
    if (lbit>9) outtxt[2*i+1]='A'+lbit-10;
    else    outtxt[2*i+1]='0'+lbit;
    }
    outtxt[2*i] = 0;
}

static int ty_httpc_data_aes_proc(unsigned char *in, unsigned int len,\
    char *key, char **pp_out)
{
    if(NULL == in || 0 == len || NULL == key || NULL == pp_out) {
        return OPRT_INVALID_PARM;
    }

    unsigned int buf_len = 0;
    unsigned char *buf = NULL;
    int op_ret = OPRT_OK;

    if(key != NULL) {
        unsigned char encode_key[16] = {0};
        memcpy(encode_key, key, strlen(key)<=16?strlen(key):16);
        op_ret = ty_aes128_ecb_encode(in,len,&buf,&buf_len,(unsigned char *)encode_key);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }
    } else {
        buf_len = len;
        buf = malloc(buf_len);
        if(buf == NULL) {
            return OPRT_MALLOC_FAILED;
        }
        memcpy(buf, in, buf_len);
    }

    char *out = malloc(2*buf_len + 10);
    if(NULL == out) {
        free(buf);
        return OPRT_MALLOC_FAILED;
    }
    memset(out,0,(2*buf_len + 10));

    // prepare data=ase(data)
    strcpy(out,"data=");
    com_hex_str((unsigned char *)out+strlen("data="), (unsigned char *)buf, buf_len);
    free(buf);
    *pp_out = out;

    return OPRT_OK;
}

int ty_make_encode_data(int progress , char *buf,char *aes_key)
{
    int op_ret = 0;
    char *fal_send = NULL;
    time_t curr_time = time((time_t *)NULL);
    char *post_data = malloc(128);
    if(post_data == NULL)
    {
        printf("Malloc Fail.\n");
        return OPRT_MALLOC_FAILED;
    }
    memset(post_data, 0, 128);
    snprintf(post_data, 128, "{\"progress\":%d}", progress);
    
    if(post_data != NULL) {
        int actual_len = strlen(post_data);
        int left_len = 128 - actual_len;

        snprintf(post_data + actual_len - 1, left_len + 1, ",\"t\":%u}", curr_time);
    } 

    dgb_print("Post Data: %s", post_data);

    //加密Data
    op_ret = ty_httpc_data_aes_proc((unsigned char *)post_data, strlen(post_data), aes_key, &fal_send);
    if(OPRT_OK != op_ret) {
       printf("AES Post Data Fail.\n");
    }
    memcpy(buf, fal_send, strlen(fal_send));
    if(fal_send != NULL) {
    free(fal_send);
    fal_send = NULL;
    }
    return 0;
}

static int upd_getfile_cb(http_inf_h_s *hand)
{
    long int down_size = 0;
    GET_FILE_S *gf = (GET_FILE_S *)(*(hand->pri_data));
    int op_ret = OPRT_OK;
    unsigned int recv_size = 0;
    int ret = 0;
    int idx = 0;

    if(TRUE == hand->chunked) {
        dgb_print("\n hand->chunked %d\n %d\n",hand->chunked,TRUE);
        return OPRT_COM_ERROR;
    }

    while(recv_size < hand->content_len) {

        if(gf->len - gf->buf_off != 0)
        {
            ret = hand->recv(hand->hand, gf->buf + gf->buf_off, gf->len - gf->buf_off);
            if(ret < 0)
            {
                printf("socket rev error %d\n",ret);
                gf->gfd_cb(gf->buf,ret,gf->priv_data);
                op_ret = -1;
                break;
            }else if(0 == ret)
            {
                printf("socket rev empty this time\n");
                break;
            }
        } else {
            printf("gf->len == gf->buf_off %u\n", gf->buf_off);
            ret = 0;
        }
        op_ret = gf->gfd_cb(gf->buf,ret,gf->priv_data);

        if(op_ret != OPRT_OK)
        {
            printf("write data handle error %d\n", op_ret);
            break;
        }
        recv_size += ret;
        //printf("\nrecv_size %d , hand->content_len %d\n",recv_size,hand->content_len);

    }
    dgb_print("download success this time\n");
    return op_ret;
}

static int upd_post_resp_cb(http_inf_h_s *hand)
{
    if(TRUE == hand->chunked) {
        dgb_print("\n hand->chunked %d\n %d\n",hand->chunked,TRUE);
        return OPRT_COM_ERROR;
    }
    long int down_size = 0;

    GET_FILE_S *gf = (GET_FILE_S *)(*(hand->pri_data));

    int op_ret = OPRT_OK;
    unsigned int recv_size = 0;
    int ret = 0;
    int idx = 0;

    while(recv_size < hand->content_len) {

        if(gf->len - gf->buf_off != 0)
        {
            ret = hand->recv(hand->hand, gf->buf + gf->buf_off, gf->len - gf->buf_off);
            if(ret < 0)
            {
                printf("socket rev error %d\n",ret);
                op_ret = -1;
                break;
            }else if(0 == ret)
            {
                printf("socket rev empty this time\n");
                break;
            }
        } else {
            printf("gf->len == gf->buf_off %u\n", gf->buf_off);
            ret = 0;
        }
        recv_size += ret;
        gf->buf_off += ret;
    }
    op_ret = gf->gfd_cb(gf->buf,ret,gf->priv_data);
    if(op_ret != OPRT_OK)
    {
        printf("write data handle error %d\n", op_ret);
    }

    return op_ret;
}

int tuya_http_get_file(char *url,\
                            http_recv_cb callback,\
                            long offset_start,\
                            long download_size,\
                            void *pri_data)
{
    GET_FILE_S *gf = NULL;
    int mlk_buf_len = 64*1024;
    int op_ret = OPRT_OK;
    int try_time = 0;
    gf = malloc(sizeof(GET_FILE_S)+mlk_buf_len);
    if(NULL == gf)
    {
        printf("malloc fails %ld\n", (sizeof(GET_FILE_S)+mlk_buf_len) );
        return OPRT_MALLOC_FAILED;
    }
    gf->total_size = download_size;
    gf->len = mlk_buf_len;
    gf->buf = (unsigned char *)gf + sizeof(GET_FILE_S);
    gf->buf_off = 0;
    gf->offset_size = 0;
    gf->priv_data = pri_data;
    gf->gfd_cb = callback;

    for(try_time = 0; try_time < 2; try_time++)
    {
        op_ret = ty_http_inf_get_file(url,upd_getfile_cb,(void * *)&gf, offset_start, download_size);
        if(op_ret == OPRT_OK)
        {
             dgb_print("download file finish\n");
             break;
        }
        if(op_ret == OPRT_HTTPS_NO_SUPPORT_RANGE)
        {
             printf("Server Not Support Range Mode. Try Download in Full Mode Once\n");
             op_ret = ty_http_inf_get_file(url,upd_getfile_cb,(void * *)&gf, 0, 0);
             break;
        }
        sleep(2);
    }

    free(gf);
    return op_ret;
}

int tuya_http_inf_post(char *url,\
                            http_recv_cb callback,\
                            unsigned char *data,\
                            unsigned long len,\
                            char *header_data,\
                            void * pri_data)
{
    GET_FILE_S *gf = NULL;
    int mlk_buf_len = 64*1024;
    gf = malloc(sizeof(GET_FILE_S)+mlk_buf_len);
    if(NULL == gf)
    {
        printf("malloc fails %ld\n", (sizeof(GET_FILE_S)+mlk_buf_len) );
        return OPRT_MALLOC_FAILED;
    }

    gf->len = mlk_buf_len;
    gf->buf = (unsigned char *)gf + sizeof(GET_FILE_S);
    gf->buf_off = 0;
    gf->offset_size = 0;
    gf->priv_data = pri_data;
    gf->gfd_cb = callback;
    int op_ret = OPRT_OK;

    unsigned int total_size = gf->total_size;

    op_ret = ty_http_inf_post_msg(url, upd_post_resp_cb,(unsigned char *)data, strlen(data),NULL, header_data, (void * *)&gf);
    if(OPRT_OK != op_ret) {
        free(gf);
        return op_ret;
    }
    free(gf);
    return OPRT_OK;
}
