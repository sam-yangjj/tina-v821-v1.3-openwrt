#include "unpack_httpc.h"
#include "tuya_tls.h"

#define net_close(c) unw_close(c)

#define PARSE_EXTRA_BYTES 10
#define DEFAULT_RETRY_COUNT 2
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_HTTPS_PORT 443

#define DEBUG_LOG
#ifdef DEBUG_LOG
#define dgb_print(format,...) printf (format, ##__VA_ARGS__)
#else
#define dgb_print(format, ...)
#endif


/** Max amount of time in mS, the socket functions user (httpc module) will
    retry in case there are timeouts */
#define DEF_T_MAX_TO_MS 1000

/*
 * We are not sure of the exact header size. Instead of reading one octet
 * at a time we will have prefetch buffer. This buffer will be used to read
 * the either header and then this header is parsed.
 * This prefetch buffer is also used as a buffer to store the request
 * header, before sending an HTTP request.
 */
#if defined(ENABLE_CLOUD_OPERATION) && (ENABLE_CLOUD_OPERATION==1)
#define MAX_REQ_RESP_HDR_SIZE 4096
#else
#define MAX_REQ_RESP_HDR_SIZE 1024
#endif
/*
 * Reuse prefetch buffer to extract the proper hostname from the user given
 * scheme://hostname:port combination.
 */
#define MAX_HOSTNAME_LEN MAX_REQ_RESP_HDR_SIZE
/*
 * Chunk header is the data structure before each chunk. Chunked data is a
 * type of transfer encoding.
 */
#define MAX_CHUNK_HEADER_SIZE 32
#define PROTOCOL_STR "HTTP"
#define PROTOCOL_VER_1_0 "1.0"
#define PROTOCOL_VER_1_1 "1.1"
#define DEFAULT_USER_AGENT "TUYA_IOT_SDK"
#define NEXT_STR(str) (char *)(str + strlen(str) + 1)

typedef enum {
    SESSION_ALLOC,
    SESSION_IT_DONE,
    SESSION_PREPARE,
    SESSION_REQ_DONE,
    SESSION_RESP_DONE,
    SESSION_DATA__PROGRESS,
    SESSION_COMPLETE,
    SESSION_ERROR,		/* FIXME: Should we flush the socket stream
                   during close, in this state? */
    SESSION_RAW,
} session_state_t;

typedef enum {
    STATE_STANDARD,
    STATE_START_CHUNK,
    STATE_BETWEEN_CHUNK,
    STATE_END_CHUNK,
    STATE_ERROR_CHUNK,
} chunk_state_t;


/*
 * Prefetch buffer info.
 */
typedef struct {
    char *buf;
    int size_read;		/* The total data in the buffer including
                   the header */
    int offset;		/* The offset (excluding) upto which data
                   is processed */
    /* This caches the offset of name-value pairs in the http header */
    int hdr_fields_start_offset;
} prefetch_t;

typedef struct {
    chunk_state_t state;
    int size;
} chunk_t;

#define HOSTNM_MAX	255
typedef struct {
    int outstanding_requests; /* Used for interleaved mode */
    session_state_t state;
    int sockfd;		/* Socket descriptor. Not exposed to user */
    char hostname[HOSTNM_MAX + 1];	/* Hostname of connecting server */
    prefetch_t pbuf;	/* Prefetch buffer */
    http_resp_t resp;	/* The response structure. */
    chunk_t chunk;		/* For chunked transfers  */
    int content_returned;	/* content bytes returned till now */
    tuya_tls_hander tls_hander;
} session_t;

static inline int net_get_sock_error(int sock)
{
    switch (errno) {
    case EWOULDBLOCK:
        return -WM_E_AGA;
    case EBADF:
        return -WM_E_BADF;
    case ENOBUFS:
        return -WM_E_NOMEM;
    default:
        return -WM_FAIL;
    }
}

int unw_set_block(int fd,int block)
{
    if( fd < 0 ) {
        return -3000 + fd;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if(TRUE == block) {
        flags &= (~O_NONBLOCK);
    }else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd,F_SETFL,flags) < 0) {
        return -1;
    }

    return 0;
}
int unw_recv(int fd, void *buf, unsigned int nbytes)
{
    if( fd < 0 ) {
        return -3000 + fd;
    }

    return recv(fd,buf,nbytes,0);
}
typedef unsigned int  UNW_IP_ADDR_T;
typedef OPERATE_RET (*UNW_QUERY_CUSTOM_DNS_CB)(char *domain, UNW_IP_ADDR_T *addr);

#define MAX_DOMAIN_NAME_LEN 64
#define MAX_DNS_CNT         8
typedef struct
{
    char domain[MAX_DOMAIN_NAME_LEN];
    UNW_IP_ADDR_T addr;
}TY_DNS_INFO;

typedef struct
{
    pthread_mutex_t mutex;
    TY_DNS_INFO cache_arr[MAX_DNS_CNT];

    UNW_QUERY_CUSTOM_DNS_CB dns_query_cb;
}TY_DNS_MGR;

static TY_DNS_MGR s_dns_mgr = {0};

int unw_gethostbyname(char *domain, UNW_IP_ADDR_T *addr)
{
    int index = 0;
    int unused_index = -1;
    pthread_mutex_lock(&s_dns_mgr.mutex);
    for(index = 0; index < MAX_DNS_CNT; index++) {
        if(s_dns_mgr.cache_arr[index].domain[0] == 0){
            unused_index = index;
            continue;
        }
        if(strncmp(domain, s_dns_mgr.cache_arr[index].domain, strlen(s_dns_mgr.cache_arr[index].domain)) == 0) {
            *addr = s_dns_mgr.cache_arr[index].addr;
            pthread_mutex_unlock(&s_dns_mgr.mutex);
            return 0;
        }
    }

    struct hostent* h;
    if((h = gethostbyname(domain)) == NULL) {
        if(s_dns_mgr.dns_query_cb != NULL) {
            pthread_mutex_unlock(&s_dns_mgr.mutex);

            UNW_IP_ADDR_T query_addr = 0;
            OPERATE_RET ret = s_dns_mgr.dns_query_cb(domain, &query_addr);
            if(ret != OPRT_OK){
                return -2000 + ret;
            } else {
                pthread_mutex_lock(&s_dns_mgr.mutex);
                for(index = 0; index < MAX_DNS_CNT; index++) {
                    if(s_dns_mgr.cache_arr[index].domain[0] == 0){
                        unused_index = index;
                        continue;
                    }
                    if(strncmp(domain, s_dns_mgr.cache_arr[index].domain, strlen(s_dns_mgr.cache_arr[index].domain)) == 0) {
                        *addr = s_dns_mgr.cache_arr[index].addr;
                        pthread_mutex_unlock(&s_dns_mgr.mutex);
                        return 0;
                    }
                }
                *addr = query_addr;
            }

        }else {
            pthread_mutex_unlock(&s_dns_mgr.mutex);
            return -3000;
        }
    } else {
        *addr = ntohl(((struct in_addr *)(h->h_addr_list[0]))->s_addr);
    }

    if(unused_index >= 0) {
        dgb_print("store dns info to position %d for %s\n",unused_index,domain);
        s_dns_mgr.cache_arr[unused_index].addr = *addr;
        strncpy(s_dns_mgr.cache_arr[unused_index].domain, domain, MAX_DOMAIN_NAME_LEN-1);
    }
    pthread_mutex_unlock(&s_dns_mgr.mutex);
    return 0;
}

UNW_IP_ADDR_T unw_str2addr( CHAR_T *ip_str)
{
    if(ip_str == NULL)
    {
        return 0xFFFFFFFF;
    }
    UNW_IP_ADDR_T addr1 = inet_addr(ip_str);
    UNW_IP_ADDR_T addr2 = ntohl(addr1);
    // UNW_IP_ADDR_T addr3 = inet_network(ip_str);

    return addr2;
}
int unw_select(int maxfd, fd_set *readfds, fd_set *writefds,\
                fd_set *errorfds,unsigned long ms_timeout)
{
    if(maxfd <= 0) {
        return -3000 + maxfd;
    }

    struct timeval *tmp = NULL;
    struct timeval timeout = {ms_timeout/1000, (ms_timeout%1000)*1000};
    if(0 != ms_timeout) {
        tmp = &timeout;
    }else {
        tmp = NULL;
    }

    return select(maxfd,readfds,writefds,errorfds,tmp);
}

