/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**

测试参数的说明：
(1)pcm_file_path：指定目标pcm文件的路径，该文件是包含wave头的wav格式文件。
(2)pcm_sample_rate：指定采样率，通常设置为8000。
(3)pcm_channel_cnt：指定通道数目，通常为1或2。
(4)pcm_bit_width：指定位宽，必须设置为16。
(5)pcm_frame_size：指定frame_size，此值可不指定。
(6)mic_num:麦克风的数量。mic_num和pcm_channel_cnt有一定的联系，但也不是必然一一对应。一般一个mic对应一个channel。
    2mic1ref模式下的回声消除，2个mic对应2个audio channel，再加上回采通道，最后回声消除为1个channel输出。
    在alsa plug插件的帮助下，即使底层为1个mic的单通道，经过plug插件后，也可以变换为2个channel输出。但2mic1ref模式，我们
    不会启用plug插件。
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "sample_ai"
#include <utils/plat_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include <mm_common.h>
#include <media_common_aio.h>
#include <mpi_sys.h>
#include <mpi_ai.h>


#include "sample_ai.h"

#include <cdx_list.h>
#include "rpmsg.h"
#include "librpbuf.h"
#include <fcntl.h>

//uvoice
#include <stdarg.h>
#include "uvoice.h"
#include "uvoice_license.h"

typedef struct {
    uint32_t mSeq;
    uint16_t crc;
} AudioFrameHeader;

#define UVOICE_LOG_ARG_CHARNUM  (2048)
//uvoice end

#if 1

#define NUM_BLOCKS 6          // 小缓存块数量，总缓存大小 = BLOCK_SIZE * NUM_BLOCKS
#define RPBUF_NAME "rpbuf_test"
#define BLOCK_SIZE (sizeof(AudioFrameHeader) + 3840)     // 每次实际发送大小。头+数据
#define RPBUF_TOTAL_LEN NUM_BLOCKS * BLOCK_SIZE  // RPBUF缓冲区固定大小

typedef enum {
    BLOCK_EMPTY,
    BLOCK_FULL,
    BLOCK_SENDING
} BlockStatus;


typedef struct {
    uint8_t data[BLOCK_SIZE];
    BlockStatus status;
    int block_id;
} DataBlock;

// 在原有宏定义后补充
#define RPBUF_CTRL_ID 0  // RPBUF控制器ID

static void reset_block(DataBlock *block) {
    block->status = BLOCK_EMPTY;
}


typedef struct {
    DataBlock blocks[NUM_BLOCKS];
    int write_pos;  // 下一个可写位置（0~NUM_BLOCKS-1）
    int read_pos;   // 下一个可读位置（0~NUM_BLOCKS-1）
    int count;      // 当前使用的块数量（0~NUM_BLOCKS）
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    rpbuf_buffer_t *rpbuf_buffer;
    uint8_t *base_addr;
} BlockManager;


BlockManager g_block_manager;
volatile int g_running = 1;

int block_manager_init(BlockManager *bm) {
    if (!bm) return -1;


    for (int i = 0; i < NUM_BLOCKS; i++) {
        bm->blocks[i].status = BLOCK_EMPTY;
    }

    bm->write_pos = 0;
    bm->read_pos = 0;
    bm->count = 0;

    if (pthread_mutex_init(&bm->mutex, NULL) != 0 ||
        pthread_cond_init(&bm->cond, NULL) != 0) {
        return -1;
    }

    bm->rpbuf_buffer = rpbuf_alloc_buffer(RPBUF_CTRL_ID, RPBUF_NAME,
                                         NUM_BLOCKS * BLOCK_SIZE);
    if (!bm->rpbuf_buffer) {
        printf("RPBUF alloc failed\n");
        return -1;
    }
    bm->base_addr = rpbuf_buffer_addr(bm->rpbuf_buffer);
    return 0;
}

