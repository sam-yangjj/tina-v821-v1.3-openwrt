#ifndef __TUYA_MUDEX_CODEC_IO_H__
#define __TUYA_MUDEX_CODEC_IO_H__
#include <stdlib.h>
#include <pthread.h>

typedef struct codec_stream_info{
    long long pts;
}codec_stream_info;
struct __codec_io_procs{
    //该函数只有在switch_write_frame生效的时候才有用
    //作用为，每次switch之前先把内容二次处理之后才形成node

    //输出前处理
    void (*output_proc)(unsigned char* buf,unsigned char** ret_buf,int *size,void *args);
    //目前未使用以下两个参数
    int max_node_size;
    unsigned char *proc_temp;
    //输出前处理，
    //   1)该处理可能是在写入时先处理
    //   2)也可能是形成node结点时读出再处理（目前该处理暂时不可用）
    int output_proc_flag;
    //所有回调函数公用的参数
    void *output_proc_args;
    pthread_mutex_t output_proc_mutex;
        
    //先经过处理后再写入
    void (*proc_before_write)(unsigned char* buf,unsigned char** ret_buf,int *size,void *args);
        
    //针对不连续内存，使能该方法内部会去使能write_yuv_planar_frame函数
    void (*proc_before_write_non_continue_mem)(unsigned char *y_buf,unsigned char *u_buf,unsigned char *v_buf,
                                                unsigned char** ret_buf,int *size,void *args);
    //一些差异性接口,
    //该接口只有在proc_before_write_yuv这个接口不为NULL时才使能
    //对于有些解码输出的yuv为不连续存放
    //该接口会把每部分指针给外部回调中处理
    //该函数写入的数据都是未裁剪的原始数据，未裁剪的数据更方便外部进行其他处理
    // int (*write_yuv_planar_frame)(struct __codec_io_procs *procs,void *args,unsigned char *y_buf,int y_size,
                                                                    // unsigned char *u_buf,int u_size,
                                                                    // unsigned char *v_buf,int v_size);
    //void *proc_before_write_args;
    
    
};
typedef struct __codec_io_procs codec_io_procs;

typedef struct TUYA_CODEC_OUTPUT_STREAM TUYA_CODEC_OUTPUT_STREAM;
struct TUYA_CODEC_OUTPUT_STREAM{
    //内部维护，文件方式,mem方式
    void *stream;
    int w_frame_counter;
    int r_frame_counter;
    int drop_frame_counter;
    int if_overwrite;
    int (*write_frame)(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *buf,int size);
    int (*switch_write_frame)(TUYA_CODEC_OUTPUT_STREAM *stream,codec_stream_info *info);
    int (*read_frame)(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *buf,codec_stream_info *info);
    int (*close)(TUYA_CODEC_OUTPUT_STREAM *stream);

    int (*malloc_after_get_outputsize)(TUYA_CODEC_OUTPUT_STREAM *stream,int size);
    int pic_count;

    codec_io_procs io_procs;
};

int tuya_mudex_open_codec_io_stream(TUYA_CODEC_OUTPUT_STREAM **stream,void *ring,
                                    char *file_name,unsigned char *base,int size,int output_flag,
                                    int manag_internal,int pic_count,int if_overwrite,
                                        void (*proc_before_write)(unsigned char* buf,unsigned char** ret_buf,int *size,void *args),
                                        void (*proc_before_write_yuv)(unsigned char *y_buf,unsigned char *u_buf,unsigned char *v_buf,unsigned char** ret_buf,int *size,void *args),
                                        void (*output_proc)(unsigned char* buf, unsigned char **ret_buf,int *size,void *args),int max_node_size,void *args);
/*
* 是否可以取到帧
*/
int tuya_mudex_codec_io_stream_if_has_frame(TUYA_CODEC_OUTPUT_STREAM *stream);

int tuya_mudex_close_codec_io_stream(TUYA_CODEC_OUTPUT_STREAM *stream);

int tuya_sw_codec_set_picsize_later(TUYA_CODEC_OUTPUT_STREAM *stream,int pic_size);
/*
* 每帧处理可以动态改变，但是内部buffer只malloc一次，并且在release的时候一起释放
*/

/*
* 写完整的一帧
*/
int tuya_mudex_codec_io_stream_write_frame(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *buf,int size,long long pts);
/*
* 写片段
*/
int tuya_mudex_codec_io_stream_write_segment(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *buf,int size,long long pts,int frame_dowm);
/*
* 读完整的一帧
*/
int tuya_mudex_codec_io_stream_read_frame(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *buf,long long *pts);


/*
* buffer处理
*/
enum codec_io_procs_err_code{
    codec_io_procs_err_code_not_enable_proc = -1,
    codec_io_procs_err_code_stream_err = -2,
};

int codec_io_procs_reset(TUYA_CODEC_OUTPUT_STREAM *new_stream,void *args,
                            void (*proc_before_write)(unsigned char* buf,unsigned char** ret_buf,int *size,void *args),
                            void (*proc_before_write_yuv)(unsigned char *y_buf,unsigned char *u_buf,unsigned char *v_buf,unsigned char** ret_buf,int *size,void *args),
                            void (*output_proc)(unsigned char* buf, unsigned char **ret_buf,int *size,void *args),int max_node_size);


enum codec_io_procs_err_code codec_io_procs_proc_before_write_yuv(TUYA_CODEC_OUTPUT_STREAM *stream,unsigned char *y_buf,int y_size,
                              unsigned char *u_buf,int u_size,
                              unsigned char *v_buf,int v_size);
#endif