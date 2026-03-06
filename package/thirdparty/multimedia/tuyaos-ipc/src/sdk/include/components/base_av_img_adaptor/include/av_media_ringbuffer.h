#ifndef __AV_MEDIA_RINGBUFFER_H__
#define __AV_MEDIA_RINGBUFFER_H__

enum AV_MID_RINGBUFFER_FRAME_TYPE{
    AV_MID_RINGBUFFER_FRAME_TYPE_AUDIO_PCM,
    AV_MID_RINGBUFFER_FRAME_TYPE_VIDEO_H264,
    AV_MID_RINGBUFFER_FRAME_TYPE_VIDEO_H265,
    AV_MID_RINGBUFFER_FRAME_TYPE_VIDEO_YUV,
};

typedef struct AV_MID_FRMAE_NODE_INFO{
    int time_stamp;
    int pts;
    int frm_sz;
    unsigned char* frm_buf;
    int if_key_frame;
}AV_MID_FRMAE_NODE_INFO;

int av_mid_ringbuffer_init(enum AV_MID_RINGBUFFER_FRAME_TYPE type,int buf_len);
int av_media_ringbuffer_get_frame(int ringbuffer_id,AV_MID_FRMAE_NODE_INFO *info_out);
int av_media_ringbuffer_set_frame(int ringbuffer_id,AV_MID_FRMAE_NODE_INFO *info_in,int switch_frame_flag);
int av_mid_ringbuffer_init_release(int ringbuffer_id);
#endif /* __AV_MEDIA_RINGBUFFER_H__ */
