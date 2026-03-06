#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_media_stream_event.h"
#include "ty_sdk_common.h"

#include "tal_audio.h"
#include "tal_video_enc.h"
#include "tal_thread.h"
#include "tal_system.h"

RING_BUFFER_USER_HANDLE_T s_ring_buffer_handles[E_IPC_STREAM_MAX] = {
    NULL,
};

void tuya_video_stream_cb(TKL_VENC_FRAME_T *frame, int channel)
{
    //0:E_IPC_STREAM_VIDEO_MAIN  1:E_IPC_STREAM_VIDEO_SUB
    if (s_ring_buffer_handles[channel] == NULL) {
        s_ring_buffer_handles[channel] = tuya_ipc_ring_buffer_open(0, 0, channel, E_RBUF_WRITE);
    }

    if (s_ring_buffer_handles[channel]) {
        tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[channel], frame->pbuf,
                                            frame->used_size, frame->frametype, frame->pts);

    } else {
        PR_ERR("tuya_ipc_ring_buffer_open failed,channel:%d\n", channel);
    }
}

void tuya_audio_stream_cb(TKL_AUDIO_FRAME_INFO_T *frame, int channel)
{
    //9: E_IPC_STREAM_AUDIO_MAIN
    channel = E_IPC_STREAM_AUDIO_MAIN;

    if (s_ring_buffer_handles[channel] == NULL) {
        s_ring_buffer_handles[channel] = tuya_ipc_ring_buffer_open(0, 0, channel, E_RBUF_WRITE);
    }

    if (s_ring_buffer_handles[channel]) {
        tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[channel], frame->pbuf,
                                            frame->used_size, frame->type, frame->pts);

    } else {
        PR_ERR("tuya_ipc_ring_buffer_open failed,channel:%d\n", channel);
    }
}


// void * __video_main(void *args)
// {
//     UCHAR_T *frame_buff = NULL;
//     // int bufSize = 138*1024; // 1280 * 720 * 3 / 2 /10
// #ifdef IPC_DUAL
//     int bufSize = 2*305*1024; // 2 * 1920 * 1080 * 3 / 2 /10
// #else
//     int bufSize = 305*1024; // 1920 * 1080 * 3 / 2 /10
// #endif
//     int naluSeiLen = 0;
//     TAL_VENC_FRAME_T frame = {0};

//     frame_buff = (UCHAR_T *)malloc(bufSize);
//     if (NULL == frame_buff) {
//         PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_VIDEO_MAIN);
//         return NULL;
//     }
//     frame.pbuf = (char *)frame_buff;
//     frame.buf_size = bufSize;
//     while(1){
//         if (0 == tal_venc_get_frame(0, 0, &frame)) {

//             if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] == NULL) {
//                 s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_VIDEO_MAIN, E_RBUF_WRITE);
//             }

//             if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN]) {
//                 tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN], frame_buff,
//                                                  frame.used_size + naluSeiLen, frame.frametype, frame.pts);

//             } else {
//                 PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_VIDEO_MAIN);
//             }
//         } else {
//             tal_system_sleep(10);
//         }
//     }
//     free(frame_buff);
//     if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] != NULL) {
//         tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN]);
//     }
//     return NULL;
// }

// void * __video_sub(void *args)
// {
//     UCHAR_T *frame_buff = NULL;
//     // int bufSize = MAX_MEDIA_FRAME_SIZE;
//     int bufSize = 46*1024;
//     int naluSeiLen = 0;
//     TAL_VENC_FRAME_T frame = {0};

//     frame_buff = (UCHAR_T *)malloc(bufSize);
//     if (NULL == frame_buff) {
//         PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_VIDEO_SUB);
//         return NULL;
//     }
//     frame.pbuf = (char *)frame_buff;
//     frame.buf_size = bufSize;
//     while(1){
//         if (0 == tal_venc_get_frame(0, 2, &frame)) {

//             if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] == NULL) {
//                 s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_VIDEO_SUB, E_RBUF_WRITE);
//             }

//             if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB]) {
//                 tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB], frame_buff,
//                                                  frame.used_size + naluSeiLen, frame.frametype, frame.pts);

//             } else {
//                 PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_VIDEO_SUB);
//             }
//         } else {
//             tal_system_sleep(10);
//         }
//     }
//     free(frame_buff);
//     if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] != NULL) {
//         tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB]);
//     }
//     return NULL;
// }

// void * __audio_main(void *args)
// {
//     UCHAR_T *frame_buff = NULL;
//     int bufSize = 512;
//     TAL_AUDIO_FRAME_INFO_T frame = {0};
//     frame_buff = (UCHAR_T *)malloc(bufSize);
//     if (NULL == frame_buff) {
//         PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_AUDIO_MAIN);
//         return NULL;
//     }
//     frame.pbuf = (char *)frame_buff;
//     frame.buf_size = bufSize;
//     while(1){
//         if (0 ==  tal_ai_get_frame(0, 0, &frame)) {
//             if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] == NULL) {
//                 s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_AUDIO_MAIN, E_RBUF_WRITE);
//             }

//             if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN]) {
//                 tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN], frame_buff, frame.used_size,
//                                                      frame.type, frame.pts);

//             } else {
//                 PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_AUDIO_MAIN);
//             }
//         }
//     }
//     free(frame_buff);
//     if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] != NULL) {
//         tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN]);
//     }
//     return NULL;
// }


VOID TUYA_IPC_av_start()
{
    int ret = 0;

    printf("create av task!!!\n"); 
    ret = tal_vi_init(NULL, 1);
    if (0 != ret) {
        PR_ERR("ty_dsp_init failed,%d\n", ret);
        return ;
    }

    ret = tal_venc_init(0, NULL, 4);
    if (0 != ret) {
        PR_ERR("ty_dsp_init failed,%d\n", ret);
        return ;
    }

    ret = tal_ai_init(-1, 1);
    if (0 != ret) {
        PR_ERR("tal_ai_init failed,%d\n", ret);
        return ;
    }

    // THREAD_CFG_T video_main_thrd_param;
    // video_main_thrd_param.priority = THREAD_PRIO_2;
    // video_main_thrd_param.stackDepth = 512 * 1024;
    // video_main_thrd_param.thrdname = "video_main";
    // THREAD_HANDLE video_main_thrd_hdl;
    // tal_thread_create_and_start(&video_main_thrd_hdl, NULL, NULL, __video_main, NULL, &video_main_thrd_param);

    // THREAD_CFG_T video_sub_thrd_param;
    // video_sub_thrd_param.priority = THREAD_PRIO_2;
    // video_sub_thrd_param.stackDepth = 512 * 1024;
    // video_sub_thrd_param.thrdname = "video_sub";
    // THREAD_HANDLE video_sub_thrd_hdl;
    // tal_thread_create_and_start(&video_sub_thrd_hdl, NULL, NULL, __video_sub, NULL, &video_sub_thrd_param);

    // THREAD_CFG_T audio_main_thrd_param;
    // audio_main_thrd_param.priority = THREAD_PRIO_2;
    // audio_main_thrd_param.stackDepth = 256 * 1024;
    // audio_main_thrd_param.thrdname = "audio_main";
    // THREAD_HANDLE audio_main_thrd_hdl;
    // tal_thread_create_and_start(&audio_main_thrd_hdl, NULL, NULL, __audio_main, NULL, &audio_main_thrd_param);

    return ; 
}