int block_manager_write(BlockManager *bm, const void *data, size_t len) {
    //if (!bm || !data || len != BLOCK_SIZE) return -1;

    pthread_mutex_lock(&bm->mutex);

    if (bm->count >= NUM_BLOCKS) {
        pthread_mutex_unlock(&bm->mutex);
        printf("buffer full(%d/%d),dropped data\n", bm->count, NUM_BLOCKS);
        return -1;
    }

    uint8_t *write_addr = bm->base_addr + bm->write_pos * BLOCK_SIZE;
    memcpy(write_addr, data, BLOCK_SIZE);
    bm->blocks[bm->write_pos].status = BLOCK_FULL;

    bm->write_pos = (bm->write_pos + 1) % NUM_BLOCKS;
    bm->count++;

    //printf("w block %d（now used: %d/%d, w point: %d, r point: %d）\n",
     //      (bm->write_pos - 1 + NUM_BLOCKS) % NUM_BLOCKS,
     //      bm->count, NUM_BLOCKS,
     //      bm->write_pos, bm->read_pos);

    pthread_cond_signal(&bm->cond);
    pthread_mutex_unlock(&bm->mutex);
    return 0;
}

void *rpbuf_send_thread(void *arg) {
    BlockManager *bm = &g_block_manager;

    while (g_running) {
        pthread_mutex_lock(&bm->mutex);

        while (g_running && bm->count == 0) {
            struct timespec ts = {0};
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 50000000;  // 50ms超时
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            pthread_cond_timedwait(&bm->cond, &bm->mutex, &ts);
        }

        if (!g_running) break;

        int send_pos = bm->read_pos;
        bm->blocks[send_pos].status = BLOCK_SENDING;
        pthread_mutex_unlock(&bm->mutex);

        size_t offset = send_pos * BLOCK_SIZE;
        int ret = rpbuf_transmit_buffer(bm->rpbuf_buffer, offset, BLOCK_SIZE);
        if (ret < 0) {
            printf("block %d send failed，retry...\n", send_pos);
            usleep(10000);
            ret = rpbuf_transmit_buffer(bm->rpbuf_buffer, offset, BLOCK_SIZE);
        }

        pthread_mutex_lock(&bm->mutex);
        bm->blocks[send_pos].status = BLOCK_EMPTY;
        bm->read_pos = (bm->read_pos + 1) % NUM_BLOCKS;
        bm->count--;

        //printf("release %d（now used: %d/%d, w point: %d, r point: %d）\n",
        //       send_pos, bm->count, NUM_BLOCKS,
        //       bm->write_pos, bm->read_pos);

        pthread_mutex_unlock(&bm->mutex);
		usleep(12000);
    }

    pthread_mutex_unlock(&bm->mutex);
    return NULL;
}



void rpbuf_block_deinit() {
    g_running = 0;
    pthread_cond_signal(&g_block_manager.cond);
    usleep(100000);

    if (g_block_manager.rpbuf_buffer) {
        rpbuf_free_buffer(g_block_manager.rpbuf_buffer);
        g_block_manager.rpbuf_buffer = NULL;
        g_block_manager.base_addr = NULL;
    }

    pthread_mutex_destroy(&g_block_manager.mutex);
    pthread_cond_destroy(&g_block_manager.cond);
}

size_t send_block_to_rtos(const void *data, size_t len) {
    if (block_manager_write(&g_block_manager, data, len) == 0) {
        return len;
    }
    return 0;
}


int recv_flag = 0;
void *rpmsg_recv_thread(void *arg) {
    char recv_buf[64] = {0};

    while (g_running) {
	int index = 0;
	float conf = .0f;
        int ret = tkl_wifi_rpmsg_recv(recv_buf, 32);
		if(recv_buf != NULL)
		{
			//recv_flag = 1;
//			printf("recv_buf  = %s\n",recv_buf);
			//uvoice
			UVoice_Sdk_Process(recv_buf, 64);
			//uvoice end
		}
		usleep(10000);
    }

    return NULL;
}

int rpbuf_block_init() {
    if (block_manager_init(&g_block_manager) != 0) {
        printf("Block manager init failed\n");
        return -1;
    }

    pthread_t send_tid;
    if (pthread_create(&send_tid, NULL, rpbuf_send_thread, NULL) != 0) {
        printf("Create send thread failed\n");
        return -1;
    }
	pthread_t recv_tid;
    if (pthread_create(&recv_tid, NULL, rpmsg_recv_thread, NULL) != 0) {
        printf("Create send thread failed\n");
        return -1;
    }
    pthread_detach(send_tid);
    pthread_detach(recv_tid);

    return 0;
}
#endif


