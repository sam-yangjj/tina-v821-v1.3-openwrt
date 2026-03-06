#ifndef __TUYA_SVC_CODEC_H__
#define __TUYA_SVC_CODEC_H__

#include "tuya_mudex_codec_io.h"
#include "tuya_mudex_defs.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

#define  TUYA_DECODER_USER_HANDLER void*
#define  TUYA_ENCODER_USER_HANDLER void*

//decoder param flags
#define DECODE_FLAG_RETURN_PER_FRAME 0x1

enum TUYA_SW_DEC_CODEC_ID{
    TUYA_SW_DEC_CODEC_H264,
};
enum SW_DEC_OUT_TYPE{
    SW_DEC_UNKNOWN_TYPE,
    SW_DEC_YUV420_PLANAR,
    SW_DEC_YUV420_INTERLEAVE,
};

//enum for codec, sync with tuya cloud and APP, should NOT be changed
typedef enum
{
    TUYA_SW_CODEC_VIDEO_H264,
    TUYA_SW_CODEC_VIDEO_HEVC,
    TUYA_SW_CODEC_AUDIO_AAC,
    TUYA_SW_CODEC_INVALID,
}TUYA_SW_CODEC_ID;



typedef struct TUYA_ENC_PARAM_SETTING{
    //通用
    int                         frame_size;
    //视频相关参数
    int                         width;
    int                         height;
    //音频相关参数
    int                         sample_rate;//采样率
    int                         channel_num;//单双声道
    int                         bit_len;//bit位深
}TUYA_ENC_PARAM_SETTING;

typedef struct TUYA_DEC_PARAM_SETTING{
    //int                        display_flag;
}TUYA_DEC_PARAM_SETTING;



typedef struct TUYA_VIDEO_DECODER_PARAM{
    //使用文件形式的cfg，默认为0，不使用
    int                        use_cfg_file;
    char*                      cfg_path;
    TUYA_DEC_PARAM_SETTING     setting;
    TUYA_IO_SETTING            codec_io;
    TUYA_SW_CODEC_ID              codec;
    
}TUYA_VIDEO_DECODER_PARAM;

typedef struct TUYA_VIDEO_ENCODER_PARAM{
    //使用文件形式的cfg，默认为0，不使用
    int                        use_cfg_file;
    char*                      cfg_path;
    TUYA_ENC_PARAM_SETTING     setting;
    TUYA_IO_SETTING            codec_io;
    TUYA_SW_CODEC_ID              codec;
    
}TUYA_VIDEO_ENCODER_PARAM;

#define MAX_CODEC_TYPE_STR_SIZE 32
typedef struct TUYA_SW_CODEC_SKILLS {
    int decoder_num;
    char decoder_skill_list[3][MAX_CODEC_TYPE_STR_SIZE];
    int encoder_num;
    char encoder_skill_list[3][MAX_CODEC_TYPE_STR_SIZE];

} TUYA_SW_CODEC_SKILLS;



enum TY_SW_CODEC_FLAG{
    DEC,
    ENC
};
/**
void tuya_sw_codec_get_skills(TUYA_SW_CODEC_SKILLS *skills);
 * \brief get supported codec standard
 * \return void
 */
void tuya_sw_codec_get_skills(TUYA_SW_CODEC_SKILLS *skills);
/**
 * \fn void tuya_sw_codec_list_skills()
 * \brief print skiils
 * \return void
 */
void tuya_sw_codec_list_skills();
/**
 * \fn bool_t tuya_sw_codec_check_codec_ifsupport(TUYA_SW_CODEC_ID codec,enum TY_SW_CODEC_FLAG codec_flag)
 * \brief Determine whether the input parameter coedec is within the range of the codec standard's capability set
 * \param[in] codec: user set codec
 * \param[in] codec_flag: DEC/ENC
 * \return bool_t
 */
int tuya_sw_codec_check_codec_ifsupport(TUYA_SW_CODEC_ID codec,enum TY_SW_CODEC_FLAG codec_flag);
/**
 * \fnvoid tuya_get_decoder_default_setting(TUYA_DEC_PARAM_SETTING *setting)
 * \brief return a default setting struct
 * \return TUYA_DEC_PARAM_SETTING
 */
void tuya_get_decoder_default_setting(TUYA_DEC_PARAM_SETTING *setting);
/**
 * \fn TUYA_DECODER_USER_HANDLER tuya_open_sw_decoder(TUYA_VIDEO_DECODER_PARAM *param)
 * \brief return a handler of decoder context
 * \param[in] param: user set param
 * \param[in] init_flag: 若为false，需要用户自己调用tuya_sw_decoder_init_priv_dec进行解码器的初始化
 * \return TUYA_DECODER_USER_HANDLER
 */
TUYA_DECODER_USER_HANDLER tuya_open_sw_decoder(TUYA_VIDEO_DECODER_PARAM *param);
/**
 * \fn INT_T tuya_sw_decoder_init_priv_dec(TUYA_DECODER_USER_HANDLER handler,TUYA_VIDEO_DECODER_PARAM *param)
 * \brief return a handler of decoder context
 * \param[in] handler: the handler which generates from tuya_open_sw_decoder() 
 * \param[in] param: user set param
 * \return TUYA_DECODER_USER_HANDLER
 */
