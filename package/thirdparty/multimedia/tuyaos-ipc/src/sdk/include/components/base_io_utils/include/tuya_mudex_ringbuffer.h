#ifndef __MUDEX_RINGBUFFER_H__
#define __MUDEX_RINGBUFFER_H__
#ifdef __cplusplus
extern "C" {
#endif
#define MUDEX_RINGBUFFER_HANDLE void*


//flags

/*
* 每个node使用固定大小，初始化ringbuffer时使用了该flag需要在初始化后调用mudex_ringbuffer_set_fixed_node_size()初始化node大小
*/
#define MUDEX_RINGBUF_FLAG_FIXED_NODE_SIZE 1<<0
/*
* 写数据时强制覆盖
*/
#define MUDEX_RINGBUF_FLAG_FORCE_OVERWRITE 1<<1

int mudex_ringbuffer_init(MUDEX_RINGBUFFER_HANDLE *ring,int buf_len,int flags);
int mudex_ringbuffer_init_malloc_outside(MUDEX_RINGBUFFER_HANDLE *ring,unsigned char *buf,int buf_len,int flags);

int mudex_ringbuffer_set_fixed_node_size(MUDEX_RINGBUFFER_HANDLE _ring,int fixed_size);
/*
* 读、写数据
*/
int mudex_ringbuffer_get_frame(MUDEX_RINGBUFFER_HANDLE ring,unsigned char* frm_buf);
int mudex_ringbuffer_set_frame(MUDEX_RINGBUFFER_HANDLE ring,unsigned char* frm_buf,int frm_sz,int switch_frame_flag);
int mudex_ringbuffer_try_get_frame(MUDEX_RINGBUFFER_HANDLE _ring);
/*
* 读、写数据时附带附加信息
*/
int mudex_ringbuffer_set_frame_with_attch_info(MUDEX_RINGBUFFER_HANDLE _ring,unsigned char* frm_buf,int frm_sz,
                                                            const void *args,int args_size,int switch_node_flag);
int mudex_ringbuffer_get_frame_with_attch_info(MUDEX_RINGBUFFER_HANDLE _ring,unsigned char* frm_buf,void *args);
int mudex_ringbuffer_try_get_frame_with_attch_info(MUDEX_RINGBUFFER_HANDLE _ring,void *args);


int mudex_ringbuffer_release(MUDEX_RINGBUFFER_HANDLE ring);

//若要修改最新node，谨慎在多个线程中同时进行修改。(临时，后续会修改，暂不建议使用该函数)
int mudex_ringbuffer_get_processing_node(MUDEX_RINGBUFFER_HANDLE ring,unsigned char** frm_buf,int *frm_sz);
int mudex_ringbuffer_set_processing_node(MUDEX_RINGBUFFER_HANDLE _ring,unsigned char* frm_buf,int frm_sz);
int mudex_ringbuffer_get_processing_node_end(MUDEX_RINGBUFFER_HANDLE ring);
#ifdef __cplusplus
}
#endif
#endif /* __MUDEX_RINGBUFFER_H__ */
