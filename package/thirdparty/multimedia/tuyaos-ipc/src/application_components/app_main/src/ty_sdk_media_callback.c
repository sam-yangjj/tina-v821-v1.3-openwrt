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
#include "tal_video_enc.h"
#include "tuya_ipc_album.h"
#include "tuya_list.h"
#include "tal_audio.h"
#include "tuya_g711_utils.h"

#define MAX_SNAPSHOT_BUFFER_SIZE_KB            (100)  //in KB

#define AUDIO_FRAME_SIZE 640
#define AUDIO_FPS 25
#define VIDEO_BUF_SIZE	(1024 * 400) //Maximum frame
#if defined(IPC_CHANNEL_NUM)
#define MAX_IPC_CHANNEL_NUM  IPC_CHANNEL_NUM
#else
#define MAX_IPC_CHANNEL_NUM  1
#endif

typedef struct {
    BOOL_T enabled;
    TRANSFER_VIDEO_CLARITY_TYPE_E live_clarity;
    UINT_T max_users;
    TUYA_CODEC_ID_E p2p_audio_codec;
}TUYA_APP_P2P_MGR;

STATIC TUYA_APP_P2P_MGR s_p2p_mgr = {0};

STATIC VOID __TUYA_APP_media_frame_TO_trans_video(IN CONST MEDIA_FRAME_T *p_in, INOUT MEDIA_VIDEO_FRAME_T *p_out)
{
    UINT_T codec_type = 0;
    codec_type = (p_in->type & 0xff00) >> 8;
    p_out->video_codec = (codec_type == 0 ? TUYA_CODEC_VIDEO_H264 : TUYA_CODEC_VIDEO_H265);
    p_out->video_frame_type = (p_in->type && 0xff) == E_VIDEO_PB_FRAME ? TUYA_VIDEO_FRAME_PBFRAME:TUYA_VIDEO_FRAME_IFRAME;
    p_out->p_video_buf = p_in->p_buf;
    p_out->buf_len = p_in->size;
    p_out->pts = p_in->pts;
    p_out->timestamp = p_in->timestamp;

    return;
}

STATIC VOID __TUYA_APP_media_frame_TO_trans_audio(IN CONST MEDIA_FRAME_T *p_in, INOUT MEDIA_AUDIO_FRAME_T *p_out)
{
    DEVICE_MEDIA_INFO_T media_info = {0};
    tuya_ipc_media_adapter_get_media_info(0, 0, &media_info);
    p_out->audio_codec = media_info.av_encode_info.audio_codec[E_IPC_STREAM_AUDIO_MAIN];
    p_out->audio_sample = media_info.av_encode_info.audio_sample[E_IPC_STREAM_AUDIO_MAIN];
    p_out->audio_databits = media_info.av_encode_info.audio_databits[E_IPC_STREAM_AUDIO_MAIN];
    p_out->audio_channel = media_info.av_encode_info.audio_channel[E_IPC_STREAM_AUDIO_MAIN];
    p_out->p_audio_buf = p_in->p_buf;
    p_out->buf_len = p_in->size;
    p_out->pts = p_in->pts;
    p_out->timestamp = p_in->timestamp;

    return;
}
STATIC INT_T TUYA_APP_Enable_Speaker_CB(IN BOOL_T enable)
{
    int ret;

    if (enable == TRUE) {
        ret = tal_ao_init(-1, 1, NULL);
        if (0 != ret) {
            PR_ERR("tal_ao_init failed,%d\n", ret);
        }
    } else {
        ret = tal_ao_uninit(NULL);
        if (0 != ret) {
            PR_ERR("tal_ao_uninit failed,%d\n", ret);
        }
    }

    return ret;
}

STATIC VOID __TUYA_APP_ss_pb_event_cb(IN UINT_T pb_idx, IN SS_PB_EVENT_E pb_event, IN PVOID_T args)
{
    PR_DEBUG("ss pb rev event: %u %d", pb_idx, pb_event);
    if (pb_event == SS_PB_FINISH) {
        tuya_ipc_media_playback_send_finish(pb_idx);
    }

    return;
}

STATIC VOID __TUYA_APP_ss_pb_get_video_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_T *p_frame)
{
    MEDIA_VIDEO_FRAME_T video_frame = {0};
    __TUYA_APP_media_frame_TO_trans_video(p_frame, &video_frame);
    tuya_ipc_media_playback_send_video_frame(pb_idx, &video_frame);

    return;
}

STATIC VOID __TUYA_APP_ss_pb_get_audio_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_T *p_frame)
{
    MEDIA_AUDIO_FRAME_T audio_frame = {0};
    __TUYA_APP_media_frame_TO_trans_audio(p_frame, &audio_frame);
    tuya_ipc_media_playback_send_audio_frame(pb_idx, &audio_frame);

    return;
}