static int __select_wait_socketfd( int fd)
{
    #define HTTP_TM_S 5
    unw_set_block(fd, FALSE);
    int activefds_cnt;
    fd_set readfds;
    memset(&readfds, 0, sizeof(fd_set));
    FD_SET(fd,&readfds);
    extern int errno;
    errno=0;
    activefds_cnt = unw_select(fd + 1, &readfds,NULL, NULL, HTTP_TM_S * 1000);
    if (activefds_cnt < 0) {
        //int err = unw_get_errno();
        //printf("select fail %d err:%d", activefds_cnt, errno);
        return -WM_E_HTTPC_SOCKET_ERROR;
    }else if(0 == activefds_cnt) {
        //int err = unw_get_errno();
        //printf("http socket recv select time out. err:%d", errno);
        return -WM_E_HTTPC_SOCKET_TIME;
    }
    return 0;
}

/**
 * @internal
 *
 * @note If this function returns WM_E_HTTPC_SOCKET_ERROR then errno should
 * be read to get more information.
 */
static int _http_raw_recv(session_t *s, char *buf, int maxlen,
                          int wait_for_to)
{
    //ASSERT(maxlen != 0);
    if(0 == maxlen) {
        return -WM_E_VAL;
    }

    int rv;

    if(s->tls_hander != NULL)
    {
        rv = tuya_tls_read(s->tls_hander, (unsigned char *)buf, maxlen);
    }
    else
    {
        int select_result = __select_wait_socketfd(s->sockfd);
        if(select_result != 0)
            return select_result;

        rv = unw_recv(s->sockfd, buf, maxlen);
        unw_set_block(s->sockfd, TRUE);
    }
//        printf("recv rv:%d", rv);

    if (rv > 0)
        return rv;

    if (rv == 0)
        return -WM_E_HTTPC_SOCKET_SHUTDOWN;

//    if (rv < 0)
    return -WM_E_HTTPC_SOCKET_ERROR;
}
int unw_send(int fd, const void *buf, unsigned int nbytes)
{
  if( fd < 0 ) {
      return -3000 + fd;
  }

  return send(fd,buf,nbytes,0);
}

static int _http_raw_send(session_t *s, const char *buf, int len)
{
    if(s->tls_hander != NULL)
    {
        return tuya_tls_write(s->tls_hander, (unsigned char *)buf, len);
    }
    else
    {
        return unw_send(s->sockfd, buf, len);
    }
}


int http_write_standard(http_session_t handle, const void *buf, unsigned len)
{
    if (!handle || !buf || !len) {
        dgb_print("Cannot write standard");
        return -WM_E_VAL;
    }

    session_t *s = (session_t *)handle;
    if (s->state < SESSION_IT_DONE || s->state == SESSION_ERROR) {
        printf("Unable to do standard write");
        return -WM_E_VAL;
    }

    return _http_raw_send(s, buf, len);
}



int httpc_write_chunked(http_session_t handle, const char *data, int len)
{
    session_t *s = (session_t *) handle;
    if (!handle || (len && !data)) {
        printf("Invalid params");
        return -WM_E_VAL;
    }
    if (!(s->state == SESSION_REQ_DONE &&
          s->chunk.state == STATE_START_CHUNK))  {
        printf("Cannot write data. session state: %d,"
            " session chunk state: %d", s->state, s->chunk.state);
        return -WM_FAIL;
    }

    int post_len = sprintf(s->pbuf.buf, "%x\r\n", len);
    int sent = _http_raw_send(s, s->pbuf.buf, post_len);
    if (sent != post_len) {
        s->chunk.state = STATE_ERROR_CHUNK;
        return sent;
    }
    if (len) {
        sent = _http_raw_send(s, data, len);
        if (sent != len) {
            s->chunk.state = STATE_ERROR_CHUNK;
            return sent;
        }

        sent = _http_raw_send(s, "\r\n", 2);
        if (sent != 2) {
            s->chunk.state = STATE_ERROR_CHUNK;
            return sent;
        }
    } else {
        sent = _http_raw_send(s, "\r\n", 2);
        if (sent != 2) {
            s->chunk.state = STATE_ERROR_CHUNK;
            return sent;
        }

        s->chunk.state = STATE_END_CHUNK;
    }
    return WM_SUCCESS;
}

/**
 * @internal
 *
 * Read the stream until header delimiter is found in the read
 * blocks. Store these read octets into the prefetch buffer.
 */
static int prefetch_header(session_t *s)
{
    prefetch_t *pbuf = &s->pbuf;
    /* One less so that delimit strings */
    int max_read_size = MAX_REQ_RESP_HDR_SIZE - 1;
    int total_read_size = 0;
    int size_read;
    dgb_print("Start header read\n");
    memset(pbuf->buf, 0x00, MAX_REQ_RESP_HDR_SIZE);
    while (max_read_size) {
        int read_start_offset = total_read_size;
        size_read = _http_raw_recv(s, &pbuf->buf[read_start_offset],
                       max_read_size, true);
//		printf("Header read: %d bytes errno  = %d",
//			size_read < 0 ? get_code(size_read) : size_read,
//			errno);
        if (size_read < 0) {
            if (total_read_size != 0) {
                /* Unrecoverable condition */
                s->state = SESSION_ERROR;
            }

            return size_read;
        }

        total_read_size += size_read;
        pbuf->buf[total_read_size] = 0;
        /*
         * Check if just read data has the header end
         *  delimiter. The delimiter may have started in tail bytes
         *  of last read buffer.
         */
        int check_offset =
            read_start_offset > 4 ? read_start_offset - 4 : 0;
        if (strstr(&pbuf->buf[check_offset], "\r\n\r\n")) {
            /*
             * Even if we have found the delimiter in the read
             * buffer, the buffer may have some content octets
             * after the header. This will be read later by the
             * prefetch-buffer-aware read function.
             */
            pbuf->size_read = total_read_size;
            pbuf->offset = 0;
            return WM_SUCCESS;
        }

        max_read_size -= size_read;
    }

    dgb_print("Header not found.");
    dgb_print("The stream may be in inconsistent state or");
    dgb_print("in a rare case, the header may be bigger that %d bytes",
            MAX_CHUNK_HEADER_SIZE - 1);
    return -WM_FAIL;
}

/**
 * @internal
 * Prefetch buffer aware network read.
 *
 * All network read requests except header reads go through this
 * function. For the reads, the data not yet read from the prefetch buffer
 * will be returned. If prefetch buffer is empty then the TCP stream will
 * be read directly.
 *
 * If timeout (wait_for_to) parameter is 'true' then this function will
 * re-try read for DEF_T_MAX_TO_MS time. Note to caller: Set this
 * parameter to true if you are doing a task transparent to the httpc caller.
 */
static int recv_buf(session_t *s, char *buf, int max_read_size,
            bool_t wait_for_to)
{
    /* The header is already read */
    int r;
    prefetch_t *pbuf = &s->pbuf;

    /* Check if pre-fetched data is already read */
    if (pbuf->offset == pbuf->size_read) {
        r = _http_raw_recv(s, buf, max_read_size, wait_for_to);
        //printf("Using actual read: %d %d", r, max_read_size);
        return r;
    }

    /* There is some prefetched data left. */
    int prefetch_remaining = (pbuf->size_read - pbuf->offset);
    int to_copy = prefetch_remaining <= max_read_size ?
        prefetch_remaining : max_read_size;

    /*printf("Using prefetch: %d %d %d %d",
        pbuf->offset, prefetch_remaining, to_copy, max_read_size);*/
    memcpy(buf, &pbuf->buf[pbuf->offset], to_copy);
    pbuf->offset += to_copy;


    return to_copy;
}

/**
 * Create a socket and connect to the given host
 *
 * @param[in,out] sock Pointer to a socket variable to be filled by callee
 * @param[in] hostname Hostname or an IP address
 * @param[in] port Port number
 *
 * @return WM_SUCCESS if the socket was created successfully and -WM_FAIL
 * if failed
 */
