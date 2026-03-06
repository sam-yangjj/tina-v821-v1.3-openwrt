#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#include "tal_system.h"
#include "uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_media_stream_event.h"
#include "ty_sdk_common.h"
#include "tuya_ipc_album.h"
#include "tuya_p2p_api.h"

/* album info */
extern CHAR_T s_raw_path[128];
#define ABS_PATH_LEN 128
#define CMD_LEN 1024
static char s_test_pic_thumbNail[ABS_PATH_LEN] = "thumbnail/pic_thumbnail.jpg";
static char s_test_mp4_thumbNail[ABS_PATH_LEN] = "thumbnail/mp4_thumbnail.jpg";
static char s_raw_emergency_path[ABS_PATH_LEN] = { 0 };
static char s_raw_emergency_thumbnail[ABS_PATH_LEN] = { 0 };
static char s_emergency_path[ABS_PATH_LEN] = { 0 };
static char s_emergency_thunbnail[ABS_PATH_LEN] = { 0 };
static int emerge_album_path_inited = 0;

STATIC VOID __TUYA_APP_ss_album_event_cb(IN UINT_T pb_idx, IN SS_PB_EVENT_E pb_event, IN PVOID_T args)
{
    PR_DEBUG("album play send event: %u,%d", pb_idx, pb_event);
    if(pb_event == SS_PB_FINISH) {
        tuya_imm_p2p_album_play_send_finish(NULL, pb_idx);
    }

    return;
}

STATIC VOID __TUYA_APP_ss_album_get_video_cb(IN UINT_T pb_idx, UINT_T reqId, IN CONST SS_MEDIA_FRAME_WITH_ENCRYPT_T *p_frame)
{
    INT_T ret = 0;
    TUYA_ALBUM_PLAY_FRAME_T encrypt_head;
    memset(&encrypt_head, 0, sizeof(encrypt_head));

    encrypt_head.type = p_frame->frame_type;
    encrypt_head.size = p_frame->size;
    encrypt_head.timestamp = p_frame->timestamp;
    encrypt_head.pts = p_frame->pts;

    // 根据实际参数填写
    encrypt_head.media.video.video_codec = TUYA_CODEC_VIDEO_H265;
    encrypt_head.media.video.frame_rate = 20;
    encrypt_head.media.video.video_width = 2560;
    encrypt_head.media.video.video_height = 1440;
    
    encrypt_head.encrypt_info.encrypt = 0;
    encrypt_head.p_buf = p_frame->p_buf;

    // 发送帧
    ret = tuya_imm_p2p_app_album_play_send_data(NULL, pb_idx, &encrypt_head);

    PR_DEBUG("==>send ALBUM video typ:%d,tm:%llu,sz:%u,ret:%d", encrypt_head.type, encrypt_head.timestamp, encrypt_head.size, ret);

    return;
}

STATIC VOID __TUYA_APP_ss_album_get_audio_cb(IN UINT_T pb_idx, int reqId, IN CONST SS_MEDIA_FRAME_WITH_ENCRYPT_T *p_frame)
{
    INT_T ret = 0;
    TUYA_ALBUM_PLAY_FRAME_T encrypt_head;
    memset(&encrypt_head, 0, sizeof(encrypt_head));

    encrypt_head.type = p_frame->frame_type;
    encrypt_head.size = p_frame->size;
    encrypt_head.timestamp = p_frame->timestamp;
    encrypt_head.pts = p_frame->pts;

    // 根据实际参数填写
    encrypt_head.media.audio.audio_codec = TUYA_CODEC_AUDIO_G711U;
    encrypt_head.media.audio.audio_sample = 8000;
    encrypt_head.media.audio.audio_databits = TUYA_AUDIO_DATABITS_16;
    encrypt_head.media.audio.audio_channel = TUYA_AUDIO_CHANNEL_MONO;

    encrypt_head.encrypt_info.encrypt = 0;
    encrypt_head.p_buf = p_frame->p_buf;

    // 发送帧
    ret = tuya_imm_p2p_app_album_play_send_data(NULL, pb_idx, &encrypt_head);

    return;
}

