#ifndef __STREAM_H__
#define __STREAM_H__

#include "tuya_mudex_io_util.h"

//用于输入，读取数据
MUDEX_STREAM_T *tym_stream_in_open(int type,char *fname);
//用于输出，写数据
MUDEX_STREAM_T *tym_stream_out_open(int type,char *fname);
//自定义
MUDEX_STREAM_T *tym_stream_open(int type,char *fname,int mode);
MUDEX_STREAM_T *tym_stream_open_with_flags(int type,char *fname,int mode,int flags);
int tym_set_mem_param(MUDEX_STREAM_T * stream,unsigned char *base,int size);
#endif //__STREAM_H__