int unw_set_timeout(const int fd,const int ms_timeout,\
                    const UNW_TRANS_TYPE_E type)
{
    if( fd < 0 ) {
        return -3000 + fd;
    }

    struct timeval timeout = {ms_timeout/1000, (ms_timeout%1000)*1000};
    int optname = ((type == TRANS_RECV) ? SO_RCVTIMEO:SO_SNDTIMEO);

    if(0 != setsockopt(fd, SOL_SOCKET, optname, (char *)&timeout, sizeof(timeout))) {
        return -1;
    }

    return 0;
}

int unw_socket_create(const UNW_PROTOCOL_TYPE type)
{
    int fd = -1;

    if(PROTOCOL_TCP == type) {
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    }else {
        fd = socket(AF_INET, SOCK_DGRAM,0);
    }

    return fd;
}

int unw_clear_dns_cache(const char *domain)
{
    if(domain == NULL)
        return OPRT_COM_ERROR;

    pthread_mutex_lock(&s_dns_mgr.mutex);
    int index = 0;
    for(index = 0; index < MAX_DNS_CNT; index++) {
        if(s_dns_mgr.cache_arr[index].domain[0] == 0)
            continue;
        if(strncmp(domain, s_dns_mgr.cache_arr[index].domain, strlen(s_dns_mgr.cache_arr[index].domain)) == 0) {
//            printf("clean dns info for %s\n",domain);
            memset(&s_dns_mgr.cache_arr[index],0,sizeof(TY_DNS_INFO));
            break;
        }
    }
    pthread_mutex_unlock(&s_dns_mgr.mutex);

    return OPRT_OK;
}
int unw_connect(const int fd,const UNW_IP_ADDR_T addr,const USHORT_T port)
{
    struct sockaddr_in sock_addr;
    USHORT_T tmp_port = port;
    UNW_IP_ADDR_T tmp_addr = addr;

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    return connect(fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
}
int unw_close(const int fd)
{
    return close(fd);
}

static inline int tcp_connect(int *sockfd, char *hostname,
                  unsigned short port, int retry_cnt)
{
    int r;
    unsigned int  hostaddr;

    if (isalpha(hostname[0]))
    {
       int result = unw_gethostbyname(hostname, &hostaddr);

       if(result != 0)
       {
           printf("No entry for host %s found. %d\n", hostname, result);
           r = result;
           goto create_socket;
       }
    }else
    {
        hostaddr = unw_str2addr(hostname);
    }

    if (!retry_cnt)
        retry_cnt = DEFAULT_RETRY_COUNT;
    while (retry_cnt--)
    {
        *sockfd = unw_socket_create(PROTOCOL_TCP );
        if (*sockfd >= 0)
            break;
        /* Wait some time to allow some sockets to get released */
        sleep(1);
    }

    if (*sockfd < 0) {
        printf("Unable to create socket");
        r =  -100 + *sockfd;
        goto create_socket;
    }

    // add http send timeout to prevent from blocking
    #define HTTP_SEND_TIME_MS 10000
    unw_set_timeout(*sockfd,HTTP_SEND_TIME_MS,TRANS_SEND);

#define CONFIG_LWIP_STACK
#ifndef CONFIG_LWIP_STACK
#error 如果打开需要使用uni_network.c中的函数
    int opt;
    /* Limit Send/Receive buffer size of socket to 2KB */
    opt = 2048;
    if (setsockopt(*sockfd, SOL_SOCKET, SO_SNDBUF,
               (char *)&opt, sizeof(opt)) == -1) {
        printf("Unsupported option SO_SNDBUF: %d\n",
            net_get_sock_error(*sockfd));
    }

    opt = 2048;
    if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVBUF,
               (char *)&opt, sizeof(opt)) == -1) {
        printf("Unsupported option SO_RCVBUF: %d",
            net_get_sock_error(*sockfd));
    }
#endif /* CONFIG_LWIP_STACK */

#ifdef HTTPC_TCP_NODELAY
#error 如果打开需要使用uni_network.c中的函数
    {
        int on = 1;
        socklen_t len = sizeof(on);
        int res =
            setsockopt(*sockfd, IPPROTO_TCP, TCP_NODELAY, &on, len);
        if (res < 0) {
            printf("setsockopt TCP_NODELAY failed\n");
            net_close(*sockfd);
            *sockfd = 0;
            return -WM_FAIL;
        }
    }
#endif /* HTTPC_TCP_NODELAY */
    r = WM_SUCCESS;
    goto create_socket;

create_socket:

    if (r != WM_SUCCESS) {
        unw_clear_dns_cache(hostname);
        printf("Socket creation for %s:%d failed\n", hostname, port);
        return r;
    }

    dgb_print("Connecting .. %s:%d", hostname, port);

    if (unw_connect(*sockfd, hostaddr, port) != 0)
    {
        printf("tcp connect failed %s:%d errno=%d\n", hostname, port,
            errno);
        net_close(*sockfd);
        *sockfd = 0;
        unw_clear_dns_cache(hostname);
        return -WM_E_HTTPC_TCP_CONNECT_FAIL;
    }

    dgb_print("Connected .. sockfd: %d", *sockfd);
    return WM_SUCCESS;
}

static inline session_t *new_session_object()
{
    session_t *s = malloc(sizeof(session_t));
    if (!s) {
        printf("Could not allocate session object\n");
        return NULL;
    }

    memset(s, 0x00, sizeof(session_t));
    s->pbuf.buf = malloc(MAX_REQ_RESP_HDR_SIZE);
    if (!s->pbuf.buf) {
        printf("Could not allocate prefetch buffer\n");
        free(s);
        return NULL;
    }

    s->state = SESSION_ALLOC;
    return s;
}

static inline void delete_session_object(session_t *s)
{
    if(s->tls_hander != NULL)
    {
        tuya_tls_disconnect(s->tls_hander);
        s->tls_hander = NULL;
    }

    if (s->pbuf.buf)
    {
        free(s->pbuf.buf);
        s->pbuf.buf = NULL;
    }
    free(s);
}

static int _http_parse_URL(char *URL, parsed_url_t *parsed_url)
{
    char *pos = URL;
    const char *r;
    char *token;
    bool_t found_portno = false;

//	printf("Parsing: %s", URL);

    parsed_url->portno = DEFAULT_HTTP_PORT;

    /* Check for the scheme */
    token = strstr(pos, "://");
    if (token) {
        *token = 0;
        dgb_print("Scheme: %s", pos);
        parsed_url->scheme = pos;
        pos = token + 3;
    } else {
        dgb_print("No scheme in given URL");
        parsed_url->scheme = NULL;
    }

    parsed_url->hostname = pos;

    // add by nzy 20150716 防止解析端口时与JSON格式中的":"冲突
    int tmp = 0;
    char *tmp_token = strchr(pos, '/');
    if(tmp_token) {
        *tmp_token = 0;
        tmp = 1;
    }

    /* Check for the port number */
    token = strchr(pos, ':');

    // add by nzy 20150716 数据恢复
    if(tmp) {
        *tmp_token = '/';
    }

    if (token) {
        *token = 0;
        dgb_print("Parse: Hostname: %s", pos);
        /* Skip the ':' */
        token++;
        /*
         * Set r to start of port string so that we can search for
         * start of resource later.
         */
        r = token;
        if (sscanf(token, "%u", &parsed_url->portno) != 1) {
            dgb_print("Port number parsing failed: %s", token);
            return -WM_FAIL;
        }

        found_portno = true;
        dgb_print("Parse: Port No: %d", parsed_url->portno);
    } else {
        dgb_print("No port number given");
        /*
         * No port number given. So have to search from start of
         * hostname for resource start
         */
        r = parsed_url->hostname;
    }

    /* Check for the resource */
    token = strchr(r, '/');
    if (token) {
        /*
         * Resource is found. We have to move the resource string
         * (including the NULL termination) to he right by one byte
         * to NULL terminate the hostname string. However, this can
         * be avoided if the hostname was postfixed by the port no.
         */
        if (!found_portno) {
            memmove(token + 1, token, strlen(token) + 1);
            /* NULL terminate the hostname */
            *token = 0;
            token++;
        } else
            *(token - 1) = 0;

        /* point token to the resource */
        dgb_print("Resource: %s", token);
        parsed_url->resource = token;
    } else {
        dgb_print("No resource specified in given URL");
        parsed_url->resource = NULL;
    }

    return WM_SUCCESS;
}