static ERRORTYPE loadSampleAIConfig(SampleAIConfig *pConfig, const char *conf_path)
{
    int ret = SUCCESS;

    strcpy(pConfig->mPcmFilePath, "/media/test.wav");
    pConfig->mSampleRate = 16000;
    pConfig->mMicNum = 1;
    pConfig->mChannelCnt = 1;
    pConfig->mBitWidth = 16;
    pConfig->mFrameSize = 1024;
    pConfig->mCapDuraSec = 10;
    pConfig->ai_gain = 80;
    pConfig->mAiAnsEn = 0;
    pConfig->mAiAnsMode = 0;
    pConfig->mAiAgcEn = 0;
    pConfig->mAiAgcTargetDb = 0;
    pConfig->mAiAgcMaxGainDb = 30;

    alogd("para: mSampleRate[%d], mBitWidth[%d], mChannelCnt[%d], mFrameSize[%d], mCapTime[%d]s", pConfig->mSampleRate,
        pConfig->mBitWidth, pConfig->mChannelCnt, pConfig->mFrameSize, pConfig->mCapDuraSec);
    return ret;
}

void config_AIO_ATTR_S_by_SampleAIConfig(AIO_ATTR_S *dst, SampleAIConfig *src)
{
    memset(dst, 0, sizeof(AIO_ATTR_S));
    dst->enSamplerate = map_SampleRate_to_AUDIO_SAMPLE_RATE_E(src->mSampleRate);
    dst->enBitwidth = map_BitWidth_to_AUDIO_BIT_WIDTH_E(src->mBitWidth);
    if(1 == src->mChannelCnt)
    {
        dst->enSoundmode = AUDIO_SOUND_MODE_MONO;
    }
    else if(2 == src->mChannelCnt)
    {
        dst->enSoundmode = AUDIO_SOUND_MODE_STEREO;
    }
    else
    {
        aloge("fatal error! channel count[%d] wrong", src->mChannelCnt);
        dst->enSoundmode = AUDIO_SOUND_MODE_MONO;
    }
    dst->mMicNum = src->mMicNum;
    dst->mChnCnt = src->mChannelCnt;
    dst->ai_ans_en = src->mAiAnsEn;
    dst->ai_ans_mode = src->mAiAnsMode;
    dst->ai_agc_en = src->mAiAgcEn;

    dst->ai_aec_en = 1; //一定要开aec 。因为aec 才会有ref 参考音出来；
    dst->aec_delay_ms = 0;
    dst->mAecNlpMode = 0;

    /*
     * agc param
     * SampleRate > 8000Hz
     * Channel only support 1 or 2 channels
     * SampleLen is the frame, it will be match to input len
     * BitWidth Only Support 16bit
     * TargetDb range: [-30~0], Support float.
     * MaxGaintDb range: [0~95], Support float.
     */
    if(dst->ai_agc_en)
    {
        dst->ai_agc_float_cfg.fTargetDb = src->mAiAgcTargetDb;
        dst->ai_agc_float_cfg.fMaxGainDb = src->mAiAgcMaxGainDb;
    }
}

void PcmDataAddWaveHeader(SampleAIContext *ctx)
{
    struct WaveHeader{
        int riff_id;
        int riff_sz;
        int riff_fmt;
        int fmt_id;
        int fmt_sz;
        short audio_fmt;
        short num_chn;
        int sample_rate;
        int byte_rate;
        short block_align;
        short bits_per_sample;
        int data_id;
        int data_sz;
    } header;
    SampleAIConfig *pConf = &ctx->mConfigPara;

    memcpy(&header.riff_id, "RIFF", 4);
    header.riff_sz = ctx->mPcmSize + sizeof(struct WaveHeader) - 8;
    memcpy(&header.riff_fmt, "WAVE", 4);
    memcpy(&header.fmt_id, "fmt ", 4);
    header.fmt_sz = 16;
    header.audio_fmt = 1;       // s16le
    header.num_chn = pConf->mChannelCnt;
    header.sample_rate = pConf->mSampleRate;

    if(pConf->mBitWidth == 24) // the data captured from driver is 32bits when set 24 bitdepth
    {
        header.byte_rate = pConf->mSampleRate * pConf->mChannelCnt * 32/8;
        header.block_align = pConf->mChannelCnt * 32/8;
        header.bits_per_sample = 32;
    }
    else
    {
        header.byte_rate = pConf->mSampleRate * pConf->mChannelCnt * pConf->mBitWidth/8;
        header.block_align = pConf->mChannelCnt * pConf->mBitWidth/8;
        header.bits_per_sample = pConf->mBitWidth;
    }
    memcpy(&header.data_id, "data", 4);
    header.data_sz = ctx->mPcmSize;

    if(ctx->mFpPcmFile) {
        fseek(ctx->mFpPcmFile, 0, SEEK_SET);
        fwrite(&header, 1, sizeof(struct WaveHeader), ctx->mFpPcmFile);
    }
}

