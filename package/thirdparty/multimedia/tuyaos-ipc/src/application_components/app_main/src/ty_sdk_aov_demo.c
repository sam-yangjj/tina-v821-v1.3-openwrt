
#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "tuya_iot_config.h"
#include "tal_time_service.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ring_buffer.h"
#include "uni_log.h"

#if !defined(ENABLE_OFFLINE_RESUME)||(ENABLE_OFFLINE_RESUME!=1)
#if defined(ENABLE_AOV_STORAGE)&&(ENABLE_AOV_STORAGE==1)
#include "tuya_ipc_aov_buffer.h"
#include "tuya_ipc_aov_cloud_storage.h"

// 测试文件，实际产品中AOV视频要和主码流视频参数保持一致
#define AOV_TEST_FILE_NAME "./application_components/app_main/resource/aov_video.265"

static int read_one_h265_frame_from_aov_file(unsigned char *buffer, unsigned int buf_size, unsigned int offset,
                    unsigned int *is_key_frame, unsigned int *frame_len, unsigned int *frame_start)
{
    int pos = 0;
    int bneed_cal = 0;
    unsigned char nal_type = 0;
    int idx = 0;

    if (buf_size <= 5) {
        printf("buf size is too small\n");
        return -1;
    }

    for (pos = 0; pos <= buf_size - 5; pos++) {
        if (buffer[pos] == 0x00 && buffer[pos+1] == 0x00 &&
            buffer[pos+2] == 0x00 && buffer[pos+3] == 0x01) {
            nal_type = (buffer[pos + 4] & 0x7E) >> 1;

            // vps
            if (nal_type == 32) {
                if (bneed_cal == 1) {
                    *frame_len = pos - idx;
                    return 0;
                }

                *is_key_frame = 1;
                *frame_start = offset + pos;
                bneed_cal = 1;
                idx = pos;
            } else if (nal_type == 0x1) {
                if (bneed_cal == 1) {
                    *frame_len = pos - idx;
                    return 0;
                }

                *is_key_frame = 0;
                *frame_start = offset + pos;
                idx = pos;
                bneed_cal = 1;
            }
        }
    }

    return -1;
}

// 只展示 AOV ringbuf 和 开启存储 的调用关系， 可根据实际需求调用和填写参数
void *thread_aov_video(void *arg)
{
    char path[128] = {0};
    unsigned char *video_buf = NULL;
    int file_size = 0;
    BOOL_T is_last_frame = FALSE;
    unsigned int frame_len = 0, frame_start = 0;
    unsigned int next_frame_len = 0, next_frame_start = 0;
    unsigned int offset = 0;
    unsigned int is_key_frame = 0;
    unsigned int next_is_key_frame = 0;
    FILE *fp = NULL;
    MEDIA_FRAME_TYPE_E frame_type;
    UINT64_T timestamp = tal_time_get_posix_ms() - 10 * 60 * 1000;  // 测试用，AOV 的时间段要合普通录像的时间段分开；产品中需要和实际录像时间一致
    AOV_BUFFER_USER_HANDLE_T aov_handle = NULL;

    AOV_BUFFER_INIT_PARAM_T param;
    param.buffer_size = 3 * 1024 * 1024;                                    // Ensure that the buffer size is larger than the AOV file
    tuya_ipc_aov_buffer_init(0, 0, E_IPC_STREAM_VIDEO_MAIN, &param);        // first, init AOV buffer

    // AOV local storage begins before frames are written to ring buf
    tuya_ipc_ss_start_aov_local_storage(E_IPC_STREAM_VIDEO_MAIN);

    // AOV cloud storage
    tuya_ipc_aov_cloud_storage_start();

    snprintf(path, sizeof(path), AOV_TEST_FILE_NAME);
    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("cant not read aov file %s\n", path);
        pthread_exit(0);
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    video_buf = (unsigned char *)malloc(file_size);
    if (video_buf == NULL) {
        printf("malloc aov video buf failed\n");
        fclose(fp);
        pthread_exit(0);
    }
    fread(video_buf, 1, file_size, fp);

    aov_handle = tuya_ipc_aov_buffer_open(0, 0, 0, E_AOV_BUF_WRITE);

    while (1) {
        offset = frame_start + frame_len;
        if (offset >= file_size) {
            break;
        }

        int ret = read_one_h265_frame_from_aov_file(video_buf + offset, file_size - offset, offset, &is_key_frame, &frame_len, &frame_start);
        if (ret) {
            break;
        }

        // Determine whether it is the last frame
        offset = frame_start + frame_len;
        ret = read_one_h265_frame_from_aov_file(video_buf + offset, file_size - offset, offset, &next_is_key_frame, &next_frame_len, &next_frame_start);
        if (ret) {
            is_last_frame = TRUE;           // important
        } else {
            is_last_frame = FALSE;
        }

        if (is_key_frame) {
            frame_type = E_VIDEO_I_FRAME;
        } else {
            frame_type = E_VIDEO_PB_FRAME;
        }

        // printf("append_data_with_timestamp:typ:%d,%llu,%u,%d\n", frame_type, timestamp, frame_len, is_last_frame);
        tuya_ipc_aov_buffer_append_data_with_timestamp(aov_handle, video_buf + frame_start, frame_len, frame_type, timestamp * 1000, timestamp, is_last_frame);
        timestamp += 2000;

        usleep(100);
    }

    free(video_buf);
    fclose(fp);
    tuya_ipc_aov_buffer_close(aov_handle);

    pthread_exit(0);
}

#endif
#endif

int ty_aov_video_test(void)
{
#if !defined(ENABLE_OFFLINE_RESUME)||(ENABLE_OFFLINE_RESUME!=1)
#if defined(ENABLE_AOV_STORAGE)&&(ENABLE_AOV_STORAGE==1)
    tuya_ipc_aov_cloud_storage_init();

    pthread_t aov_thread;
    pthread_create(&aov_thread, NULL, thread_aov_video, NULL);
    pthread_detach(aov_thread);
#endif
#endif

    return 0;
}