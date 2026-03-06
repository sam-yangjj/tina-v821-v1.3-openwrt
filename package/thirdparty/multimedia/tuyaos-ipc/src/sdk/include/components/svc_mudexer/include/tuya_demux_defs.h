#ifndef __TUYA_DEMUX_DEFS_H__
#define __TUYA_DEMUX_DEFS_H__



typedef enum TUYA_DEMUX_FLAGS{
    //对于解析出的packets数据直接回写到用户配置的输出buffer中
    //对于配置了当前flag的解封装器将不再内部缓存数据，tuya_demux_one_video_frame接口和tuya_demux_one_audio_frame接口无法使用
    TUYA_DEMUX_PARAM_FLAG_PACKET_WRITEBACK_DIRECT = 0x1,
}TUYA_DEMUX_FLAGS; 

#endif