STATIC VOID __TUYA_APP_ss_pb_get_video_encrypt_cb(IN UINT_T pb_idx, UINT_T reqId, IN CONST SS_MEDIA_FRAME_WITH_ENCRYPT_T *p_frame)
{
    TRANSFER_MEDIA_FRAME_WIHT_ENCRYPT_T trans_frame;

    memset(&trans_frame, 0, sizeof(TRANSFER_MEDIA_FRAME_WIHT_ENCRYPT_T));
    trans_frame.frame_type = p_frame->frame_type;
    trans_frame.p_buf = p_frame->p_buf;
    trans_frame.size = p_frame->size;
    trans_frame.pts = p_frame->pts;
    trans_frame.timestamp = p_frame->timestamp;

    // 在 MP4 方案中，video_codec, video_width, video_height 参数需要使用初始化的音视频参数，勿用 p_frame 的数值 ！！！！
    trans_frame.media.video.video_codec = p_frame->media.video.video_codec;
    trans_frame.media.video.frame_rate = p_frame->media.video.frame_rate;
    trans_frame.media.video.video_width = p_frame->media.video.video_width;
    trans_frame.media.video.video_height = p_frame->media.video.video_height;

    trans_frame.encrypt_info.encrypt = p_frame->encrypt_info.encrypt;
    trans_frame.encrypt_info.security_level = p_frame->encrypt_info.security_level;
    memcpy(&trans_frame.encrypt_info.uuid, &p_frame->encrypt_info.uuid, sizeof(trans_frame.encrypt_info.uuid));
    memcpy(&trans_frame.encrypt_info.iv, &p_frame->encrypt_info.iv, sizeof(trans_frame.encrypt_info.iv));
    tuya_ipc_media_playback_send_video_frame_with_encrypt(pb_idx,reqId, (TRANSFER_MEDIA_FRAME_WIHT_ENCRYPT_T *)&trans_frame);
    return;
}