#define MAX_3SEC_BUF_SIZE (16000 * 1 * 2 * 10)
typedef enum {
    RPBUF_UNKNOWN = -1,  // 未知命令
    RPBUF_OK = 0,        // 操作成功
    RPBUF_ERROR,         // 操作失败
    RPBUF_START,         // 开始信号
    RPBUF_STOP           // 停止信号
} V821_MSG_ID_E;


int send_rpmsg_command(V821_MSG_ID_E cmd) {
    if (cmd < RPBUF_OK || cmd > RPBUF_STOP) {
        printf("Invalid command: %d\n", cmd);
        return -1;
    }

    int ret = tkl_wifi_rpmsg_send(&cmd, sizeof(V821_MSG_ID_E));
    if (ret != 0) {
        printf("Failed to send command: %d\n", cmd);
        return -1;
    }

    printf("Sent command: %d (%s)\n", cmd,
           cmd == RPBUF_OK ? "RPBUF_OK" :
           cmd == RPBUF_ERROR ? "RPBUF_ERROR" :
           cmd == RPBUF_START ? "RPBUF_START" :
           cmd == RPBUF_STOP ? "RPBUF_STOP" : "UNKNOWN");
    return 0;
}
int send_rpbuf_start_with_len(int total_len)
{

    char send_buf[64];

    int len = snprintf(send_buf, sizeof(send_buf), "%d %d", RPBUF_START, total_len);

    if (len <= 0 || len >= sizeof(send_buf)) {
        printf("Failed to format RPBUF_START command\n");
        return -1;
    }

    int ret = tkl_wifi_rpmsg_send(send_buf, len);
    if (ret < 0) {
        printf("tkl_wifi_rpmsg_send failed (ret: %d)\n", ret);
        return ret;
    }

    printf("Sent RPBUF_START with len: %d (data: %s)\n", total_len, send_buf);
    return 0;
}

static FILE *g_pcm_file = NULL;
#define PCM_FILE_PATH "/mnt/extsd/audio_recording.pcm"

static void init_pcm_file(void) {
    if (g_pcm_file == NULL) {
        g_pcm_file = fopen(PCM_FILE_PATH, "ab");
        if (g_pcm_file == NULL) {
            printf("Failed to open PCM file: %s", PCM_FILE_PATH);
        } else {
            printf("PCM file opened successfully: %s\n", PCM_FILE_PATH);
        }
    }
}

static void close_pcm_file(void) {
    if (g_pcm_file != NULL) {
        fclose(g_pcm_file);
        g_pcm_file = NULL;
        printf("PCM file closed successfully: %s\n", PCM_FILE_PATH);
    }
}