VOID __process_album_play(C2C_CMD_IO_CTRL_ALBUM_PLAY_CTRL_T* app_req)
{
    int ret = 0;

    SS_ALBUM_ALBUM_PLAY_CTRL_T play_ctrl;
    memset(&play_ctrl, 0, sizeof(play_ctrl));
    play_ctrl.channel = app_req->channel;
    play_ctrl.req_id = app_req->req_id;
    play_ctrl.user_idx = app_req->user_idx;
    play_ctrl.operation = app_req->operation;
    play_ctrl.thumbnail = app_req->thumbnail;
    play_ctrl.start_time = app_req->start_time;
    memcpy(play_ctrl.album_name, app_req->album_name, SS_ALBUM_MAX_FILE_NAME_LEN);
    memcpy(play_ctrl.file_name, app_req->file_name, SS_ALBUM_MAX_FILE_NAME_LEN);

    if (TY_CMD_IO_CTRL_ALBUM_PLAY_START == app_req->operation) {
        PR_DEBUG("ALBUM_PLAY_START:%d,%s,%s", play_ctrl.user_idx, 
                                             play_ctrl.album_name, play_ctrl.file_name);

        ret = tuya_ipc_album_play_start(&play_ctrl, __TUYA_APP_ss_album_event_cb,
                                             __TUYA_APP_ss_album_get_video_cb, 
                                             __TUYA_APP_ss_album_get_audio_cb);
        if (0 != ret) {
            PR_DEBUG("ALBUM_PLAY_START failed");
            tuya_ipc_media_playback_send_finish(play_ctrl.user_idx);
        } else {
            if (0 != tuya_ipc_album_play_seek_with_reqId(play_ctrl.user_idx, play_ctrl.req_id, &play_ctrl)) {
                PR_DEBUG("ALBUM_PLAY_START seek failed");
                tuya_ipc_media_playback_send_finish(play_ctrl.user_idx);
            }
        }
    } else if (TY_CMD_IO_CTRL_ALBUM_PLAY_STOP == app_req->operation) { 
        PR_DEBUG("ALBUM_PLAY_STOP:%d", play_ctrl.user_idx);
        tuya_ipc_album_play_stop(play_ctrl.user_idx);
    } else if (TY_CMD_IO_CTRL_ALBUM_PLAY_PAUSE == app_req->operation) {
        PR_DEBUG("ALBUM_PLAY_PAUSE:%d", play_ctrl.user_idx);
        tuya_ipc_album_play_set_status_with_reqId(play_ctrl.user_idx, SS_ALBUM_PLAY_PAUSE, play_ctrl.req_id);
    } else if (TY_CMD_IO_CTRL_ALBUM_PLAY_RESUME == app_req->operation) {
        PR_DEBUG("ALBUM_PLAY_RESUME:%d", play_ctrl.user_idx);
        tuya_ipc_album_play_set_status_with_reqId(play_ctrl.user_idx, SS_ALBUM_PLAY_RESUME, play_ctrl.req_id);
    }
}