STATIC VOID __TUYA_APP_ss_pb_get_audio_encrypt_cb(IN UINT_T pb_idx,int reqId,  IN CONST SS_MEDIA_FRAME_WITH_ENCRYPT_T *p_frame)
{
    tuya_ipc_media_playback_send_audio_frame_with_encrypt(pb_idx,reqId, (TRANSFER_MEDIA_FRAME_WIHT_ENCRYPT_T *)p_frame);
    return;
}
/* Callback functions for transporting events */
INT_T TUYA_IPC_p2p_event_cb(IN CONST INT_T device, IN CONST INT_T channel, IN CONST MEDIA_STREAM_EVENT_E event, IN PVOID_T args)
{
    int ret = 0;
    PR_DEBUG("p2p rev event cb=[%d] ", event);

    switch (event) {
    case MEDIA_STREAM_LIVE_VIDEO_START:
    {
        C2C_TRANS_CTRL_VIDEO_START *parm = (C2C_TRANS_CTRL_VIDEO_START *)args;
        PR_DEBUG("chn[%u] video start", parm->channel);
        break;
    }
    case MEDIA_STREAM_LIVE_VIDEO_STOP:
    {
        C2C_TRANS_CTRL_VIDEO_STOP *parm = (C2C_TRANS_CTRL_VIDEO_STOP *)args;
        PR_DEBUG("chn[%u] video stop", parm->channel);
        break;
    }
    case MEDIA_STREAM_LIVE_AUDIO_START:
    {
        C2C_TRANS_CTRL_AUDIO_START *parm = (C2C_TRANS_CTRL_AUDIO_START *)args;
        PR_DEBUG("chn[%u] audio start", parm->channel);
        break;
    }
    case MEDIA_STREAM_LIVE_AUDIO_STOP:
    {
        C2C_TRANS_CTRL_AUDIO_STOP *parm = (C2C_TRANS_CTRL_AUDIO_STOP *)args;
        PR_DEBUG("chn[%u] audio stop", parm->channel);
        break;
    }
    case MEDIA_STREAM_SPEAKER_START:
    {
        PR_DEBUG("enable audio speaker");
        TUYA_APP_Enable_Speaker_CB(TRUE);
        break;
    }
    case MEDIA_STREAM_SPEAKER_STOP:
    {
        PR_DEBUG("disable audio speaker");
        TUYA_APP_Enable_Speaker_CB(FALSE);
        break;
    }
    case MEDIA_STREAM_DISPLAY_START:
    {
        PR_DEBUG("enable video display");
        break;
    }
    case MEDIA_STREAM_DISPLAY_STOP:
    {
        PR_DEBUG("disable video display");
        break;
    }
    case MEDIA_STREAM_LIVE_VIDEO_SEND_PAUSE:
    {
        PR_DEBUG("app pause video");
        break;
    }
    case MEDIA_STREAM_LIVE_VIDEO_SEND_RESUME:
    {
        PR_DEBUG("app resume video");
        break;
    }
    case MEDIA_STREAM_LIVE_LOAD_ADJUST:
    {
        C2C_TRANS_LIVE_LOAD_PARAM_S *quality = (C2C_TRANS_LIVE_LOAD_PARAM_S *)args;
        PR_DEBUG("live quality %d -> %d", quality->curr_load_level, quality->new_load_level);
        break;
    }
    case MEDIA_STREAM_PLAYBACK_LOAD_ADJUST:
    {
        C2C_TRANS_PB_LOAD_PARAM_S *quality = (C2C_TRANS_PB_LOAD_PARAM_S *)args;
        PR_DEBUG("pb idx:%d quality %d -> %d", quality->client_index, quality->curr_load_level, quality->new_load_level);
        break;
    }
    case MEDIA_STREAM_ABILITY_QUERY:
    {
        C2C_TRANS_QUERY_FIXED_ABI_REQ *pAbiReq;
        pAbiReq = (C2C_TRANS_QUERY_FIXED_ABI_REQ *)args;
        pAbiReq->ability_mask = TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_VIDEO |
            TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_SPEAKER |
            TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_MIC;
        break;
    }
    case MEDIA_STREAM_PLAYBACK_QUERY_MONTH_SIMPLIFY:
    {
        C2C_TRANS_QUERY_PB_MONTH_REQ *p = (C2C_TRANS_QUERY_PB_MONTH_REQ *)args;
        PR_DEBUG("pb query by month: %d-%d", p->year, p->month);

        OPERATE_RET ret = tuya_ipc_pb_query_by_month(p->ipcChan, p->year, p->month, &(p->day));
        if (OPRT_OK != ret) {
            PR_ERR("pb query by month: %d-%d ret:%d", p->year, p->month, ret);
        }

            break;
        }
        case MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS:
        {
            C2C_TRANS_QUERY_PB_DAY_RESP *pquery = (C2C_TRANS_QUERY_PB_DAY_RESP *)args;
            PR_DEBUG("pb_ts query by day: idx[%d]%d-%d-%d", pquery->channel,pquery->year, pquery->month, pquery->day);
            SS_QUERY_DAY_TS_ARR_T *p_day_ts = NULL;
            OPERATE_RET ret = tuya_ipc_pb_query_by_day(pquery->channel, pquery->ipcChan, pquery->year, pquery->month, pquery->day, &p_day_ts);
            if (OPRT_OK != ret) {
                PR_ERR("pb_ts query by day: %d-%d-%d Fail", pquery->channel, pquery->year, pquery->month, pquery->day);
                break;
            }
            if (p_day_ts){
                PR_INFO("%s %d count = %d\n",__FUNCTION__,__LINE__,p_day_ts->file_count);
                UINT32_T result_sz = sizeof(PLAY_BACK_ALARM_INFO_ARR) + p_day_ts->file_count * sizeof(PLAY_BACK_ALARM_FRAGMENT);
                PLAY_BACK_ALARM_INFO_ARR * pResult = (PLAY_BACK_ALARM_INFO_ARR *)malloc(result_sz);
                if (NULL == pResult){
                    PR_ERR("%s %d malloc failed \n",__FUNCTION__,__LINE__);
                    tuya_ipc_pb_query_free_ts_arr(p_day_ts);
                    pquery->alarm_arr = NULL;
                    break;
                }
                memset(pResult, 0, result_sz);

                INT_T i;
                pResult->file_count = p_day_ts->file_count;
                for (i = 0; i < p_day_ts->file_count; i++){
                    pResult->file_arr[i].type = p_day_ts->file_arr[i].type;
                    pResult->file_arr[i].video_type = p_day_ts->file_arr[i].video_type;
                    pResult->file_arr[i].time_sect.start_timestamp = p_day_ts->file_arr[i].start_timestamp;
                    pResult->file_arr[i].time_sect.end_timestamp = p_day_ts->file_arr[i].end_timestamp;
                }
                pquery->alarm_arr = pResult;
                free(p_day_ts);

            }else{
            pquery->alarm_arr = NULL; 
            }
            break;
        }
        case MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS_WITH_ENCRYPT: {
            C2C_TRANS_QUERY_PB_DAY_WITH_ENCRYPT_RESP* pquery = (C2C_TRANS_QUERY_PB_DAY_WITH_ENCRYPT_RESP*)args;
            PR_DEBUG("pb_ts query by day: idx[%d]%d-%d-%d", pquery->channel, pquery->year, pquery->month, pquery->day);
            SS_DAY_TS_RESULT_T* p_day_ts = NULL;
            OPERATE_RET ret = tuya_ipc_pb_query_by_day_with_encrypt_info(pquery->channel, pquery->ipcChan, pquery->allow_encrypt, pquery->year, pquery->month, pquery->day, &p_day_ts);
            if (OPRT_OK != ret) {
            PR_ERR("pb_ts query by day: %d-%d-%d Fail", pquery->channel, pquery->year, pquery->month, pquery->day);
            break;
            }
            if (p_day_ts) {
            printf("%s %d count = %d\n", __FUNCTION__, __LINE__, p_day_ts->file_count);
            int len = sizeof(PLAY_BACK_ALARM_INFO_WITH_ENCRYPT_ARR) + p_day_ts->file_count * sizeof(PLAY_BACK_FILE_INFOS_WITH_ENCRYPT);
            PLAY_BACK_ALARM_INFO_WITH_ENCRYPT_ARR* pResult = (PLAY_BACK_ALARM_INFO_WITH_ENCRYPT_ARR*)malloc(len);
            if (NULL == pResult) {
                    printf("%s %d malloc failed \n", __FUNCTION__, __LINE__);
                    tuya_ipc_pb_query_free_ts_arr_with_encrypt(p_day_ts);
                    pquery->alarm_arr = NULL;
                    ret = 0;
                    return 0;
            }
            memset(pResult, 0, len);

            INT_T i;
            pResult->file_count = p_day_ts->file_count;
            for (i = 0; i < p_day_ts->file_count; i++) {
                    pResult->file_arr[i].type = p_day_ts->file_arr[i].type;
                    pResult->file_arr[i].video_type = p_day_ts->file_arr[i].video_type;
                    pResult->file_arr[i].time_sect.start_timestamp = p_day_ts->file_arr[i].start_timestamp;
                    pResult->file_arr[i].time_sect.end_timestamp = p_day_ts->file_arr[i].end_timestamp;
                    memcpy(pResult->file_arr[i].uuid, p_day_ts->file_arr[i].enrypt_info.uuid, 32);
                    pResult->file_arr[i].encrypt = p_day_ts->file_arr[i].enrypt_info.encrypt;
                    memcpy(pResult->file_arr[i].key_hash, p_day_ts->file_arr[i].enrypt_info.key_hash, 16);
            }
            pquery->alarm_arr = pResult;
            tuya_ipc_pb_query_free_ts_arr_with_encrypt(p_day_ts);

            } else {
            pquery->alarm_arr = NULL;
            }
            break;
        }
        case MEDIA_STREAM_PLAYBACK_QUERY_DAY_V3:
        {
            C2C_TRANS_QUERY_PB_DAY_V3_RESP_T* pquery = (C2C_TRANS_QUERY_PB_DAY_V3_RESP_T*)args;
            PR_DEBUG("pb_ts query by day_V3: idx[%d]%d-%d-%d", pquery->channel, pquery->year, pquery->month, pquery->day);
            SS_DAY_TS_RESULT_LIST_WITH_EVENT_TYPE_T result_list;
            OPERATE_RET ret = tuya_ipc_pb_query_by_day_with_event_type(pquery->channel, pquery->ipcChan, 
                                                                        pquery->allow_encrypt, pquery->year, 
                                                                        pquery->month, pquery->day,
                                                                        &result_list);
            if (OPRT_OK != ret) {
            PR_ERR("pb_ts query by day: %d-%d-%d Fail", pquery->channel, pquery->year, pquery->month, pquery->day);
            break;
            }
            if (result_list.list_cnt > 0) {
                printf("%s %d count = %d\n", __FUNCTION__, __LINE__, result_list.list_cnt);

                // PLAY_BACK_EVENT_INFO_V3_T 为可变长度，提前申请整个扩展数组的长度
                int arr_len = result_list.list_cnt * sizeof(PLAY_BACK_EVENT_INFO_V3_T) + result_list.event_type_total_cnt * sizeof(int);
                char* p_result_arr = (char*)malloc(arr_len);
                if (NULL == p_result_arr) {
                    printf("%s %d malloc failed \n", __FUNCTION__, __LINE__);
                    tuya_ipc_pb_query_free_ts_list_with_event_type(&result_list);
                    pquery->file_count = 0;
                    pquery->file_arr = NULL;
                    return 0;
                }
                memset(p_result_arr, 0, arr_len);

                INT_T offset = 0;
                SS_DAY_TS_ENRTY_WITH_EVENT_TYPE_T *node = NULL;
                P_LIST_HEAD pos = NULL;
                P_LIST_HEAD pos_next = NULL;

                pquery->file_count = result_list.list_cnt;
                PR_DEBUG("QUERY_PB_DAY_V3 cnt:%d,ev_tot_cnt:%d,len:%d", result_list.list_cnt, result_list.event_type_total_cnt, arr_len);

                tuya_list_for_each_safe(pos, pos_next, &(result_list.entry)) {
                    node = tuya_list_entry(pos, SS_DAY_TS_ENRTY_WITH_EVENT_TYPE_T, entry);
                    PLAY_BACK_EVENT_INFO_V3_T* out_record = (PLAY_BACK_EVENT_INFO_V3_T*)(p_result_arr + offset);

                    int type_arr_sz = node->event_cnt * sizeof(UINT_T);
                    if (type_arr_sz > 0) {
                        memcpy(out_record->event_type_arr, node->event_type_arr, type_arr_sz);
                    }
                    out_record->event_type_count = node->event_cnt;
                    out_record->video_type = node->video_type;
                    out_record->time_sect.start_timestamp = node->start_timestamp;
                    out_record->time_sect.end_timestamp = node->end_timestamp;
                    memcpy(out_record->uuid, node->enrypt_info.uuid, 32);
                    out_record->encrypt = node->enrypt_info.encrypt;
                    memcpy(out_record->key_hash, node->enrypt_info.key_hash, 16);

                    PR_DEBUG("query dayv3: offset:%d,[%u,%u],event_type_count:%u,video_type:%u", offset,
                                    out_record->time_sect.start_timestamp, out_record->time_sect.end_timestamp,
                                    out_record->event_type_count, out_record->video_type);

                    offset += (sizeof(PLAY_BACK_EVENT_INFO_V3_T) + type_arr_sz);
                }

                pquery->file_arr_sz = arr_len;
                pquery->file_arr = p_result_arr;
            } else {
                printf("%s %d count = 0\n", __FUNCTION__, __LINE__);
                pquery->file_count = 0;
            }

            tuya_ipc_pb_query_free_ts_list_with_event_type(&result_list);
            
            printf("%s %d over\n", __FUNCTION__, __LINE__);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_IMAGE_V2:
        {
            C2C_TRANS_CTRL_PB_DOWNLOAD_IMAGE_V2_T* pquery = (C2C_TRANS_CTRL_PB_DOWNLOAD_IMAGE_V2_T*)args;
            PR_DEBUG("download_image_v2:idx:%u[%u,%u],encrypt_allow:%d", pquery->channel, pquery->time_sect.start_timestamp,
                                                        pquery->time_sect.end_timestamp, pquery->encrypt_allow);

            SS_IMAGE_QUERY_INFO_T query_in;
            memset(&query_in, 0, sizeof(query_in));
            query_in.encrypt_allow = pquery->encrypt_allow;
            query_in.pb_idx = pquery->channel;
            query_in.time_sect.start_timestamp = pquery->time_sect.start_timestamp;
            query_in.time_sect.end_timestamp = pquery->time_sect.end_timestamp;

            SS_IMAGE_QUERY_OUT_T image_out;
            memset(&image_out, 0, sizeof(image_out));
            image_out.pic_name_len = sizeof(pquery->pic_name);
            image_out.pic_name_buf = pquery->pic_name;
            OPERATE_RET ret = tuya_ipc_download_image_v2(&query_in, &image_out);
            if (OPRT_OK == ret) {
                pquery->encrypt = image_out.encrypt;
                pquery->image_file_len = image_out.p_buf_len;
                pquery->p_buffer = image_out.p_buf;
                PR_DEBUG("download_image_v2 ok,pic:%s,encrypt:%d,pic_len:%d,buf:%p", pquery->pic_name,
                            pquery->encrypt, pquery->image_file_len, pquery->p_buffer);
            }
            break;
        }
        case MEDIA_STREAM_PLAYBACK_START_TS: {
            /* Client will bring the start time when playback.
            For the sake of simplicity, only log printing is done. */
            C2C_TRANS_CTRL_PB_START *pParam = (C2C_TRANS_CTRL_PB_START *)args;
            PR_DEBUG("PB StartTS idx:%d %u [%u %u]", pParam->channel, pParam->playTime,
                    pParam->time_sect.start_timestamp, pParam->time_sect.end_timestamp);

            SS_FILE_TIME_TS_T pb_file_info;
            memset(&pb_file_info, 0x00, sizeof(SS_FILE_TIME_TS_T));
            // memcpy(&pb_file_info, &pParam->time_sect, sizeof(SS_FILE_TIME_TS_T));
            pb_file_info.start_timestamp = pParam->time_sect.start_timestamp;
            pb_file_info.end_timestamp = pParam->time_sect.end_timestamp;
            int ret = tuya_ipc_ss_pb_start_with_encrypt(pParam->channel, pParam->reqId, pParam->allow_encrypt, __TUYA_APP_ss_pb_event_cb,
                __TUYA_APP_ss_pb_get_video_encrypt_cb,
                __TUYA_APP_ss_pb_get_audio_encrypt_cb);

            if (0 != ret) {
                printf("%s %d pb_start failed\n", __FUNCTION__, __LINE__);
                tuya_ipc_media_playback_send_finish(pParam->channel);
            } else {
            if (0 != tuya_ipc_ss_pb_seek_with_reqId(pParam->channel, pParam->reqId, &pb_file_info, pParam->playTime)) {
                printf("%s %d pb_seek failed\n", __FUNCTION__, __LINE__);
                tuya_ipc_media_playback_send_finish(pParam->channel);
            }
            }

            break;
        }
        case MEDIA_STREAM_PLAYBACK_PAUSE:
        {
            C2C_TRANS_CTRL_PB_PAUSE *pParam = (C2C_TRANS_CTRL_PB_PAUSE *)args;
            PR_DEBUG("PB Pause idx:%d", pParam->channel);

            tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_PAUSE);
            break;
        }
        case MEDIA_STREAM_PLAYBACK_RESUME:
        {
            C2C_TRANS_CTRL_PB_RESUME *pParam = (C2C_TRANS_CTRL_PB_RESUME *)args;
            PR_DEBUG("PB Resume idx:%d", pParam->channel);

            tuya_ipc_ss_pb_set_status_with_reqId(pParam->channel, SS_PB_RESUME, pParam->reqId);
            break;
        }
        case MEDIA_STREAM_PLAYBACK_MUTE:
        {
            C2C_TRANS_CTRL_PB_MUTE *pParam = (C2C_TRANS_CTRL_PB_MUTE *)args;
            PR_DEBUG("PB idx:%d mute", pParam->channel);

            tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_MUTE);
            break;
        }
        case MEDIA_STREAM_PLAYBACK_UNMUTE:
        {
            C2C_TRANS_CTRL_PB_UNMUTE *pParam = (C2C_TRANS_CTRL_PB_UNMUTE *)args;
            PR_DEBUG("PB idx:%d unmute", pParam->channel);

            tuya_ipc_ss_pb_set_status_with_reqId(pParam->channel, SS_PB_UN_MUTE, pParam->reqId);
            break;
        }
        case MEDIA_STREAM_PLAYBACK_SET_SPEED:
        {
            C2C_TRANS_CTRL_PB_SET_SPEED * pParam = (C2C_TRANS_CTRL_PB_SET_SPEED *)args;
            PR_DEBUG("chn[%u] video set speed[%u]\n", pParam->channel, pParam->speed);
            int ret = tuya_ipc_ss_pb_set_speed_with_reqId(pParam->channel, pParam->reqId, pParam->speed);

            if (0 != ret){
                PR_ERR("%s %d pb set speed failed\n", __FUNCTION__, __LINE__);
                tuya_ipc_p2p_playback_send_finish(pParam->channel);
            }
            break;
        }
        case MEDIA_STREAM_PLAYBACK_STOP:
        {
            C2C_TRANS_CTRL_PB_STOP *pParam = (C2C_TRANS_CTRL_PB_STOP *)args;
            PR_DEBUG("PB Stop idx:%d", pParam->channel);

            tuya_ipc_ss_pb_stop(pParam->channel);
            break;
        }
        case MEDIA_STREAM_PLAYBACK_DELETE:
        {
            C2C_TRANS_CTRL_PB_DELDATA_BYDAY_REQ *pParam = (C2C_TRANS_CTRL_PB_DELDATA_BYDAY_REQ *)args;
            if (OPRT_OK !=  tuya_ipc_ss_delete_video(pParam->channel, pParam->year,pParam->month, pParam->day))
            {
                PR_ERR("tuya_ipc_ss_delete_video err");
            }
            break;
        }
        case MEDIA_STREAM_LIVE_VIDEO_CLARITY_SET:
        {
            C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam = (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
            PR_DEBUG("set clarity:%d", pParam->clarity);
        if ((pParam->clarity == TY_VIDEO_CLARITY_STANDARD) || (pParam->clarity == TY_VIDEO_CLARITY_HIGH)) {
            PR_DEBUG("set clarity:%d OK", pParam->clarity);
            s_p2p_mgr.live_clarity = pParam->clarity;
            }
            break;
        }
        case MEDIA_STREAM_LIVE_VIDEO_CLARITY_QUERY:
        {
            C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam = (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
            pParam->clarity = s_p2p_mgr.live_clarity;
            PR_DEBUG("query larity:%d", pParam->clarity);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_START:
        {
            C2C_TRANS_CTRL_DL_START *pParam = (C2C_TRANS_CTRL_DL_START *)args;
            SS_DOWNLOAD_FILES_TS_T strParm={0};
            strParm.file_count = pParam->fileNum;
            strParm.dl_start_time = pParam->downloadStartTime;
            strParm.dl_end_time = pParam->downloadEndTime;
            strParm.file_count = pParam->fileNum;
            int len = pParam->fileNum*sizeof(SS_FILE_INFO_T);
            strParm.p_file_info_arr = (SS_FILE_INFO_T*)malloc(len);
        if (strParm.p_file_info_arr == NULL) {
            PR_DEBUG("mallocTRANS_DOWNLOAD_START failed");
            break;
        }
        if (pParam->pFileInfo) {
            memcpy(strParm.p_file_info_arr, pParam->pFileInfo, len);
        } else {
            free(strParm.p_file_info_arr);
                PR_DEBUG("TRANS_DOWNLOAD_START p_file_info_arr NULL ");
                break;
            }
            if (OPRT_OK == tuya_ipc_ss_donwload_pre(pParam->channel, &strParm)){
                tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_START);
            }
            free(strParm.p_file_info_arr);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_START_WITH_ENCRYPT: {
            C2C_TRANS_CTRL_DL_ENCRYPT_START* pParam = (C2C_TRANS_CTRL_DL_ENCRYPT_START*)args;
            SS_DOWNLOAD_FILES_TS_T strParm = { 0 };
            strParm.dl_start_time = pParam->downloadStartTime;
            strParm.dl_end_time = pParam->downloadEndTime;
            strParm.file_count = pParam->fileNum;
            int arr_size = pParam->fileNum * sizeof(SS_FILE_INFO_T);
            strParm.p_file_info_arr = (SS_FILE_INFO_T*)malloc(arr_size);
            if (strParm.p_file_info_arr == NULL) {
                PR_DEBUG("mallocTRANS_DOWNLOAD_START failed");
                break;
            }
            if (pParam->pFileInfo) {
                memcpy(strParm.p_file_info_arr, pParam->pFileInfo, arr_size);
            } else {
                free(strParm.p_file_info_arr);
                PR_DEBUG("TRANS_DOWNLOAD_START p_file_info_arr NULL ");
                break;
            }
            if (OPRT_OK == tuya_ipc_ss_donwload_pre_support_encrypt(pParam->channel, pParam->allow_encrypt, &strParm)) {
                tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_START);
            }
            free(strParm.p_file_info_arr);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_STOP: {
            C2C_TRANS_CTRL_DL_STOP *pParam = (C2C_TRANS_CTRL_DL_STOP *)args;
            tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_STOP);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_PAUSE:
        {
            C2C_TRANS_CTRL_DL_PAUSE *pParam = (C2C_TRANS_CTRL_DL_PAUSE *)args;
            tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_PAUSE);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_RESUME:
        {
            C2C_TRANS_CTRL_DL_RESUME *pParam = (C2C_TRANS_CTRL_DL_RESUME *)args;
            tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_RESUME);
            break;
        }
        case MEDIA_STREAM_DOWNLOAD_CANCLE:
        {
            C2C_TRANS_CTRL_DL_CANCLE *pParam = (C2C_TRANS_CTRL_DL_CANCLE *)args;
            tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_CANCLE);
            break;
        }
        case MEDIA_STREAM_ALBUM_QUERY:
        case MEDIA_STREAM_ALBUM_DOWNLOAD_START:
        case MEDIA_STREAM_ALBUM_DOWNLOAD_CANCEL:
        case MEDIA_STREAM_ALBUM_DELETE:
        case MEDIA_STREAM_ALBUM_PLAY_CTRL:
        {
            ret = tuya_ipc_stor_album_cb(event, args);
            break;
        }
        default:
            PR_DEBUG("not supprot cmd:%d", event);
            break;
    }
    return ret;
}