static uint16_t crc16(const unsigned char *data, uint32_t len) {
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static ERRORTYPE SampleAec_AudioDevCallback(void *cookie, AUDIO_DEV nAudioDevId, AudioDevEventType eEventType, int nPara0, void *pEventData)
{
    ERRORTYPE ret = SUCCESS;
    SampleAIContext *pContext = (SampleAIContext*)cookie;
    switch(eEventType)
    {
		case AudioDevEvent_RawCapData: //这里会回调mic数据和ref数据。[数据格式：mic/ref/mic/ref/mic/ref]
        {
        	AUDIO_FRAME_S *pstRawFrm = (AUDIO_FRAME_S*)pEventData;
			if (pstRawFrm == NULL || pstRawFrm->mpAddr == NULL || pstRawFrm->mLen <= 0) {
                aloge("Invalid audio frame data!");
                return FAILURE;
            }

            if (pstRawFrm->mBitwidth != 1) {
                aloge("Only 16bit audio is supported! Current: %dbit", pstRawFrm->mBitwidth);
                return FAILURE;
            }

            const int sample_size = 2; //16bit 大小占2个字节
            const int stereo_sample_size = 4; // 这里一组数据4个字节（包含mic和参考音）
            const int total_samples = pstRawFrm->mLen / stereo_sample_size;
            const int single_channel_len = total_samples * sample_size; //单mic数据长度

            unsigned char *pRecombinedBuf = (unsigned char*)malloc(pstRawFrm->mLen);
            if (pRecombinedBuf == NULL) {
                aloge("Failed to allocate recombined buffer!");
                return FAILURE;
            }

            //把数据整理成[mic/mic/mic.../ref/ref/ref/...]
            short *pRawData = (short*)pstRawFrm->mpAddr;
            short *pActual = (short*)pRecombinedBuf;
            short *pRef = (short*)(pRecombinedBuf + single_channel_len);
            for (int i = 0; i < total_samples; i++) {
                pActual[i] = pRawData[2 * i];
                pRef[i] = pRawData[2 * i + 1];
            }

#if 0 //保存音频
            if (g_pcm_file != NULL) {
                //size_t write_len = fwrite(pActual, 1, single_channel_len, g_pcm_file);
                //if (write_len != single_channel_len) {
                size_t write_len = fwrite(pRawData, 1, single_channel_len*2, g_pcm_file);
                if (write_len != single_channel_len*2) {
                    aloge("Failed to write PCM data: expected=%d, actual=%zu",
                          single_channel_len*2, write_len);
                } else {
                    static int total_written = 0;
                    total_written += write_len;
                    if (total_written % 1024 == 0) {
                        fflush(g_pcm_file);
                    }
                }
            }
#endif

#if 1
            int offset = 0;

			static uint32_t current_send_seq = 0;
            unsigned char *send_buf = (unsigned char*)malloc(BLOCK_SIZE);
            if (send_buf == NULL) {
                aloge("Failed to allocate send buffer!");
                free(pRecombinedBuf);
                return -1;
            }

            while (offset < pstRawFrm->mLen) {
                int data_len = (pstRawFrm->mLen - offset) > (BLOCK_SIZE - sizeof(AudioFrameHeader) ) ?
                              (BLOCK_SIZE - sizeof(AudioFrameHeader) ) : (pstRawFrm->mLen - offset);

#if 1
                uint16_t crc = crc16(pRecombinedBuf + offset, data_len);

                AudioFrameHeader *header = (AudioFrameHeader*)send_buf;
                header->mSeq = current_send_seq;
                header->crc = crc;

		if( current_send_seq % 399==0 ){
		    printf("[CRC] send_seq: %d\n",current_send_seq);
		    printf("[CRC] data_len: %d bytes?\n",data_len);
		    printf("[CRC] sample: ");
		    short* pcm = (short*)(pRecombinedBuf + offset);
		    for(int j=0;j<5;j++)
			    printf("%d,",pcm[j]);
		    printf("\n");
        }
#endif

                memcpy(send_buf + sizeof(AudioFrameHeader), pRecombinedBuf + offset, data_len);
#if 0 //测试写卡
            if (g_pcm_file != NULL) {
                size_t write_len = fwrite(pRecombinedBuf+offset, 1, data_len/2, g_pcm_file);
                if (write_len != data_len/2) {
                    aloge("Failed to write PCM data: expected=%d, actual=%zu",
                          data_len/2, write_len);
                } else {
                    static int total_written = 0;
                    total_written += write_len;
                    if (total_written % 1024 == 0) {
                        fflush(g_pcm_file);
                    }
                }
            }
#endif
                int total_send_len = sizeof(AudioFrameHeader) + data_len;

                int retry = 0;
                int actual_send = 0;
                while (retry <= 3) {
                    actual_send = send_block_to_rtos(send_buf, total_send_len);
                    //printf("actual_send = %d, total_send_len = %d, mSeq = %u\n",
                           //actual_send, total_send_len, current_send_seq);
                    if (actual_send == total_send_len) {
                        break;
                    }
                    retry++;
                    usleep(50000);
                }
				current_send_seq++;

                if (actual_send == total_send_len) {
                    offset += data_len;
                } else {
                    aloge("Failed to send block (mSeq=%u) after 3 retries", current_send_seq);
                    offset += (actual_send > sizeof(AudioFrameHeader)) ?
                             (actual_send - sizeof(AudioFrameHeader)) : 0;
                }

                usleep(10000);
            }
            free(send_buf);
#endif
            // 释放缓冲区
            free(pRecombinedBuf);
            break;
        }
        default:
        {
            aloge("fatal error! AudioDev[%d] callback unknown event:%d", nAudioDevId, eEventType);
            ret = FAILURE;
            break;
        }
    }
    return ret;
}

static void sigint_handler(int signum) {
    if (signum == SIGINT) {
        if (g_pcm_file != NULL) {
            fclose(g_pcm_file);
            g_pcm_file = NULL;
        }
        printf("sample_quit ok \n");
        exit(EXIT_SUCCESS);
    }
}

void register_sigint_handler() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        exit(EXIT_FAILURE);
    }
}