int http_parse_URL(const char *URL, char *tmp_buf, int tmp_buf_len,
           parsed_url_t *parsed_url)
{
    if (!URL || !tmp_buf || !parsed_url) {
        printf("Cannot parse URL\n");
        printf("NULL pointer(s) received as argument.\n");
        return -WM_E_VAL;
    }

    int min_size_req = strlen(URL) + PARSE_EXTRA_BYTES;
    if (min_size_req > tmp_buf_len) {
        printf("Cannot parse URL\n");
        printf("Temporary buffer size is less than required\n");
        return -WM_E_VAL;
    }

    strncpy(tmp_buf, URL, tmp_buf_len);
    return _http_parse_URL(tmp_buf, parsed_url);
}

static int __tuya_tls_http_send_cb( void *ctx, const unsigned char *buf, size_t len )
{
    session_t *s = (session_t *)ctx;
    return unw_send(s->sockfd, buf, len);
}
static int __tuya_tls_http_recv_cb( void *ctx, unsigned char *buf, size_t len )
{
    session_t *s = (session_t *)ctx;
    int select_result = __select_wait_socketfd(s->sockfd);
    if(select_result != 0)
        return select_result;

    int err_cnt = 0;
    extern int errno;
    errno = 0;
    while(err_cnt < 20) {
        int result = unw_recv(s->sockfd, buf, len);
        if(result < 0) {
            
            //UNW_ERRNO_T err = unw_get_errno();
            if(errno == ENOMEM) {
                err_cnt++;
                dgb_print("tls http recv no memory.len:%d ret:%d err:%d. try recv again %d", len, result, errno, err_cnt);
                usleep(100000);
                continue;
            }else if(errno == EAGAIN ) {
                err_cnt++;
                dgb_print("tls http recv again.len:%d ret:%d err:%d. try recv again %d", len, result, errno, err_cnt);
                sleep(10);
                continue;
            } else
            {
                unw_set_block(s->sockfd, TRUE);
                dgb_print("tls http recv fail.len:%d ret:%d", len, result);
                return result;
            }
        } else {
            unw_set_block(s->sockfd, TRUE);
            err_cnt = 0;
            return result;
        }
    }
    unw_set_block(s->sockfd, TRUE);
    //UNW_ERRNO_T err = unw_get_errno();
    dgb_print("tls http recv fail.len:%d", len);
    return -1;
}

int ty_http_open_session(http_session_t *handle,const char *hostname, \
                      int flags,int retry_cnt)
{
    dgb_print("hostname: %s\n", hostname);
    int r;
    session_t *s;

    if (!handle || !hostname) {
        printf("Cannot create session.\n");
        printf("NULL pointer(s) received as argument.\n");
        return -WM_E_VAL;
    }

    s = new_session_object();
    if (!s)
        return -WM_FAIL;

    if (strlen(hostname) >= MAX_HOSTNAME_LEN) {
        printf("Host name corrupt\n");
        delete_session_object(s);
        return -WM_FAIL;
    }

    parsed_url_t url;
    r = http_parse_URL(hostname, s->pbuf.buf, MAX_REQ_RESP_HDR_SIZE, &url);
    if (r != WM_SUCCESS) {
        delete_session_object(s);
        return r;
    }

    if (url.scheme && strcmp(url.scheme, "https") == 0) {
        dgb_print("Force enable TLS flag\n");
        flags |= TLS_ENABLE;
        if (url.portno == DEFAULT_HTTP_PORT)
            url.portno = DEFAULT_HTTPS_PORT;
    }

    dgb_print("Connect: %s Port: %d  -->>\n", url.hostname, url.portno);
    r = tcp_connect(&s->sockfd, (char *)url.hostname, url.portno, retry_cnt);
    dgb_print("Connect: %s Port: %d  --<<\n", url.hostname, url.portno);

    if (r != WM_SUCCESS) {
        delete_session_object(s);
        *handle = 0;
        return -200000 + r;
    }


    if(flags & TLS_ENABLE)
    {
        int ret = tuya_tls_connect(&(s->tls_hander), (char *)url.hostname, url.portno, 1,
                                         s, __tuya_tls_http_send_cb, __tuya_tls_http_recv_cb,
                                         s->sockfd, 10);
        if(ret != OPRT_OK){
            tuya_tls_disconnect(s->tls_hander);
            s->tls_hander = NULL;

            net_close(s->sockfd);
            delete_session_object(s);
            *handle = 0;
            return -300000 + ret;
        }
    }

    /* Save parsed hostname in session structure. This is required to be
       filled into request header later */
    strncpy(s->hostname, url.hostname, HOSTNM_MAX);
    *handle = (http_session_t) s;
    s->state = SESSION_IT_DONE;

    return r;
}

static const char *get_version_string(http_ver_t version)
{
    switch (version) {
    case HTTP_VER_1_0:
        return (const char *)PROTOCOL_VER_1_0;
    case HTTP_VER_1_1:
        return (const char *)PROTOCOL_VER_1_1;
    default:
        printf("Unknown version given\n");
        return NULL;
    }
}

/**
 * @return On success returns string corresponding to the type. On error,
 * returns NULL .
 */
static const char *get_method_string(http_method_t type)
{
    switch (type) {
    case HTTP_GET:		/* retrieve information */
        return (const char *)"GET";
    case HTTP_POST:	/* request to accept new sub-ordinate of resource */
        return (const char *)"POST";
    case HTTP_HEAD:	/* get meta-info */
        return (const char *)"HEAD";
    case HTTP_PUT:		/* modify or create new resource referred
                   to by URI */
        return (const char *)"PUT";
    case HTTP_OPTIONS:	/* request to server for communication
                   options */
    case HTTP_DELETE:	/* delete the resource */
    case HTTP_TRACE:	/* echo */
    case HTTP_CONNECT:	/* do we need this  ? */
        printf("Method not yet supported.\n");
        return NULL;
    default:
        printf("Unknown method given.\n");
    }

    return NULL;
}

static unsigned int s_range_start = -1;
static unsigned int s_range_end = -1;
static char range_buf[32] = {0};
static const char *get_range_string(unsigned int start,  unsigned int size)
{
    s_range_start = start;
    s_range_end = start + size - 1;
    memset(range_buf, 0, 32);
    snprintf(range_buf, 32, "bytes=%u-%u", s_range_start, s_range_end);
    return range_buf;
}



static inline int _http_add_header(session_t *s, const char *name,
                   const char *value)
{
    if (value && (strlen(s->pbuf.buf) + strlen(name) + strlen(value) + 4) <
        MAX_REQ_RESP_HDR_SIZE) {
        strcat(s->pbuf.buf, name);
        strcat(s->pbuf.buf, ": ");
        strcat(s->pbuf.buf, value);
        strcat(s->pbuf.buf, "\r\n");
        return WM_SUCCESS;
    }
    return -WM_FAIL;
}

int http_add_header(http_session_t handle,
            const http_req_t *req __attribute__((unused)),
            const char *name, const char *value)
{
    dgb_print("Enter: %s", __func__);
    session_t *s = (session_t *) handle;

    if (!handle || !name || !value || s->state != SESSION_PREPARE) {
        printf("Cannot add header\n");
        return -WM_E_VAL;
    }

    return _http_add_header(s, name, value);
}

static const char *sanitize_resource_name(session_t *s, const char *resource)
{
    parsed_url_t url;
    int r;
    const char *default_resource = "/";

    if (!resource)
        return default_resource;

    /* Check if the resource string starts with a '/' */
    if (resource[0] != default_resource[0]) {
        dgb_print("Have to extract\n\r");
        /* The resource string is either a valid URL or just garbage */
        r = http_parse_URL(resource, s->pbuf.buf, MAX_REQ_RESP_HDR_SIZE,
                   &url);
        if (r != WM_SUCCESS) {
            printf("Resource extraction error: %d\n", r);
            return NULL;
        }
        if (!url.resource)
            return default_resource;
        else
            return url.resource;
    } else
        return resource;
}

