#ifndef __TUYA_MUDEX_DEFS_H__
#define __TUYA_MUDEX_DEFS_H__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "tuya_mudex_codec_io.h"
enum TUYA_MUXER_TYPE{
    TUYA_MUX_NONE,
    TUYA_MUXER_MP4,
    TUYA_MUXER_MEPGTS,
    TUYA_MUXER_MEPGPS,
};

typedef struct TUYA_FILE_MEM_SETTING{
    //该接口若为NULL，必须设置输入buffer
    char*                      proc_file_path;
    unsigned char              *proc_buffer_base;
    int                        proc_buffer_size;
    int                        proc_frame_size;
    int                        proc_count;
}TUYA_FILE_MEM_SETTING;


// 与 codec io相关 
// 使用下面结构体的相当于把输入输出都由TUYA_CODEC_OUTPUT_STREAM 管理
/*
* 该结构体中的参数为特殊的参数，可以动态修改的
*/
typedef struct TUYA_DYNAMIC_IO_PROC_PARAM{

    void                       (*proc_before_write)(unsigned char* buf,unsigned char** ret_buf,int *size,void *args);
    void                       (*proc_before_write_yuv)(unsigned char *y_buf,unsigned char *u_buf,unsigned char *v_buf,
                                                        unsigned char** ret_buf,int *size,void *args);
    //输出处理可以动态修改，如可以动态修改缩放，格式等
    //已存在与buffer中的数据不会改动，修改对下一次放入buffer中的数据生效
    void                       (*output_proc)(unsigned char* buf, unsigned char **ret_buf,int *size,void *args);  
    int                        max_node_size; 
    void                       *output_proc_args;                   
}TUYA_DYNAMIC_IO_PROC_PARAM;

typedef struct TUYA_IO_SETTING{
    //该接口若为NULL，必须设置输入buffer
    char*                      input_file_path;
    unsigned char              *input_buffer_base;
    int                        input_buffer_size;
    int                        input_frame_size;
    int                        input_count;
    char*                      output_file_path;
    unsigned char              *output_buffer_base;
    int                        output_buffer_size;
    int                        output_frame_size;
    int                        output_count;
    TUYA_CODEC_OUTPUT_STREAM   *input_buffer;
    TUYA_CODEC_OUTPUT_STREAM   *output_buffer;
    TUYA_DYNAMIC_IO_PROC_PARAM  io_proc;
}TUYA_IO_SETTING;


//========enc param==========//
typedef struct TuyaAudioEnocderParam {
    unsigned long	frame_rate;
    unsigned long	nSampleRate;
    unsigned int	nChannels;
    unsigned int	channel_layout;
    unsigned int	nPCMBitSize;
    unsigned long	nInputSamples;
	unsigned long	nMaxOutputBytes;
    unsigned int	inputFormat;
    unsigned int	outputFormat;
    unsigned long   bitRate;
    unsigned int    bandWidth;
} TuyaAudioEnocderParam;

typedef struct TuyaVideoEnocderParam {
    unsigned long	nFrameRate;
    unsigned long   bitRate;
    unsigned int    width;
    unsigned int    height;
    int             intra_period;
} TuyaVideoEnocderParam;
//========enc param==========//
#endif