//uvoice
static const char* auth_cb(const char* body)
{
	/*curl -k -s --data-binary $body https://srv01.51asr.com:8007/asrsn_active2"*/
	char curl_cmd[1024];
	char curl_path[128];
	int rc;
	strcpy(curl_path, "/usr/bin/");
	rc = snprintf(curl_cmd,1024,"export LD_LIBRARY_PATH=.:lib:%s; %s/curl -k --cacert %s/ca-certificates.crt -s --data-binary '%s' %s", curl_path, curl_path, curl_path, body, "https://srv01.51asr.com:8007/asrsn_active2");
	printf("Body is:%s\n", body);
	printf("curl_cmd: %s\n", curl_cmd);
	if (rc > 0)
	{
		FILE* https_req = popen(curl_cmd, "r");
		rc = fread(curl_cmd, 1, 1024, https_req);
		if (rc > 0) curl_cmd[rc] = 0;
		return strdup(curl_cmd);
	}
	return "";
}

static int print_func(const char *format, ...)
{
	struct timeval tv;
	va_list vl;
	int ret;
	char argument[UVOICE_LOG_ARG_CHARNUM] = "\0";

	gettimeofday(&tv, NULL);
	printf("(%d.%3d)", (int)tv.tv_sec, (int)(tv.tv_usec * 1000));
	va_start(vl, format);
	ret = vsnprintf(argument, UVOICE_LOG_ARG_CHARNUM, format, vl);
	if (ret >= 0)
	{
		if (ret >= UVOICE_LOG_ARG_CHARNUM - 1)
		{
			ret = UVOICE_LOG_ARG_CHARNUM - 5;
			argument[ret++] = '.';
			argument[ret++] = '.';
			argument[ret++] = '.';
			argument[ret++] = '\n';
		}
		argument[ret] = '\0';
		printf("%s", argument);
	}
	va_end(vl);
	return 0;
}

//唤醒回调
void kws_cb(int id, float confidence)
{
    //if( (id>0) && (id<10) )
	printf("[%s]id:%d, confidence:%.2f\n", __func__, id, confidence);
    switch(id) {
        case 1:
            printf("唤醒词：你好小志\n");
            break;
        case 2:
            printf("唤醒词：打开屏幕\n");
            break;
        case 3:
            printf("唤醒词：关闭屏幕\n");
            break;
        case 4:
            printf("唤醒词：打开WIFI\n");
            break;
        case 5:
            printf("唤醒词：关闭WIFI\n");
            break;
        case 6:
            printf("唤醒词：我要拍照\n");
            break;
        case 7:
            printf("唤醒词：打开录音\n");
            break;
        case 8:
            printf("唤醒词：关闭录音\n");
            break;
        case 9:
            printf("唤醒词：我要录像\n");
            break;
        default:
            break;

    }

}
//uvoice end

