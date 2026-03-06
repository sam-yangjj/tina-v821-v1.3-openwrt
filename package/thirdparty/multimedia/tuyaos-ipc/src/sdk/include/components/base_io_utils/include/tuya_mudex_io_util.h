#ifndef __TUYA_MUDEX_IO_UTIL_H__
#define __TUYA_MUDEX_IO_UTIL_H__
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASE_IO_MALLOC
#define BASE_IO_MALLOC(size)     __base_io_malloc(size)
#endif

#ifndef BASE_IO_CALLOC
#define BASE_IO_CALLOC(n,size)   calloc(n,size)
#endif

#ifndef BASE_IO_FREE
#define BASE_IO_FREE(ptr)   free(ptr)
#endif

//#ifndef MUDEX_ERR
#include "tuya_mudex_log_util.h"
//#endif

static inline void *__base_io_malloc(size_t size){
   void *p = malloc(size);
   if(p){
        memset(p,0x0,size);
   }
   else{
	   printf("%s %s(%d)",(strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__), __FUNCTION__, __LINE__);
       printf("BASE_IO_MALLOC malloc err ,size = %d\n",(int)size);
   }
   return p;
}




#define ERR_STREAM_OK 0
#define ERR_STREAM_IS_NULL (-1)
#define ERR_STREAM_FILE_NOT_OPEN (-2)
#define ERR_STREAM_SYSTEM_ERROR (-3)




/**
 * @name io open modes
 * @{
 */
#define MUDEX_IO_FLAG_READ  1                                      /**< read-only */
#define MUDEX_IO_FLAG_WRITE 2                                      /**< write-only */
#define MUDEX_IO_FLAG_READ_WRITE (MUDEX_IO_FLAG_READ|MUDEX_IO_FLAG_WRITE)  /**< read-write pseudo flag */


enum TUYA_MUDEX_IO_STREAM_TYPE{
	STREAM_TYPE_NONE,
	STREAM_TYPE_FILE,
	STREAM_TYPE_MEM
};
enum TUYA_MUDEX_IO_TYPE{
	TUYA_MUDEX_IO_DEFAULT,
	TUYA_MUDEX_IO_WRITE,
	TUYA_MUDEX_IO_READ,
};
struct _mudex_stream
{
	char                              *name;
	void*                              priv_protocol;
	int                                priv_protocol_size;
	const char                        *default_whitelist;
	enum TUYA_MUDEX_IO_STREAM_TYPE     priv_type;
	void*   (*stream_open)             (void *name,char *mode);
//_mudex_stream flags defines
/*
* 只对STREAM_TYPE_MEM类型有效
* 指定输出的大小，若长度不够则输出为0
*/
#define MUDEX_STREAM_FLAG_READ_SPECIFIC_LENGTH_DATA 0x1<<0
#define MUDEX_STREAM_FLAG_WRITE_SPECIFIC_LENGTH_DATA 0x1<<1


	void*   (*stream_open_with_flags)  (void *name,char *mode,int flags);
    int     (*stream_set_param)        (void* priv_protocol,unsigned char *base,int size);
	int     (*stream_read)             (void* priv_protocol, void* buf, int size);
	int64_t (*stream_seek)             (void* priv_protocol, int64_t offset, int whence);
	//该方法主要用于可读可写的形式，如mem的方式，type选择读或写
	int64_t (*stream_seek2)            (void* priv_protocol, int64_t offset, int whence,enum TUYA_MUDEX_IO_TYPE type);
    int     (*stream_write)            (void* priv_protocol, void *buf, int size);
	int     (*stream_close)            (void* priv_protocol);
    int     (*get_stream_size)         (void* priv_protocol);
	//根据指定位置读,并不改变任何指针
	//light_flag 为1表示使用内部内存，无拷贝
	int     (*stream_try_read)         (void* priv_protocol, int64_t offset,int whence,enum TUYA_MUDEX_IO_TYPE type,int light_flag,unsigned char **buf,int size);
	int     (*stream_try_write)        (void* priv_protocol, int size );
	
};
typedef struct _mudex_stream MUDEX_STREAM_T;


MUDEX_STREAM_T *ty_ioutil_get_stream_format(enum TUYA_MUDEX_IO_STREAM_TYPE type,int mode);

int16_t *ty_ioutil_check_stream_flags(enum TUYA_MUDEX_IO_STREAM_TYPE type,int mode,int flags);

void ty_ioutil_free_stream(MUDEX_STREAM_T *st);
#ifdef __cplusplus
}
#endif
#endif