static inline int _http_send_request(session_t *s, const http_req_t *req, int send_content)
{
    /* Complete the header */
    strcat(s->pbuf.buf, "\r\n");
    dgb_print("send header:%s ", s->pbuf.buf);
    /* Send the header on the network */
    int to_send = strlen(s->pbuf.buf);
    int sent = _http_raw_send(s, s->pbuf.buf, to_send);
    if (sent != to_send) {
        printf("Failed to send header\n");
        return -WM_E_IO;
    }

    unsigned int content_len = 0;
    if(req->p_custom_content_ctx != NULL)
        content_len = req->p_custom_content_ctx->get_content_len_cb( req->p_custom_content_ctx->pri_data );
    else
        content_len = req->content_len;

    dgb_print("content len:%u", content_len);
    if(send_content != 0) {
        /* Send the data if this was POST/PUT request */
        if (((req->type == HTTP_POST)||(req->type == HTTP_PUT)) && content_len) {
            if(req->p_custom_content_ctx != NULL) {
                http_custom_content_ctx_s *p_ctx = req->p_custom_content_ctx;
                unsigned int pre_malloc_size = 0;
                int result = p_ctx->before_read_cb(p_ctx->pri_data, &pre_malloc_size);
                if((result != 0) || (pre_malloc_size == 0)) {
                    printf("custom content before read cb fail. %d %u\n", result, pre_malloc_size);
                    return -WM_E_IO;
                }
                dgb_print("custom content. before read cb %d %u", result, pre_malloc_size);
                unsigned char *p_buf = (unsigned char *)malloc(pre_malloc_size);
                if(p_buf == NULL) {
                    printf("malloc fail. %u\n", pre_malloc_size);
                    return -WM_E_IO;
                }
                unsigned int send_size = 0;
                while(send_size < content_len) {
                    int read_cnt = p_ctx->read_content_cb(p_buf, pre_malloc_size, p_ctx->pri_data);
                    if(read_cnt <= 0) {
                        printf("custom content. read content cb read_cnt:%d %u .read fail\n", read_cnt, pre_malloc_size);
                        break;
                    }
                    dgb_print("custom content. read content cb read_cnt:%d %u\n", read_cnt, pre_malloc_size);
                    sent = _http_raw_send(s, (char *)p_buf, read_cnt);
                    if (sent != read_cnt) {
                        printf("_http_raw_send Failed to send data. %d %d\n", sent, read_cnt);
                        break;
                    }
                    send_size += sent;
                }
                free(p_buf);

                result = p_ctx->after_read_cb(p_ctx->pri_data);
                if(result != 0)
                    printf("after read cb fail. %d， continue...\n", result);

                if(send_size != content_len) {
                    printf("custom content send fail. %u %d\n", send_size, content_len);
                    return -WM_E_IO;
                } else {
                    printf("custom content send finish %u %d\n", send_size, content_len);
                }

            } else {
                sent = _http_raw_send(s, req->content, req->content_len);
                if (sent != req->content_len) {
                    printf("Failed to send data.\n");
                    return -WM_E_IO;
                }
            }
        }
    }

    s->outstanding_requests++;
    return WM_SUCCESS;
}