int main(int argc, char *argv[])
{
    int result = 0;
    //uvoice
    int uvoice_ret;
	uv_activate_param activate_param;
	struct callback_function function_cb;
    //uvoice end
    alogd("Hello, sample_ai!");
    SampleAIContext stContext;
	//char recv_buf[64] = {0};
    memset(&stContext, 0, sizeof(SampleAIContext));
	V821_MSG_ID_E recv_cmd;
	register_sigint_handler() ;
	tkl_wifi_rpmsg_init();
    //确定小核先开启服务后，再运行这个
	if(access("/tmp/rpbuf_started", F_OK) == -1)
	{
		send_rpbuf_start_with_len(RPBUF_TOTAL_LEN);
		while(1)
		{
			tkl_wifi_rpmsg_recv(&recv_cmd, sizeof(V821_MSG_ID_E));
			printf("recv_cmd = %d\n", recv_cmd);
			switch(recv_cmd)
			{
				case RPBUF_OK:
					printf("rpbuf_creat success !\n");
					break;
				case RPBUF_ERROR:
					printf("rpbuf_creat failed !\n");
					break;
				default:
	                printf("Unknown command: %d\n", recv_cmd);
	                break;
			}
			if (recv_cmd == RPBUF_OK && rpbuf_block_init() == 0) {
				int fd = open("/tmp/rpbuf_started", O_CREAT | O_RDWR, 0644);
				if (fd != -1) close(fd);
		        break;
		    }
			usleep(1000);
		}
	}
	else
	{
		rpbuf_block_init();
	}
	//uvoice
	printf("UVoice_Sdk_GetVersion:%s\n", UVoice_Sdk_GetVersion());

	function_cb.kws_cb = kws_cb;
	UVoice_Sdk_SetCallback(&function_cb);
	UVoice_Sdk_SetLogFun(print_func);

    printf("========== update license from internel==========\n");
	activate_param.license = "099251101002c000c3f52ca4032d3629b3d2e40a2b6965fa";
	//activate_param.license = NULL ; //注册授权码，不注册的话，受限使用
	activate_param.license_path = "/mnt/extsd/test.lic"; //生成本机授权文件，后续不联网也可以使用(目录必须要有读写权限，不然会失败)
	activate_param.activate_type = 0;
	activate_param.auth_cb = auth_cb;
	uvoice_ret = UVoice_Sdk_Init((void*)(&activate_param));
	if (uvoice_ret < 0)
	{
		printf("UVoice_Sdk_Init error: %d\n", uvoice_ret);
		result = -1;
		goto _exit;
	}
	//uvoice end

    //parse config file.
    if(loadSampleAIConfig(&stContext.mConfigPara, NULL) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }

    //open pcm file
    stContext.mFpPcmFile = fopen(stContext.mConfigPara.mPcmFilePath, "wb");
    if(!stContext.mFpPcmFile)
    {
        aloge("fatal error! can't open pcm file[%s]", stContext.mConfigPara.mPcmFilePath);
        stContext.mFpPcmFile  = NULL;

    }
    else
    {
        alogd("sample_ai produce file: %s", stContext.mConfigPara.mPcmFilePath);
        if(stContext.mFpPcmFile)
            fseek(stContext.mFpPcmFile, 44, SEEK_SET); // 44: size(WavHeader)
    }

    //init mpp system
    stContext.mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&stContext.mSysConf);
    AW_MPI_SYS_Init();

    //enable ai dev
    stContext.mAIDev = 0;
    config_AIO_ATTR_S_by_SampleAIConfig(&stContext.mAIOAttr, &stContext.mConfigPara);
    AW_MPI_AI_SetPubAttr(stContext.mAIDev, &stContext.mAIOAttr);
    //embedded in AW_MPI_AI_CreateChn
    //AW_MPI_AI_Enable(stContext.mAIDev);
    AW_MPI_AI_RegisterDevCallback(stContext.mAIDev, (void*)&stContext, &SampleAec_AudioDevCallback); //设置回调。语音唤醒是从这里取数据。
    AW_MPI_AI_Enable(stContext.mAIDev);
	init_pcm_file();
    //create ai channel.
    ERRORTYPE ret;
    BOOL bSuccessFlag = FALSE;
    stContext.mAIChn = 0;
    stContext.mAIChnAttr.nFrameSize = stContext.mConfigPara.mFrameSize;
    while(stContext.mAIChn < AIO_MAX_CHN_NUM)
    {
        ret = AW_MPI_AI_CreateChn(stContext.mAIDev, stContext.mAIChn, &stContext.mAIChnAttr);
        if(SUCCESS == ret)
        {
            bSuccessFlag = TRUE;
            alogd("create ai channel[%d] success!", stContext.mAIChn);
            break;
        }
        else if (ERR_AI_EXIST == ret)
        {
            alogd("ai channel[%d] exist, find next!", stContext.mAIChn);
            stContext.mAIChn++;
        }
        else if(ERR_AI_NOT_ENABLED == ret)
        {
            aloge("audio_hw_ai not started!");
            break;
        }
        else
        {
            aloge("create ai channel[%d] fail! ret[0x%x]!", stContext.mAIChn, ret);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        stContext.mAIChn = MM_INVALID_CHN;
        aloge("fatal error! create ai channel fail!");
        goto _exit;
    }

    AW_MPI_AI_SetDevVolume(stContext.mAIDev,stContext.mConfigPara.ai_gain);

    int tmp_gain = 0;
    AW_MPI_AI_GetDevVolume(stContext.mAIDev,&tmp_gain);

    aloge("sampl_ai_gain_value:%d",tmp_gain);

    AW_MPI_AI_EnableDevSendCaptureFrame(stContext.mAIDev, 1);

    //start ai dev.
    AW_MPI_AI_EnableChn(stContext.mAIDev, stContext.mAIChn);

    int nWriteLen;
    AUDIO_FRAME_S nAFrame;
    SampleAIConfig *pAiConf = &stContext.mConfigPara;
    //cap pcm for xx s
    int nMaxWantedSize = 0;
    if(pAiConf->mBitWidth == 24)
    {
        nMaxWantedSize = pAiConf->mSampleRate * pAiConf->mChannelCnt * 32/8 * pAiConf->mCapDuraSec;
    }
    else
    {
        nMaxWantedSize = pAiConf->mSampleRate * pAiConf->mChannelCnt * pAiConf->mBitWidth/8 * pAiConf->mCapDuraSec;
    }

	short msg_buf[2100] = {0};
	int max_send_len = BLOCK_SIZE;
	int send_len = 0;

	static char g_3sec_buf[MAX_3SEC_BUF_SIZE] = {0};
	static int g_buf_offset = 0;

	while (1)
	{
        ret = AW_MPI_AI_GetFrame(stContext.mAIDev, stContext.mAIChn, &nAFrame, NULL, -1);
        if (SUCCESS == ret)
        {
            //这里获取的到的音频可以继续送编码或者其他处理，语音唤醒的不从这里取，从回调中取数据。

            ret = AW_MPI_AI_ReleaseFrame(stContext.mAIDev, stContext.mAIChn, &nAFrame, NULL);
            if (SUCCESS != ret)
            {
                aloge("release frame to ai fail! ret: %#x", ret);
            }
        }
        else
        {
            aloge("get pcm from ai in block mode fail! ret: %#x", ret);
            break;
        }
        usleep(10000);

		if(recv_flag == 1)
	        break;

	}
    if(stContext.mFpPcmFile) {
        PcmDataAddWaveHeader(&stContext);
        fclose(stContext.mFpPcmFile);
        stContext.mFpPcmFile = NULL;
    }

    //stop ai chn.
    AW_MPI_AI_DisableChn(stContext.mAIDev, stContext.mAIChn);
    //reset and destroy ai chn & dev.
    AW_MPI_AI_ResetChn(stContext.mAIDev, stContext.mAIChn);
    AW_MPI_AI_DestroyChn(stContext.mAIDev, stContext.mAIChn);
//    exit = 1;
//    pthread_join(tid, NULL);

    stContext.mAIDev = MM_INVALID_DEV;
    stContext.mAIChn = MM_INVALID_CHN;
     close_pcm_file();
    //uvoice
    UVoice_Sdk_Fini();
    //uvoice end

    //exit mpp system
    AW_MPI_SYS_Exit();

_exit:
    alogd("%s test result: %s", argv[0], ((0 == result) ? "success" : "fail"));
    return result;
}