/* Callback functions for transporting events */
int tuya_ipc_stor_album_cb(int event, void* args)
{
    PR_DEBUG("p2p rev event cb=[%d] ", event);
    switch (event)
    {
        /* 内容结构：
        C2C_ALBUM_INDEX_HEAD
        C2C_ALBUM_INDEX_ITEM
        C2C_ALBUM_INDEX_ITEM
        C2C_ALBUM_INDEX_ITEM
        */
        case MEDIA_STREAM_ALBUM_QUERY: /* query album */
        {
            C2C_QUERY_ALBUM_REQ* pSrcType = (C2C_QUERY_ALBUM_REQ*)args;

            // NOTICE: pIndexFile malloc/free in SDK
            int ret = tuya_ipc_album_query_by_name(pSrcType->albumName, pSrcType->channel, &pSrcType->fileLen, (C2C_ALBUM_INDEX_HEAD**)&(pSrcType->pIndexFile));
            if (0 != ret) {
                PR_ERR("err %d", ret); 
            }

            if (pSrcType->pIndexFile) {
                C2C_ALBUM_INDEX_HEAD* ptmp = (C2C_ALBUM_INDEX_HEAD*)pSrcType->pIndexFile;
                PR_DEBUG("get album items %d", ptmp->itemCount);
            }
            break;
        }
        case MEDIA_STREAM_ALBUM_DOWNLOAD_START: /* start download album */
        {
            C2C_CMD_IO_CTRL_ALBUM_DOWNLOAD_START* pSrcType = (C2C_CMD_IO_CTRL_ALBUM_DOWNLOAD_START*)args;
            PR_DEBUG("start download\n");
            SS_DOWNLOAD_STATUS_E status = 0;
            SS_ALBUM_DOWNLOAD_START_INFO_T strStarInfo = { 0 };
            strStarInfo.session_id = pSrcType->channel;
            memcpy(strStarInfo.album_name, pSrcType->albumName, strlen(pSrcType->albumName));
            strStarInfo.file_count = pSrcType->fileTotalCnt;
            strStarInfo.thumbnail = pSrcType->thumbnail;
            strStarInfo.p_file_info_arr = (SS_FILE_PATH_T*)pSrcType->pFileInfoArr;
            int ret = tuya_ipc_album_set_download_status(SS_DL_START, &strStarInfo);
            if (0 != ret) {
                PR_ERR("err %d", ret);
            }
            break;
        }
        case MEDIA_STREAM_ALBUM_DOWNLOAD_CANCEL: // cancel album
        {
            // thread do nothing...
            C2C_ALBUM_DOWNLOAD_CANCEL* pSrcType = (C2C_ALBUM_DOWNLOAD_CANCEL*)args;
            SS_ALBUM_DOWNLOAD_START_INFO_T strStarInfo = { 0 };
            strStarInfo.session_id = pSrcType->channel;
            memcpy(strStarInfo.album_name, pSrcType->albumName, strlen(pSrcType->albumName));
            int ret = tuya_ipc_album_set_download_status(SS_DL_CANCLE, &strStarInfo);
            if (0 != ret) {
                PR_ERR("err %d", ret);
            }
            PR_ERR("ok %d", ret);

            break;
        }
        case MEDIA_STREAM_ALBUM_DELETE: //delete
        {
            C2C_CMD_IO_CTRL_ALBUM_DELETE* pSrcType = (C2C_CMD_IO_CTRL_ALBUM_DELETE*)args;
            int ret = tuya_ipc_album_delete_by_file_info(pSrcType->channel, pSrcType->albumName, pSrcType->fileNum, (SS_FILE_PATH_T*)pSrcType->pFileInfoArr);
            if (0 != ret) {
                PR_ERR("err %d", ret);
                tuya_ipc_delete_video_finish_v2(pSrcType->channel, TUYA_DOWNLOAD_ALBUM, 0);
            }
            break;
        }
        case MEDIA_STREAM_ALBUM_PLAY_CTRL: {
            C2C_CMD_IO_CTRL_ALBUM_PLAY_CTRL_T* app_req = (C2C_CMD_IO_CTRL_ALBUM_PLAY_CTRL_T*)args;
            __process_album_play(app_req);
        }
            break;
        default:
            return -1;
            break;
    }
    return 0;
}


/*
  本地相册文件名要求；
  ipc_emergency_record/
        A.mp4
        B.jpg
        thumbnail/
              A.jpg
              B.jpg
  缩略图文件名和源文件文件名必须相同(除后缀外);
  源文件的文件名不可重名;

  本地相册列表交互ver1:
  1.user获取某相册路径、某相册缩略图路径，
  2.user在相册路径写入源数据（mp4/图片）
  3.user在缩略图路径写入对应缩略图
  4.user通过tuya_ipc_album_write_file_info，更新文件信息
*/

void album_file_init()
{
    if (0 == emerge_album_path_inited) {
        emerge_album_path_inited = 1;
        tuya_ipc_album_get_path(TUYA_IPC_ALBUM_EMERAGE_FILE, s_emergency_path, s_emergency_thunbnail);
        PR_DEBUG("get album file path %s \nthumbNail %s\n", s_emergency_path, s_emergency_thunbnail);

        snprintf(s_raw_emergency_path, ABS_PATH_LEN, "%s/resource/ipc_emergency_record", s_raw_path);
        snprintf(s_raw_emergency_thumbnail, ABS_PATH_LEN, "%s/resource/ipc_emergency_record/thumbnail", s_raw_path);
    }

}

void album_file_put()
{
    album_file_init();

    CHAR_T cmd[CMD_LEN];
    CONST CHAR_T* video_name = "1601278626_58_ch2.mp4";
    memset(cmd, 0, sizeof(cmd));

    // Emergency video generation, this step needs to be designed by yourself
    snprintf(cmd, CMD_LEN, "cp -rf %s/* %s/", s_raw_emergency_path, s_emergency_path);
    PR_DEBUG("cmd:%s\n",cmd);
    system(cmd);

    // write file info
    ALBUM_FILE_INFO_T strInfo;
    memset(&strInfo, 0, sizeof(ALBUM_FILE_INFO_T));
    strInfo.channel = 2;
    strInfo.type = SS_DATA_TYPE_VIDOE;
    memcpy(&strInfo.file_name, video_name, strlen(video_name));
    strInfo.create_time = 1601278626;
    strInfo.duration = 58;
    tuya_ipc_album_write_file_info(TUYA_IPC_ALBUM_EMERAGE_FILE, &strInfo);

    return;
}