#define AUDIO_DECODE_SIZE 700
unsigned char audio_decode_drc[AUDIO_DECODE_SIZE];

VOID TUYA_IPC_APP_rev_audio_cb(IN INT_T device, IN INT_T channel, IN CONST MEDIA_AUDIO_FRAME_T *p_audio_frame)
{
    // MEDIA_FRAME_T audio_frame = {0};
    // audio_frame.p_buf = p_audio_frame->p_audio_buf;
    // audio_frame.size = p_audio_frame->buf_len;

    // PR_INFO("Rev Audio. size:[%u] audio_codec:[%d] audio_sample:[%d] audio_databits:[%d] audio_channel:[%d]\n",p_audio_frame->buf_len,
    //          p_audio_frame->audio_codec, p_audio_frame->audio_sample, p_audio_frame->audio_databits, p_audio_frame->audio_channel);

//    TUYA_APP_Rev_Audio_CB( &audio_frame, TUYA_AUDIO_SAMPLE_8K, TUYA_AUDIO_DATABITS_16, TUYA_AUDIO_CHANNEL_MONO);

    unsigned int out_len = 0;
    unsigned char type = TUYA_G711_MU_LAW;

    if (p_audio_frame->audio_codec != TUYA_CODEC_AUDIO_G711U) {
        PR_INFO("audio_codec:%d, not support");
        return;
    }

    memset(audio_decode_drc, 0, sizeof(audio_decode_drc));

    if (p_audio_frame->buf_len <= 0 || (p_audio_frame->buf_len * 2) >= AUDIO_DECODE_SIZE) {
        PR_INFO("AUDIO_DECODE_SIZE not enough! %d", p_audio_frame->buf_len);
        return;
    }
    tuya_g711_decode(type, (unsigned short *)p_audio_frame->p_audio_buf, p_audio_frame->buf_len, audio_decode_drc, &out_len);


    TAL_AUDIO_FRAME_INFO_T pframe = {0};
    pframe.sample = p_audio_frame->audio_sample;
    pframe.datebits = p_audio_frame->audio_databits;
    pframe.channel = (p_audio_frame->audio_channel == TUYA_AUDIO_CHANNEL_MONO) ? TKL_AUDIO_CHANNEL_MONO : TKL_AUDIO_CHANNEL_STEREO;
    pframe.used_size = out_len;
    pframe.pbuf = audio_decode_drc;

    tal_ao_put_frame(0, 0, NULL, &pframe);

    return;
}

