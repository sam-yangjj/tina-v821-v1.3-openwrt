/**
* @file tuya_svc_muxer.h
* @author www.tuya.com
* @brief demuxer module is used to unpack mp4/ts stream,and send the stream into decoder if needed.
*
* @copyright Copyright (c) tuya.inc 2019
*
*/
#ifndef __TUYA_SVC_DEMUXER_H__
#define __TUYA_SVC_DEMUXER_H__

#include "tuya_mudex_log_util.h"
#include "tuya_mudex_io_util.h"
#include "tuya_svc_muxer.h"
#include "tuya_demux_defs.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct __TUYA_DEMUX_PARAM{
    //input 
    enum TUYA_MUXER_TYPE  demuxer_type;
    char                  *input_file_name;
    MUDEX_STREAM_T        *input_buffer;
    int                   in_type;

    int                   a_codec_type;
    int                   v_codec_type;

    //output
    //enum TUYA_MUXER_TYPE  out_muxer_type;
    //_Bool                  if_display;
    char                  *output_file_name;
    char                  *output_video_file_name;
    char                  *output_audio_file_name;
    unsigned char         *out_v_buffer_base;
    int                    out_v_buffer_sz;
    unsigned char         *out_a_buffer_base;
    int                    out_a_buffer_sz;

    TUYA_DEMUX_FLAGS       flags;//在tuya_demux_defs.h文件中定义了flags
}TUYA_DEMUX_PARAM;
// typedef struct __TUYA_DEMUX_HANDLER{
//     void *handler;
//     void *tydm_ctx;
// }TUYA_DEMUX_HANDLER;
#define TUYA_DEMUX_HANDLER void*

enum OUTPUT_MEM_TYPE{
    OUTPUT_MEM_TYPE_VIDEO,
    OUTPUT_MEM_TYPE_AUDIO,
    OUTPUT_MEM_TYPE_REMUX_OUTPUT,
};
enum DEMUX_STATUS{
    DEMUX_STATUS_DEMUXING = 0x0,//所有service都未解复用完成
    DEMUX_STATUS_VIDEO_DOWN = 0x1,//所有video service解复用完成
    DEMUX_STATUS_AUDIO_DOWN = 0x2,//所有audio service解复用完成
    //0x3是指DEMUX_STATUS_VIDEO_DOWN|DEMUX_STATUS_AUDIO_DOWN
    //后续若有修改需注意这点
    DEMUX_STATUS_DEMUX_DOWN = 0x3,//所有service都解复用完成，可以正常close掉
};

enum TUYA_DEMUX_PARSER_FLAG{
    TUYA_DEMUX_PARSER_BUFFERED,
    TUYA_DEMUX_PARSER_WRITE_DIRECT,
};
TUYA_DEMUX_HANDLER tuya_open_demuxer(TUYA_DEMUX_PARAM *param);
//该方法从open中分离出是因为避免在初始化的时候做太多操作，初始化后启动现在，track在线程中完成
void tuya_mp4_track_data(TUYA_DEMUX_HANDLER handler);

//若TUYA_DEMUX_PARAM_FLAG_PACKET_WRITEBACK_DIRECT拉高，则该接口无法使用
int tuya_demux_one_video_frame(TUYA_DEMUX_HANDLER handler);

//若TUYA_DEMUX_PARAM_FLAG_PACKET_WRITEBACK_DIRECT拉高，则该接口无法使用
int tuya_demux_one_audio_frame(TUYA_DEMUX_HANDLER handler);


//如果返回-1，表示当前handler是实时解封装，没有总的帧数
//如果返回>0的合法整数则表示当前handler需要demux的总帧数
int tuya_demux_get_all_video_frame_num(TUYA_DEMUX_HANDLER handler);
//检查当前handler的状态
enum DEMUX_STATUS tuya_demux_check_status(TUYA_DEMUX_HANDLER handler);

enum TUYA_DEMUX_PARSER_FLAG tuya_demux_get_parse_type(TUYA_DEMUX_HANDLER handler);
int tuya_demux_one_packet(TUYA_DEMUX_HANDLER handler);
int tuya_demux_access_packets(TUYA_DEMUX_HANDLER handler);
int get_demux_output_content(TUYA_DEMUX_HANDLER handler,enum OUTPUT_MEM_TYPE type,unsigned char*buf);
int set_demux_io(TUYA_DEMUX_HANDLER handler,unsigned char *base,int size,enum OUTPUT_MEM_TYPE type);
int start_demux_all_frames2file(TUYA_DEMUX_HANDLER handler);
int tuya_close_demuxer(TUYA_DEMUX_HANDLER *handler);



#ifdef __cplusplus
}
#endif
#endif