int tuya_sw_decoder_init_priv_dec(TUYA_DECODER_USER_HANDLER handler,TUYA_VIDEO_DECODER_PARAM *param);
/**
 * \fn int tuya_dec_set_input_bs(TUYA_DECODER_USER_HANDLER handler,MUDEX_STREAM_T *input_bs)
 * \brief set input bs 
 * \param[in] handler: the handler which generates from tuya_open_sw_decoder() 
 * \param[in] input_bs: the input_bs is generated from tym_io_open() 
 * \return int : if ture,return 0; if false,return <0
 */
int tuya_set_dec_input_bs(TUYA_DECODER_USER_HANDLER handler,TUYA_CODEC_OUTPUT_STREAM *input_bs);
/**
 * \fn int tuya_start_dec_one_frame(TUYA_DECODER_USER_HANDLER _handler)
 * \brief start decoder one frame
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : if ture,return 0; if false,return <0
 */
int tuya_start_dec_one_frame(TUYA_DECODER_USER_HANDLER _handler);
/**
 * \fn int tuya_start_dec_access_frames(TUYA_DECODER_USER_HANDLER _handler)
 * \brief start decoder ,it returns when there are no more access frames 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : if ture,return 0; if false,return <0
 */
int tuya_start_dec_access_frames(TUYA_DECODER_USER_HANDLER _handler);
/**
 * \fn int tuya_get_decoded_frame_num(TUYA_DECODER_USER_HANDLER _handler)
 * \brief return decoded frame num 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : decoded frame num
 */
int tuya_get_decoded_frame_num(TUYA_DECODER_USER_HANDLER _handler);
/**
 * \fn int tuya_get_decoded_frame(TUYA_DECODER_USER_HANDLER _handler)
 * \brief return decoded frame of decoded pic list 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : decoded frame num
 */
int tuya_get_decoded_frame(TUYA_DECODER_USER_HANDLER _handler,unsigned char *buf,int *size,unsigned long long *pts);
/**
 * \fn int tuya_sw_decoder_get_dst_frame_info(TUYA_DECODER_USER_HANDLER _handler,int key_num,char **keys,void **values)
 * \brief 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : decoded frame num
 */
int tuya_sw_decoder_get_dst_frame_info(TUYA_DECODER_USER_HANDLER _handler,int key_num,char **keys,void **values);
/**
 * \fn int tuya_sw_encoder_get_dst_frame_info(TUYA_DECODER_USER_HANDLER _handler,int key_num,char **keys,void **values,int **value_size_in_byte)
 * \brief 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : decoded frame num
 */
int tuya_sw_encoder_get_dst_frame_info(TUYA_DECODER_USER_HANDLER _handler,int key_num,char **keys,void **values,int **value_size_in_byte);
/**
 * \fn int tuya_sw_dec_check_running_status(TUYA_DECODER_USER_HANDLER _handler)
 * \brief return decoded frame of decoded pic list 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : decoded status ,0-running 1-down
 */
int tuya_sw_dec_check_running_status(TUYA_DECODER_USER_HANDLER _handler);
/**
 * \fn int tuya_close_decoder(TUYA_DECODER_USER_HANDLER _handler)
 * \brief close decoder ,and release all resources 
 * \param[in] _handler: the handler which generates from tuya_open_sw_decoder() 
 * \return int : if ture,return 0; if false,return <0
 */
int tuya_close_decoder(TUYA_DECODER_USER_HANDLER _handler);
/**
 * \fn int tuya_sw_decoder_param_reset(TUYA_DECODER_USER_HANDLER handler,TUYA_DYNAMIC_IO_PROC_PARAM *reset_param)
 * \brief  reset decoder handler
 * \param[in] reset_param: reset param
 * \return return err code
 */
int tuya_sw_decoder_param_reset(TUYA_DECODER_USER_HANDLER handler,TUYA_DYNAMIC_IO_PROC_PARAM *reset_param);
/**
 * \fn TUYA_ENCODER_USER_HANDLER tuya_open_sw_encoder(TUYA_VIDEO_DECODER_PARAM *param)
 * \brief return a handler of encoder context
 * \param[in] param: user set param
 * \return TUYA_ENCODER_USER_HANDLER
 */
TUYA_ENCODER_USER_HANDLER tuya_open_sw_encoder(TUYA_VIDEO_ENCODER_PARAM *param);
/**
 * \fn int tuya_close_encoder(TUYA_DECODER_USER_HANDLER _handler)
 * \brief close encoder ,and release all resources 
 * \param[in] _handler: the handler which generates from tuya_close_encoder() 
 * \return int : if ture,return 0; if false,return <0
 */
int tuya_set_and_start_enc_one_frame(TUYA_DECODER_USER_HANDLER _handler,
                                        unsigned char *in_buf,int in_size,long long in_pts);
int tuya_set_and_start_enc_one_frame_with_get_output(TUYA_DECODER_USER_HANDLER _handler,
                                        unsigned char *in_buf,int in_size,long long in_pts,
                                        unsigned char *out_buf,int *out_size,long long *out_pts);
int tuya_set_and_start_enc_one_frame(TUYA_DECODER_USER_HANDLER _handler,
                                        unsigned char *in_buf,int in_size,long long in_pts);
int tuya_close_encoder(TUYA_DECODER_USER_HANDLER _handler);

#ifdef __cplusplus
}
#endif

#endif