VOID TUYA_IPC_APP_rev_video_cb(IN INT_T device, IN INT_T channel, IN CONST MEDIA_VIDEO_FRAME_T *p_video_frame)
{
    PR_INFO("Rev video. size:[%u] video_codec:[%d] video_frame_type:[%d]\n", p_video_frame->buf_len, p_video_frame->video_codec, p_video_frame->video_frame_type);
    return;
}

VOID TUYA_IPC_APP_rev_file_cb(IN INT_T device, IN INT_T channel, IN CONST MEDIA_FILE_DATA_T *p_file_data)
{
    return;
}

VOID TUYA_APP_get_snapshot_cb(IN INT_T device, IN INT_T channel, OUT CHAR_T *snap_addr, OUT INT_T *snap_size)
{
    // in this demo ignore device and channel parameter
    IPC_APP_get_snapshot(snap_addr, snap_size);
}

VOID TUYA_IPC_Media_Adapter_Init(TUYA_IPC_SDK_MEDIA_ADAPTER_S *p_media_adatper_info, TUYA_IPC_SDK_MEDIA_STREAM_S *p_media_infos)
{
    TUYA_IPC_MEDIA_ADAPTER_VAR_T media_var;
    media_var.get_snapshot_cb = p_media_adatper_info->get_snapshot_cb;
    media_var.on_recv_audio_cb = p_media_adatper_info->rev_audio_cb;
    media_var.on_recv_video_cb = p_media_adatper_info->rev_video_cb;
    media_var.on_recv_file_cb = p_media_adatper_info->rev_file_cb;
    media_var.available_media_memory = 0;
    
    tuya_ipc_media_adapter_init(&media_var);
    
    DEVICE_MEDIA_INFO_T device_media_info = { 0 };
    memcpy(&device_media_info.av_encode_info, &p_media_infos->media_info,sizeof(IPC_MEDIA_INFO_T));
    
    device_media_info.audio_decode_info.enable = 1;
    device_media_info.audio_decode_info.audio_codec = TUYA_CODEC_AUDIO_G711U;
    device_media_info.audio_decode_info.audio_sample = TUYA_AUDIO_SAMPLE_8K;
    device_media_info.audio_decode_info.audio_databits = TUYA_AUDIO_DATABITS_16;
    device_media_info.audio_decode_info.audio_channel = TUYA_AUDIO_CHANNEL_MONO;
    
    device_media_info.max_pic_len = MAX_SNAPSHOT_BUFFER_SIZE_KB;

#ifdef VIDEO_DECODE //上报视频解码能力，用于双向视频功能
    TUYA_DECODER_T video_decoder = {0};
    video_decoder.codec_id = TUYA_CODEC_VIDEO_H264;
    video_decoder.decoder_desc.v_decoder.height = 320;
    video_decoder.decoder_desc.v_decoder.width = 240;
    video_decoder.decoder_desc.v_decoder.profile = VIDEO_AVC_PROFILE_BASE_LINE;
    device_media_info.decoder_cnt = 1;
    device_media_info.decoders = &video_decoder;
#endif    
    tuya_ipc_media_adapter_set_media_info(0, 0, device_media_info);    

    return;
}

