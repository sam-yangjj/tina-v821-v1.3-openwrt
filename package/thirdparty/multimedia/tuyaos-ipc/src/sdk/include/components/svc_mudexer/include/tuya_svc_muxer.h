
/**
* @file tuya_svc_muxer.h
* @author www.tuya.com
* @brief muxer module is used to pack the video/audio/text etc. into mp4/ts stream
*
* @copyright Copyright (c) tuya.inc 2019
*
*/
#ifndef __TUYA_SVC_MUXER_H__
#define __TUYA_SVC_MUXER_H__


#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_mudex_defs.h"

enum TUYA_MUXER_RET_STATUS{
    MUXER_RUNING,
    MUXER_SAVE_BAK,
    MUXER_DOWN,
    MUXER_SECTION_SWITCH_STATUS_VIDEO_DOWN,
    MUXER_SECTION_SWITCH_STATUS_DOWN,
    MUXER_ERR,
};

#ifndef IN
#define IN 
#endif
//该flags使能时，不适用mem交互的方式，每次传入buf
//内部不对输入进行检查，直接封装，错误控制由外层控制

#define TUYA_MUXER_FLAG_WRITE_BUF_DIRCT 0x1

typedef struct __TUYA_MUXER_PARAM {
    int                          flags;
    TUYA_FILE_MEM_SETTING        output_io_setting;
    enum TUYA_MUXER_TYPE         muxer_type;
    //audio
    int                          has_audio;
    TUYA_FILE_MEM_SETTING        audio_io_setting;
    int                          audio_codec_id;
    unsigned long 	             nSampleRate;
    unsigned int	             nChannels;
    unsigned int	             channel_layout;
    unsigned int                 a_frm_rate;
    //unsigned int	nPCMBitFmt;//if is pcm,set  //delete,2020-11-12,not use
    long                          a_bit_rate; 
    //video
    int                          has_video;
    TUYA_FILE_MEM_SETTING        video_io_setting;
    int                          intra_period;
    int                          video_codec_id;
    unsigned int	             width;
    unsigned int	             height;
    unsigned int	             v_frm_rate;
    long                         v_bit_rate;
} TUYA_MUXER_PARAM;


//??
typedef struct __TUYA_MUXER_IO_DESC {
    
    char*             header_start;
    int               header_size;

    char*             tail_start;
    int               tail_size;

    char*             samples_rd;
    char*             samples_wtr;
} TUYA_MUXER_IO_DESC;

#define TUYA_MUXER_USER_HANDLER  void*

TUYA_MUXER_USER_HANDLER tuya_open_muxer(IN TUYA_MUXER_PARAM *param);

int mux_ringbuffer_append_video_data(TUYA_MUXER_USER_HANDLER _handler,unsigned char *buf,int size,int pts);
int mux_ringbuffer_append_audio_data(TUYA_MUXER_USER_HANDLER _handler,unsigned char *buf,int size,int pts);

/**
 * \fn int tuya_set_muxer_max_time(TUYA_MUXER_USER_HANDLER _handler,int seconds,int save_seconds_bak)
 * \brief set muxer max time;if set,the muxer will crop sections by time,在使用mem的输出方式时，该接口不能调用
 * \param[in] _handler: 
 * \param[in] seconds:
 * \param[in] save_seconds_bak:
 * \return int
 */
int tuya_set_muxer_max_time(IN TUYA_MUXER_USER_HANDLER _handler,IN int seconds,IN int save_seconds_bak);

/**
 * \fn tuya_set_param_output_buffer(TUYA_MUXER_USER_HANDLER _handler,unsigned char *buffer_base,int buffer_size)
 * \brief set muxer output buffer base and size,if use mem output instead of file
 * \param[in] _handler: 
 * \param[in] buffer_base:
 * \param[in] buffer_size:
 * \return void
 */
// void tuya_set_param_output_buffer(TUYA_MUXER_USER_HANDLER _handler,unsigned char *buffer_base,int buffer_size);
/**
 * \fn int tuya_start_mux_frame(IN TUYA_MUXER_USER_HANDLER _handler)
 * \brief set write data mode  for video calm
 * \param[in] _handler: 
 * \return int
 */
int tuya_start_mux_frame(IN TUYA_MUXER_USER_HANDLER _handler);
/**
 * \fn int tuya_close_muxer(IN TUYA_MUXER_USER_HANDLER _handler)
 * \brief set write data mode  for video calm
 * \param[in] _handler: TRUE/FALSE
 * \return void
 */
void tuya_close_muxer(IN TUYA_MUXER_USER_HANDLER _handler);

/**
 * \fn int tuya_start_mux_file(IN TUYA_MUXER_USER_HANDLER _handler)
 * \brief set write data mode  for video calm
 * \param[in] _handler: TRUE/FALSE
 * \return int
 */
int tuya_start_mux_file(IN TUYA_MUXER_USER_HANDLER _handler);

/**
 * \fn void tuya_set_switch_section_cb(IN TUYA_MUXER_USER_HANDLER _handler,int (*switch_cb_func)(int flag,char *output_filename,double duration))
 * \brief set write data mode  for video calm
 * \param[in] _handler
 * \return void
 */
// void tuya_set_switch_section_cb(IN TUYA_MUXER_USER_HANDLER _handler,int (*switch_cb_func)(int flag,char *output_filename,double duration));




enum MUX_MEDIA_TYPE{
    MUX_MEDIA_TYPE_NONE,
    MUX_MEDIA_TYPE_VIDEO,
    MUX_MEDIA_TYPE_AUDIO,
};




TUYA_MUXER_USER_HANDLER tuya_open_muxer2(IN TUYA_MUXER_PARAM *param,int max_out_frame_size);
int tuya_start_mux_frame2(IN TUYA_MUXER_USER_HANDLER _handler,unsigned char *buf,int size,int key_frame_flag,enum MUX_MEDIA_TYPE type,unsigned char* output,int *output_size);


#ifdef __cplusplus
}
#endif

#endif