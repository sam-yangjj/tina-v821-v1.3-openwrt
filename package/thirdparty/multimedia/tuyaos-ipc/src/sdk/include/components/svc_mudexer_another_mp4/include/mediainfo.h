/*
* This file is created to integrate all the definitions about
* media, like codec and other info. We should use this all everywhere
* that need similar definitions to avoid convertion between different
* definitions from different module.
*/

#ifndef _MEDIAINFO_H_
#define _MEDIAINFO_H_

#if defined(__cplusplus)
extern "C"
{
#endif

//***********************************************
// codec definitions
//***********************************************
#define TP_AVCODEC_UNDET            -1

/***************** video codec ******************/
#define TP_AVCODEC_H264             100
#define TP_AVCODEC_H265             101

/***************** audio codec ******************/
// AAC codec
#define TP_AVCODEC_AAC_ADTS         200
#define TP_AVCODEC_AAC_LATM         201
#define TP_AVCODEC_AAC_ADIF         202
// PCM codec
#define TP_AVCODEC_PCM_ALAW         210
#define TP_AVCODEC_PCM_MULAW        211

/***************** adts channel_configuration ******************/
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_1        1 // 1 channel : front - center
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_2        2 // 2 channels : front - left, front - right
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_3        3 // 3 channels : front - center, front - left, front - right
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_4        4 // 4 channels : front - center, front - left, front - right, back - center
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_5        5 // 5 channels : front - center, front - left, front - right, back - left, back - right
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_6        6 // 6 channels : front - center, front - left, front - right, back - left, back - right, LFE - channel
#define TP_AVCODEC_ADTS_CHANNEL_CONFIG_7        7 // 8 channels : front - center, front - left, front - right, side - left, side - right, back - left, back - right, LFE - channel

//***********************************************
// codec related definitions
//***********************************************
#define TP_AUDIO_ADTS_SAMPLES_PER_FRAME     1024

#define TP_MAX_SPS_LENGTH           256
#define TP_MAX_PPS_LENGTH           128
#define TP_MAX_VPS_LENGTH           128

#define TP_MAX_EXTRA_INFO_LENGTH    (TP_MAX_SPS_LENGTH + TP_MAX_PPS_LENGTH + TP_MAX_VPS_LENGTH)

#define TP_NALU_STARTCODE_LENGTH    4

#define TP_AAC_ADTS_HEADER_LENGTH   7

typedef struct TPMEDIAINFO {
    long long lRangeFrom;   //媒体流从多少毫秒开始
    long long lRangeTo;     //媒体流从多少毫秒结束
    char pcVideoFormat[16];
    int iVideoSampleRate;
    unsigned char pucExtraInfo[128];

    unsigned char pucH264SPS[64];
    int iH264SPSLength;
    unsigned char pucH264PPS[64];
    int iH264PPSLength;

    char pcAudioFormat[16]; //音频格式
    int iAudioSampleRate;
    int iAudioNumChannels;
} TPMEDIAINFO;

#if defined(__cplusplus)
}
#endif
#endif