VOID TUYA_IPC_Media_Stream_Init(TUYA_IPC_SDK_MEDIA_ADAPTER_S *p_media_adatper_info)
{
    MEDIA_STREAM_VAR_T  media_stream_var = {0};
    media_stream_var.on_event_cb = p_media_adatper_info->media_event_cb;
    media_stream_var.max_client_num = p_media_adatper_info->max_stream_client;
    media_stream_var.def_live_mode = p_media_adatper_info->live_mode;
#ifdef VIDEO_DECODE //增加接收缓存区大小，用于双向视频功能    
    media_stream_var.recv_buffer_size = 300*1024;
#else 
    media_stream_var.recv_buffer_size = 16*1024;
#endif

#ifdef LOW_BAT_IPC
    media_stream_var.low_power = 1;
#endif
    media_stream_var.lan_only_mode = tuya_get_lan_mode();
    int ret = tuya_ipc_media_stream_init(&media_stream_var);
    PR_DEBUG("media stream init result is %d\n",ret);

    return;
}

/*
---------------------------------------------------------------------------------
code related RingBuffer
---------------------------------------------------------------------------------
*/
IPC_MEDIA_INFO_T old_mediaInfo = { 0 } ;
RING_BUFFER_USER_HANDLE_T g_a_handle[MAX_IPC_CHANNEL_NUM] = {NULL};
RING_BUFFER_USER_HANDLE_T g_v_handle[MAX_IPC_CHANNEL_NUM] = {NULL};
RING_BUFFER_USER_HANDLE_T g_v_handle_sub[MAX_IPC_CHANNEL_NUM] = {NULL};
STATIC BOOL_T s_ring_buffer_inited[MAX_IPC_CHANNEL_NUM] = {FALSE};