static int _http_prepare_req(session_t *s, const http_req_t *req,
                 http_hdr_field_sel_t field_flags)
{
    const char *method, *version, *resource;
    method = get_method_string(req->type);
    if (!method) {
        printf("HTTPC: Method invalid\n");
        s->state = SESSION_ERROR;
        return -WM_E_VAL;
    }

    version = get_version_string(req->version);
    if (!version) {
        printf("HTTPC: version invalid\n");
        s->state = SESSION_ERROR;
        return -WM_E_VAL;
    }

    resource = sanitize_resource_name(s, req->resource);
    if (!resource) {
        printf("HTTPC: resource invalid\n");
        s->state = SESSION_ERROR;
        return -WM_E_VAL;
    }

    /* Start generating the header */
    sprintf(s->pbuf.buf, "%s %s %s/%s\r\n", method, resource,
        PROTOCOL_STR, version);

    /* Fill in saved hostname */
    _http_add_header(s, "Host", s->hostname);

    if (field_flags & HDR_ADD_DEFAULT_USER_AGENT)
        _http_add_header(s, "User-Agent", DEFAULT_USER_AGENT);

    if (field_flags & HDR_ADD_CONN_KEEP_ALIVE)
        _http_add_header(s, "Connection", "keep-alive");

    if (field_flags & HDR_ADD_CONN_CLOSE) {
        //ASSERT(!(field_flags & HDR_ADD_CONN_KEEP_ALIVE));
        if((field_flags & HDR_ADD_CONN_KEEP_ALIVE)) {
            return -WM_FAIL;
        }
        _http_add_header(s, "Connection", "close");
    }
    if (field_flags & HDR_ADD_CONTENT_TYPE_JSON) {
        _http_add_header(s, "Content-Type", "application/json");
    }else if(field_flags & HDR_ADD_CONTENT_TYPE_FORM_URLENCODE) {
        _http_add_header(s, "Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    }

    if (field_flags & HDR_ADD_TYPE_CHUNKED) {
        _http_add_header(s, "Transfer-Encoding", "chunked");
        s->chunk.state = STATE_START_CHUNK;
    } else if ((req->type == HTTP_POST)|| (req->type == HTTP_PUT)) {
        /* If chunked encoding is used for sending data,
         * Content-Length is not added in header
         */

        int cur_pos = strlen(s->pbuf.buf);

        unsigned int content_len = 0;
        if(req->p_custom_content_ctx != NULL)
            content_len = req->p_custom_content_ctx->get_content_len_cb( req->p_custom_content_ctx->pri_data );
        else
            content_len = req->content_len;

        if(content_len == 0) {
            dgb_print("HTTPC: content len is 0");
            s->state = SESSION_ERROR;
            return -WM_E_VAL;
        }
        sprintf(&s->pbuf.buf[cur_pos], "Content-Length: %d\r\n",
            content_len);
    }

    if (field_flags & HRD_ADD_DOWNLOAD_RANGE)
    {
        _http_add_header(s, "Range", get_range_string(req->download_offset, req->download_size));
    }
    if (req->user_header != NULL)
    {
        strcat(s->pbuf.buf,req->user_header);
        strcat(s->pbuf.buf,"\r\n");
    }

    s->state = SESSION_PREPARE;
    if(req->add_head_cb != NULL)
    {
        req->add_head_cb(s, req->add_head_data);
    }

    dgb_print("Send****************************\n%s******** http end******\n ", s->pbuf.buf);
    return WM_SUCCESS;
}

int ty_http_prepare_req(http_session_t handle, const http_req_t *req,
             http_hdr_field_sel_t field_flags)
{
    session_t *s = (session_t *) handle;

    if (!handle || !req) {
        printf("Cannot prepare request, invalid parameters.\n"
            " State: %d", s->state);
        return -WM_E_VAL;
    }
    if (s->state != SESSION_IT_DONE &&
        s->state != SESSION_REQ_DONE)  {
        printf("Cannot prepare request, invalid session state.\n"
            " State: %d", s->state);
        return -WM_E_VAL;
    }
    /*
     * We are here means either this API was called:
     * - directly after request was done (Interleaved Requests)
     *   OR
     * - after last API called was read content
     */
    memset(&s->chunk, 0x00, sizeof(chunk_t));
    return _http_prepare_req(s, req, field_flags);
}


int ty_http_send_request(http_session_t handle, const http_req_t *req, int send_content)
{
    session_t *s = (session_t *) handle;

    if (!req || !handle || s->state != SESSION_PREPARE) {
        printf("Cannot send request\n");
        return -WM_E_VAL;
    }

    s->pbuf.size_read = 0;
    s->pbuf.offset = 0;
    s->pbuf.hdr_fields_start_offset = -1;
    memset(&s->resp, 0x00, sizeof(http_resp_t));
    s->content_returned = 0;

    if(send_content != 0) {
        if (req->type == HTTP_POST && !req->content && req->content_len) {
            printf("Cannot send request. Buffer empty.\n");
            return -WM_E_VAL;
        }

        if (req->type == HTTP_PUT && !req->content && req->content_len) {
            printf("Cannot send request. Buffer empty.\n");
            return -WM_E_VAL;
        }
    }

    int r = _http_send_request(s, req, send_content);
    if (r == WM_SUCCESS) {
        s->state = SESSION_REQ_DONE;
        return r;
    }

    s->state = SESSION_ERROR;
    return r;
}

static void parse_keep_alive_header(const char *value, http_resp_t *resp)
{
    int ret;
    if (!strstr(value, "timeout")) {
        printf("No timeout specified in response header.\n");
        printf("NAK'ing keep alive by force\n");
        resp->keep_alive_ack = false;
        return;
    }

    /* The header has a timeout value spec */
    ret = sscanf(value, "timeout=%d", &resp->keep_alive_timeout);
    if (ret <= 0) {
        printf("timeout value not found !\n");
        resp->keep_alive_ack = false;
        return;
    }

    //printf("Server timeout value: %d", resp->keep_alive_timeout);
    value = strstr(value, "max");
    if (!value) {
        printf("max value not found !\n");
        resp->keep_alive_ack = false;
        return;
    }

    ret = sscanf(value, "max=%d", &resp->keep_alive_max);
    if (ret <= 0) {
        printf("max value invalid !\n");
        resp->keep_alive_ack = false;
        return;
    }

    //printf("Alive Max value:%d", resp->keep_alive_max);
    resp->keep_alive_ack = true;
}

/**
 * The parameter off is updated with the new offset pointing at the data,
 * if any.
 * 'len' is the amount of valid data present in the buffer
 *
 * @return On success, i.e. if the header end was found WM_SUCCESS will be
 * returned. On error -WM_FAIL will be returned.
 */
static int load_header_fields(char *header, int len, int *off,
                  http_resp_t *resp)
{
    int offset = *off;
    bool_t header_done = false;
    char *substr;
    char *pair, *name, *value;

    while (offset < len) {
        if (header[offset] == '\r' && header[offset + 1] == '\n') {
            /* Point to the start of data in case of standard
               read or start of first chunk header in case of
               chunked read */
            offset += 2;
            header_done = true;
            break;
        }
        /* FIXME: can this happen ? */
        //ASSERT(header[offset] != '\r' && header[offset + 1] != '\n');
        if(header[offset] == '\r' || header[offset + 1] == '\n') {
            return -WM_FAIL;
        }


        /* Extract the name-value pair */
        pair = &header[offset];
        substr = strstr(pair, "\r\n");
        if (!substr) {
            printf("Header parsing error in pair %s\n", pair);
            return -WM_FAIL;
        }
        *substr = 0;

        int pair_len = strlen(pair);
        if (((pair + pair_len) - header) > len) {
            dgb_print
                ("Hdr parsing over bound:len: %d pair_len: %d",
                 len, pair_len);
            dgb_print("#### %s", pair);
            return -WM_FAIL;
        }

        /* Go past the header: value and the header boundary '\r\n' */
        offset += pair_len + 2;

        /* Separate name-value */
        name = pair;
        substr = strstr(name, ": ");
        if (!substr) {
            printf("Header parsing error in name %s\n", pair);
            return -WM_FAIL;
        }
        *substr = 0;

        /* Skip the name string, the delimited :
           and the space after that */
        value = name + strlen(name) + 2;

        if (!strcasecmp(name, "Accept-Ranges"))
            resp->p_accept_ranges = value;
        if (!strcasecmp(name, "Server"))
            resp->server = value;
        if (!strcasecmp(name, "Last-Modified"))
            resp->modify_time = time((time_t *)NULL);
        if (!strcasecmp(name, "Content-Type"))
            resp->content_type = value;
        if (!strcasecmp(name, "Connection")) {
            if (!strcasecmp(value, "Keep-Alive")) {
                dgb_print("Server allows keep alive");
                resp->keep_alive_ack = true;
            } else {
                if (resp->keep_alive_ack == true) {
                    /*printf("Keep-Alive present but " 
                        "connection: Close ?");*/
                }
                dgb_print("Server does not allow keep alive");
                resp->keep_alive_ack = false;
            }
        }
        if (!strcasecmp(name, "Keep-Alive"))
            parse_keep_alive_header(value, resp);
        if (!strcasecmp(name, "Content-Length"))
            resp->content_length = strtol(value, NULL, 10);
        if (!strcasecmp(name, "Content-Encoding"))
            resp->content_encoding = value;
        if (!strcasecmp(name, "Transfer-Encoding"))
            if (!strcasecmp(value, "chunked"))
                resp->chunked = true;
    }

    if (header_done) {
        *off = offset;
        return WM_SUCCESS;
    } else
        return -WM_FAIL;
}

/**
 * @param[out] data_offset Points to start of data after last byte of header (if any)
 */
static inline int _parse_http_header(char *header, int len,
                     http_resp_t *resp,
                     int *hdr_fields_start_offset,
                     int *data_offset)
{
    int offset = 0;
    char *http_ver = NULL, *status_str = NULL, *substr;
    char *str = header;
    char *delim;

    //printf("Parsing HTTP header");
    //printf("header:%s", str);
    /* Get the status line */
    delim = strchr(str, '/');
    if (delim) {
        *delim = 0;
        resp->protocol = str;
    }
    if (!resp->protocol || strcasecmp(resp->protocol, PROTOCOL_STR)) {
        printf("Protocol mismatch in header.\n");
        return -WM_FAIL;
    }

    str = NEXT_STR(resp->protocol);
    delim = strchr(str, ' ');
    if (delim) {
        *delim = 0;
        http_ver = str;
    }
    if (!http_ver) {
        printf("Protocol version not found.\n");
        return -WM_FAIL;
    } else {
        if (!strcasecmp(http_ver, PROTOCOL_VER_1_0))
            resp->version = HTTP_VER_1_0;
        else if (!strcasecmp(http_ver, PROTOCOL_VER_1_1)) {
            resp->version = HTTP_VER_1_1;
            /*
             * Since it is HTTP 1.1 we will default to
             * persistent connection. If there is explicit
             * "Connection: Close" in the header, this
             * variable to be reset later to false
             * automatically.
             */
            //printf("Defaulting to keep alive");
            resp->keep_alive_ack = true;
        } else {
            printf("Protocol version mismatch.\n");
            return -WM_FAIL;
        }
    }

    str = NEXT_STR(http_ver);
    delim = strchr(str, ' ');
    if (delim) {
        *delim = 0;
        status_str = str;
    }
    if (!status_str) {
        printf("Status code not found.\n");
        return -WM_FAIL;
    }

    resp->status_code = strtol(status_str, NULL, 10);
    str = NEXT_STR(status_str);
    substr = strstr(str, "\r\n");
    if (substr) {
        *substr = 0;
        resp->reason_phrase = str;
    } else {
        printf("Status code string not found.\n");
        return -WM_FAIL;
    }

    offset = (NEXT_STR(resp->reason_phrase) + 1 - header);
    *hdr_fields_start_offset = offset;
    load_header_fields(header, len, &offset, resp);
    /*      if (len > offset) { */
    /* buffer has more data to be  processed later */
    *data_offset = offset;
    /*} */

    return WM_SUCCESS;
}

/*
 * Reset variables and close the transaction.
 *
 * @pre Response header is read
 */
static inline void http_close_transaction(session_t *s)
{
    s->outstanding_requests--;
    /* Reset returned content length */
    s->content_returned = 0;
    if (s->resp.keep_alive_ack)
        s->state = SESSION_IT_DONE;
    else
        s->state = SESSION_COMPLETE;
}

int ty_http_get_response_hdr(http_session_t handle, http_resp_t **resp)
{
    session_t *s = (session_t *) handle;
    if (!handle) {
        printf("Unable to get resp header, invalid params."
            " State: %d\n", s->state);
        return -WM_E_VAL;
    }
    if (s->state != SESSION_REQ_DONE &&
        !(s->state == SESSION_IT_DONE &&
          s->outstanding_requests > 0)) {
        printf("Unable to get resp header, invalid session state."
        " State: %d\n", s->state);
        return -WM_E_VAL;
    }

    /*
     * We are here means either this API was called:
     * - directly after request was done
     *   OR
     * - after last API called was read content (Interleaved mode)
     */

    /*
     * We read even past the header data. The data after the header
     * will be returned to the caller in next call to http_read_content.
     */
    int r = prefetch_header(s);

    dgb_print("Respone:%s ty_http_get_response_hdr \n", s->pbuf.buf);

    if (r != WM_SUCCESS)
        return r;

    r = _parse_http_header(s->pbuf.buf, s->pbuf.size_read, &s->resp,
                   &s->pbuf.hdr_fields_start_offset,
                   &s->pbuf.offset);
    if (r == WM_SUCCESS) {
        dgb_print("Header has used up %d bytes from the %d bytes header\n",
            s->pbuf.offset, s->pbuf.size_read);
        s->state = SESSION_RESP_DONE;

        if ((s->resp.content_length == 0) &&
            !s->resp.chunked) {
            dgb_print("No content present. Closing this x'action");
            /* Non-chunked transaction with content length
               zero. For chunked transfers 's->content_length'
               variable is always zero.
             */
            http_close_transaction(s);
        }

        if (resp)
            *resp = &s->resp;
        /* Set the chunk state to start if the transfer encoding is
         * given as chunked  */
        if (s->resp.chunked) {
            s->chunk.state = STATE_START_CHUNK;
        }
    } else {
        s->state = SESSION_ERROR;
    }

    return r;
}

/* A function very dependant on the design of httpc. It checks for
   '\0\n'. Note that the pair length returned, does not include the NULL
   terminating character. Also note that this function totally assumes that
   the name and value both are NULL terminated already. Basically should
   work because the header as been already parsed once.*/
static inline int get_header_pair(char *buf, int maxlen, int *pair_len,
                  char **name, char **value)
{
    if (buf[0] == '\0')
        return -WM_FAIL;

    bool_t name_terminator_found = false;
    char *name_term_address;

    /* We are right now at the start of a name-value pair */
    int i;
    for (i = 1; i < maxlen; i++) {
        if (buf[i] == '\0') {
            if (!name_terminator_found) {
                name_terminator_found = true;
                name_term_address = &buf[i];
                continue;
            }

            if ((i + 1) < maxlen && buf[i + 1] == '\n') {
                if ((&buf[i] - name_term_address) < 3) {
                    /* Sanity check */
                    return -WM_FAIL;
                }
                *name = buf;
                *value = name_term_address + 2;
                *pair_len = i;
                return WM_SUCCESS;
            }
        }
    }

    return -WM_FAIL;
}

static int _http_get_response_hdr_value(session_t *s,
                    const char *header_name,
                    char **value,
                    http_header_pair_t *arr, int *count)
{
    /* We should already know the offset from where the name-value
       pairs in the http header start */
    //ASSERT(s->pbuf.hdr_fields_start_offset != -1);
    if(s->pbuf.hdr_fields_start_offset == -1) {
        return -WM_FAIL;
    }

    /* Create a local copy as this function may be called more than
       once */
    int offset = s->pbuf.hdr_fields_start_offset;
    /* This is the total size of the header in the buffer */
    const int len = s->pbuf.size_read;
    char *header = s->pbuf.buf;
    int arr_index = 0;
    while (offset < len) {
        if (header[offset] == '\r' && header[offset + 1] == '\n') {
            /* Point to the start of data in case of standard
               read or start of first chunk header in case of
               chunked read */
            offset += 2;
            break;
        }
        /* FIXME: can this happen ? */
        // ASSERT(header[offset] != '\r' && header[offset + 1] != '\n');
        if(header[offset] == '\r' || header[offset + 1] == '\n') {
            return -WM_FAIL;
        }

        int pair_len;
        char *name, *val;
        int rv = get_header_pair(&header[offset], len - offset,
                     &pair_len, &name, &val);
        if (rv != WM_SUCCESS) {
            printf("Error in parsing header: %d\n", rv);
            return rv;
        }
        if (header_name == NULL) {
            /* User has asked us for the entire list */
            arr[arr_index].name = name;
            arr[arr_index].value = val;
            arr_index++;
            if (arr_index == *count)
                break;
            /* Go past the header: value and the header boundary
               '\0\n' */
            offset += pair_len + 2;
            continue;
        }

        if (strncmp(name, header_name, len - offset)) {
            /* This is not the header caller needs */

            /* Go past the header: value and the header boundary
               '\0\n' */
            offset += pair_len + 2;
            continue;
        }

        *value = val;
        return WM_SUCCESS;
    }

    /* Comes here only if complete list is required */
    /* Sanity check for single name value requested */
    if (header_name != NULL)
        return -WM_FAIL;

    *count = arr_index;
    return WM_SUCCESS;
}

int http_get_response_hdr_value(http_session_t handle,
                const char *header_name, char **value)
{
    session_t *s = (session_t *) handle;
    if (!handle) {
        printf("Unable to get resp header.\n");
        return -WM_E_VAL;
    }

    switch (s->state) {
    case SESSION_RESP_DONE:
    case SESSION_REQ_DONE:
    case SESSION_IT_DONE:
        if (s->outstanding_requests > 0)
            break;
    default:
        printf("Invalid state: %d. Cannot read hdr value\n", s->state);
        return -WM_E_VAL;
    }

    if (s->state == SESSION_REQ_DONE ||  s->state == SESSION_IT_DONE) {
        /*
         * The caller has not called ty_http_get_response_hdr. We need
         * to do it.
         */
        int status = ty_http_get_response_hdr(handle, NULL);
        if (status != WM_SUCCESS)
            return status;
    }

    /* Failure here is not a session error */
    return _http_get_response_hdr_value(s, header_name, value,
                        NULL, NULL);
}


int http_get_response_hdr_all(http_session_t handle, http_header_pair_t *arr,
                  int *count)
{
    session_t *s = (session_t *) handle;
    if (!handle || !arr || !count) {
        printf("Unable to get resp header.\n");
        return -WM_E_VAL;
    }

    switch (s->state) {
    case SESSION_RESP_DONE:
    case SESSION_REQ_DONE:
    case SESSION_IT_DONE:
        if (s->outstanding_requests > 0)
            break;
    default:
        printf("Invalid state: %d. Cannot read hdr value\n", s->state);
        return -WM_E_VAL;
    }

    if (s->state == SESSION_REQ_DONE || s->state == SESSION_IT_DONE) {
        /*
         * The caller has not called ty_http_get_response_hdr. We need
         * to do it.
         */
        int status = ty_http_get_response_hdr(handle, NULL);
        if (status != WM_SUCCESS)
            return status;
    }

    /* Failure here is not a session error */
    if (*count == 0)
        return WM_SUCCESS;
    return _http_get_response_hdr_value(s, NULL, NULL, arr, count);
}

/**
 * @return On success, returns the amount of data read. On error, return -WM_FAIL
 */
static int http_read_standard(session_t *s, void *buf, unsigned int max_len)
{
    if (s->resp.content_length == s->content_returned) {
        dgb_print("Standard read complete");
        http_close_transaction(s);
        return 0;
    }

    int content_len = s->resp.content_length;
    int size_remaining = content_len - s->content_returned;
    int size_to_read = size_remaining <= max_len ? size_remaining : max_len;

    //printf("contentlen:%d size_remain:%d size_to_read:%d", content_len, size_remaining, size_to_read);
    int size_read = recv_buf(s, buf, size_to_read, false);
    if (size_read < 0) {
        if (
            (size_read == -WM_E_HTTPC_SOCKET_ERROR &&
             errno != EAGAIN ) ||
            (size_read == -WM_E_HTTPC_SOCKET_SHUTDOWN)) {
            printf("Error in reading content: %d\n", size_read);
            s->state = SESSION_ERROR;
        }

        return size_read;
    }
    s->content_returned += size_read;
    s->state = SESSION_DATA__PROGRESS;
    return size_read;
}

/**
 * @pre The buffer should point to the start of the chunk header.
 *
 * @return On success, i.e. if the data size was found correctly. If data
 * size was not found then -WM_FAIL is returned.
 */
static int get_content_len(session_t *s)
{
    char chunk_header[MAX_CHUNK_HEADER_SIZE];
    bool_t first_delim_found = false;
    int i;
    char c;
    for (i = 0; i < MAX_CHUNK_HEADER_SIZE; i++) {
        int size_read = recv_buf(s, &c, 1, true);
        if (size_read < 0) {
            printf("Error while reading chunk len: %d\n",
                size_read);
            return size_read;
        }

        chunk_header[i] = c;
        if (first_delim_found) {
            if (chunk_header[i] != '\n') {
                printf("Unable to read len. No newline\n");
                return -WM_FAIL;
            }
            break;
        }
        if (chunk_header[i] == '\r')
            first_delim_found = true;
    }

    if (i == MAX_CHUNK_HEADER_SIZE) {
        printf("Unable to read length. Header corrupted.\n");
        return -WM_FAIL;
    }

    /* replace \r with string delim */
    chunk_header[i - 1] = 0;
    dgb_print("Chunk size str (value in hex): %s", chunk_header);
    return strtol(chunk_header, NULL, 16);
}

/*
 * Reads and discards the '\r\n' end part of a chunk block.
 */
static int zap_chunk_boundary(session_t *s)
{
    char chunk_boundary[2];
    int size_read;
    size_read = recv_buf(s, &chunk_boundary[0], 1, true);
    if (size_read != 1) {
        printf("Error while reading chunk len: %d\n",
            size_read);
        s->chunk.state = STATE_ERROR_CHUNK;
        return size_read;
    }
    size_read = recv_buf(s, &chunk_boundary[1], 1, true);
    if (size_read != 1) {
        printf("Error while reading chunk len: %d\n",
            size_read);
        s->chunk.state = STATE_ERROR_CHUNK;
        return size_read;
    }

    if (strncmp(chunk_boundary, "\r\n", 2)) {
        s->chunk.state = STATE_ERROR_CHUNK;
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}

/**
 * @return On success, returns the amount of data read. On error, return -WM_FAIL
 */
static int http_read_chunked(session_t *s, void *buf, unsigned int max_len)
{
    if (s->chunk.state == STATE_BETWEEN_CHUNK) {
        /* partial length from a chunk was returned earlier */
        int size_remaining = s->chunk.size - s->content_returned;
        int size_to_read =
            size_remaining <= max_len ? size_remaining : max_len;
        dgb_print("Readng %d bytes of chunked data (b/w)... remn'g %d",
             size_to_read, size_remaining);
        int size_read = recv_buf(s, buf, size_to_read, false);
        if (size_read < 0) {
            if (
                (size_read == -WM_E_HTTPC_SOCKET_ERROR &&
                 errno != EAGAIN ) ||
                (size_read == -WM_E_HTTPC_SOCKET_SHUTDOWN)) {
                printf("Failed to read partial chunk: %d\n",
                    size_read);
                s->chunk.state = STATE_ERROR_CHUNK;
            }
            return size_read;
        }

        s->content_returned += size_read;
        dgb_print("Read %d bytes of partial chunked data.", size_read);
        dgb_print("Cumulative: %d  Total: %d",
            s->content_returned, s->chunk.size);
        if (s->content_returned == s->chunk.size) {
            /* This chunk is done with */
            if (zap_chunk_boundary(s) == WM_SUCCESS) {
                s->content_returned = 0;
                s->chunk.size = 0;
                s->chunk.state = STATE_START_CHUNK;
            } else {
                printf("Zap chunk boundary failed\n");
                return -WM_FAIL;
            }
        }
        return size_read;
    } else if (s->chunk.state == STATE_START_CHUNK) {
        /* We are start of chunk header */
        s->chunk.size = get_content_len(s);
        if (s->chunk.size < 0) {
            s->chunk.state = STATE_ERROR_CHUNK;
            s->state = SESSION_ERROR;
            /* Return error code */
            return s->chunk.size;
        }
        dgb_print("Chunk size: %d", s->chunk.size);
        if (s->chunk.size == 0) {
            /* HTTP transaction complete */
            if (zap_chunk_boundary(s) == WM_SUCCESS) {
                s->outstanding_requests--;
                /* Reset returned content length */
                s->content_returned = 0;
                if (s->resp.keep_alive_ack)
                    s->state = SESSION_IT_DONE;
                else
                    s->state = SESSION_COMPLETE;
                return 0;
            } else {
                dgb_print("At end of transaction, ");
                dgb_print("could not zap chunk boundary");
                return -WM_FAIL;
            }
        }
        int to_read =
            s->chunk.size <= max_len ? s->chunk.size : max_len;
        int size_read = recv_buf(s, buf, to_read, false);
        if (size_read < 0) {
            if (
                (size_read == -WM_E_HTTPC_SOCKET_ERROR &&
                 errno != EAGAIN ) ||
                (size_read == -WM_E_HTTPC_SOCKET_SHUTDOWN)) {
                printf("Failed to read chunk data: %d\n",
                    size_read);
                s->chunk.state = STATE_ERROR_CHUNK;
            }

            return size_read;
        }
        s->content_returned = size_read;
        if (s->content_returned == s->chunk.size) {
            /* This chunk was done with in one go */
            if (zap_chunk_boundary(s) == WM_SUCCESS) {
                s->content_returned = 0;
                s->chunk.size = 0;
                s->chunk.state = STATE_START_CHUNK;
            } else {
                printf("Could not zap chunk boundary\n");
                return -WM_FAIL;
            }
        } else
            s->chunk.state = STATE_BETWEEN_CHUNK;

        dgb_print("Read %d bytes of start chunked data.", size_read);
        dgb_print("Cumulative: %d  Total: %d",
            s->content_returned, s->chunk.size);
        return size_read;
    } else {
        printf("Unknown state %d\n", s->chunk.state);
    }

    return -WM_FAIL;
}

int http_read_content(http_session_t handle, void *buf, unsigned int max_len)
{
    if (!handle || !buf || !max_len) {
        printf("Cannot read content.\n");
        return -WM_E_VAL;
    }
    int size_read;
    int status;
    session_t *s = (session_t *) handle;
    http_resp_t *resp;

    switch (s->state) {
    case SESSION_IT_DONE:
        if (s->outstanding_requests == 0) {
            printf("Cannot read content, no outstanding requests"
                ". Session state %d\n", s->state);
            return -WM_FAIL;
        }
    case SESSION_REQ_DONE:
        /*
         * The caller has not called ty_http_get_response_hdr. We need
         * to do it.
         */
        status = ty_http_get_response_hdr(handle, &resp);
        if (status != WM_SUCCESS)
            return status;
        if (resp->status_code != 200) {
            printf("Not reading any content.\n");
            printf("HTTP status code: %d\n", resp->status_code);
            return -WM_FAIL;
        }
        break;
    case SESSION_RESP_DONE:
    case SESSION_DATA__PROGRESS:
        if (STATE_ERROR_CHUNK == s->chunk.state)
            return -WM_FAIL;
        break;
    case SESSION_COMPLETE:
    case SESSION_ALLOC:
    case SESSION_PREPARE:
    case SESSION_ERROR:
    case SESSION_RAW:
    default:
        printf("Cannot read content. Unknown state %d\n", s->state);
        return -WM_FAIL;
    }

    if (s->resp.chunked)
        size_read = http_read_chunked(s, buf, max_len);
    else
        size_read = http_read_standard(s, buf, max_len);

#ifdef DUMP_RECV_DATA
    char *buff = buf;
    int i;
    dgb_print("\n\rHTTPC: ********** HTTPC data *************\n\r");
    if (size_read > 0) {
        for (i = 0; i < size_read; i++) {
            if (buff[i] == '\r')
                continue;
            if (buff[i] == '\n') {
                wmprintf("\n\r");
                continue;
            }
            dgb_print("%c", buff[i]);
        }
        dgb_print("\n\r");
    } else {
        dgb_print("Size: %d Not reading\n\r", size_read);
    }
    dgb_print("HTTPC: ***********************************\n\r");
#endif /* DUMP_RECV_DATA */

    return size_read;
}

void ty_http_close_session(http_session_t *handle)
{
    session_t *s = (session_t *) *handle;

    net_close(s->sockfd);
    delete_session_object(s);
    *handle = 0;
}
