#ifndef __DEMO_FOR_MP4_STORAGE_H__
#define __DEMO_FOR_MP4_STORAGE_H__

#include <stdio.h>
#include <pthread.h>
#include "mp4_mux.h"
#include "mp4_demux.h"
#include "tp_enc.h"

#define MP4_STORAGE_OUTPUT_FILE_ADDRESS "/tmp/test.mp4"
#define AUDIO_RECORD_SUPPORT
#define AUDIO_SAMPLE_RATE 8000
typedef enum
{
    MEDIA_TYPE_VIDEO = 0,
    MEDIA_TYPE_SOUND,
    MEDIA_TYPE_NUM,
}STREAMTYPR;

typedef struct MP4_STORAGE_STRU
{
    MP4MUXCONTEXT mp4_mux_context;
    MBUFFERESBuffer v_input_buf;
#ifdef AUDIO_RECORD_SUPPORT
    MBUFFERESBuffer a_input_buf;
#endif
    int fd;
    int if_first_gop;
    int if_file_full;
    int if_mp4_context_initial;
    pthread_mutex_t mutex; // Lock for using and destroying mp4 context
} MP4_STORAGE, *PTR_MP4_STORAGE;

int mp4_storage_update_index();
int mp4_storage_update_all_indexes();
int mp4_storage_check_file_size(unsigned int frame_cnt, unsigned int gop_size);
int mp4_storage_save_frame(TP_FRAME_S *tp_frame);
int mp4_storage_save_mp4(unsigned char *p_gop_buf, unsigned int gop_size);
int mp4_storage_mp4_init(unsigned int fd, int frame_cnt_in_fragment, unsigned int video_codec_type,unsigned int audio_codec_type);
int mp4_storage_mp4_destroy();
int mp4_storage_init();
int mp4_storage_destroy();

/* MP4 parse */
typedef struct MP4_PARSE_STRU
{
    MP4DEMUXCONTEXT mp4_demux_context;
    MBUFFERESBuffer v_output_buf;
    MBUFFERESBuffer a_output_buf;

    TPWFile p_input;
	int stm_fd;
} MP4_PARSE, *PTR_MP4_PARSE;

void mp4_parse_get_codec(int *video_codec, int *audio_codec, PTR_MP4_PARSE p_mp4_parse_context);
// pts: us
int mp4_parse_get_one_frame(unsigned char **data, unsigned int *data_size, unsigned long long *pts,
    unsigned char *frame_type, PTR_MP4_PARSE p_mp4_parse_context, int *iframe);
int mp4_parse_init(char *file_path, PTR_MP4_PARSE p_mp4_parse_context);
int mp4_parse_destroy(PTR_MP4_PARSE p_mp4_parse_context);
int mp4_parse_check_file_valid(char *p_file_name);

#endif /* __DEMO_FOR_MP4_STORAGE_H__ */