OPERATE_RET TUYA_APP_Init_Ring_Buffer(CONST IPC_MEDIA_INFO_T *pMediaInfo, INT_T channel)
{
    if (NULL == pMediaInfo) {
        PR_ERR("create ring buffer para is NULL\n");
        return -1;
    }
    OPERATE_RET ret = OPRT_OK;

    if (s_ring_buffer_inited[channel] == TRUE) {
        PR_DEBUG("The Ring Buffer Is Already Inited");
        return OPRT_OK;
    }

    IPC_STREAM_E ringbuffer_stream_type;
    // CHANNEL_E channel;
    RING_BUFFER_INIT_PARAM_T param = { 0 };
    for (ringbuffer_stream_type = E_IPC_STREAM_VIDEO_MAIN; ringbuffer_stream_type < E_IPC_STREAM_MAX; ringbuffer_stream_type++) {
        PR_DEBUG("init ring buffer Channel:%d Stream:%d Enable:%d", channel, ringbuffer_stream_type, pMediaInfo->stream_enable[ringbuffer_stream_type]);
        if (pMediaInfo->stream_enable[ringbuffer_stream_type] == TRUE) {
            if (ringbuffer_stream_type == E_IPC_STREAM_AUDIO_MAIN) {
                param.bitrate = pMediaInfo->audio_sample[E_IPC_STREAM_AUDIO_MAIN] * pMediaInfo->audio_databits[E_IPC_STREAM_AUDIO_MAIN] / 1024;
                param.fps = pMediaInfo->audio_fps[E_IPC_STREAM_AUDIO_MAIN];
                param.max_buffer_seconds = 5;
                param.request_key_frame_cb = NULL;
                PR_DEBUG("audio_sample %d, audio_databits %d, audio_fps %d", pMediaInfo->audio_sample[E_IPC_STREAM_AUDIO_MAIN], pMediaInfo->audio_databits[E_IPC_STREAM_AUDIO_MAIN], pMediaInfo->audio_fps[E_IPC_STREAM_AUDIO_MAIN]);
                ret = tuya_ipc_ring_buffer_init(0, channel, ringbuffer_stream_type, &param);
            } else {
                param.bitrate = pMediaInfo->video_bitrate[ringbuffer_stream_type];
                param.fps = pMediaInfo->video_fps[ringbuffer_stream_type];
                param.max_buffer_seconds = 5;
                param.request_key_frame_cb = NULL;
                PR_DEBUG("video_bitrate %d, video_fps %d", pMediaInfo->video_bitrate[ringbuffer_stream_type], pMediaInfo->video_fps[ringbuffer_stream_type]);
                ret = tuya_ipc_ring_buffer_init(0, channel, ringbuffer_stream_type, &param);
            }
            if (ret != 0) {
                PR_ERR("init ring buffer fails. %d %d", ringbuffer_stream_type, ret);
                return OPRT_MALLOC_FAILED;
            }
            PR_DEBUG("init ring buffer success. channel:%d", ringbuffer_stream_type);
        }
    }

    memcpy(&old_mediaInfo,pMediaInfo,sizeof(IPC_MEDIA_INFO_T)) ;
    s_ring_buffer_inited[channel] = TRUE;

    return OPRT_OK;
}


OPERATE_RET TUYA_APP_Put_Frame(RING_BUFFER_USER_HANDLE_T handle, IN CONST MEDIA_FRAME_T *p_frame)
{
    PR_TRACE("Put Frame. type:%d size:%u pts:%llu ts:%llu",
            p_frame->type, p_frame->size, p_frame->pts, p_frame->timestamp);

    OPERATE_RET ret = tuya_ipc_ring_buffer_append_data(handle,p_frame->p_buf, p_frame->size,p_frame->type,p_frame->pts);

    if (ret != OPRT_OK) {
        PR_ERR("Put Frame Fail.%d  type:%d size:%u pts:%llu ts:%llu", ret,
            p_frame->type, p_frame->size, p_frame->pts, p_frame->timestamp);
    }
    return ret;
}

int IPC_APP_get_snapshot(char *snap_addr, int *snap_size)
{
    TAL_VENC_FRAME_T frame = {0};
    if (snap_addr == NULL || snap_size == NULL || *snap_size <= 0) {
        PR_ERR("parm is wrong\n");
        return OPRT_COM_ERROR;
    }

    frame.pbuf = snap_addr;
    frame.buf_size = *snap_size;
    OPERATE_RET ret = tal_venc_get_frame(0, 8, &frame);
    if (ret != OPRT_OK){
        PR_ERR("get pic failed\n", ret);
        *snap_size = 0;
        return OPRT_COM_ERROR;
    }
    *snap_size = frame.used_size;
    PR_INFO("get pic suc, size:%d", frame.used_size);

    return 0;
}

#ifdef IPC_DUAL
int IPC_APP_get_snapshot_dual_gun(char *snap_addr, int *snap_size)
{
    TAL_VENC_FRAME_T frame = {0};
    if (snap_addr == NULL || snap_size == NULL || *snap_size <= 0) {
        PR_ERR("parm is wrong\n");
        return OPRT_COM_ERROR;
    }

    frame.pbuf = snap_addr;
    frame.buf_size = *snap_size;
    OPERATE_RET ret = tal_venc_get_frame(0, 9, &frame);
    if (ret != OPRT_OK){
        PR_ERR("get dual_gun pic failed\n", ret);
        *snap_size = 0;
        return OPRT_COM_ERROR;
    }
    *snap_size = frame.used_size;
    PR_INFO("get dual_gun pic suc, size:%d", frame.used_size);

    return 0;
}
#endif
