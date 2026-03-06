#include <stdio.h>
#include <time.h>
#include "tuya_ipc_ai_station.h"
#include "utilities/uni_log.h"

extern CHAR_T s_ipc_storage[64];

VOID TUYA_IPC_ai_cmd_cb(TUYA_AI_CMD_TYPE_E cmd, CONST PVOID_T args)
{
    switch (cmd) 
    {
        case E_AI_CMD_SERVER_VAD: 
            PR_DEBUG("trigger cloud VAD, audio and video collection ends");
            tuya_ipc_ai_station_stop_act();
        break;
        case E_AI_CMD_CHAT_START: {
            TUYA_AI_AUDIO_ATTR_T *attr = (TUYA_AI_AUDIO_ATTR_T *)args;
            PR_DEBUG("ai chat start, codec type: %d, sample_rate:%d, channels:%d, bit_depth:%d",
                attr->codec_type, attr->sample_rate, attr->channels, attr->bit_depth);
        }
        break;
        case E_AI_CMD_CHAT_FIN:
            PR_DEBUG("ai chat finish");
            break;
        case E_AI_CMD_MOVE_LEFT:
            PR_DEBUG("please turn left");
            break;
        case E_AI_CMD_MOVE_RIGHT:
            PR_DEBUG("please turn right");
            break;
        case E_AI_CMD_MOVE_UP:
            PR_DEBUG("please turn upward");
            break;
        case E_AI_CMD_MOVE_DOWN:
            PR_DEBUG("please turn downward");
            break;
        case E_AI_CMD_MOVE_AROUND:
            PR_DEBUG("please turn around");
            break;
        case E_AI_CMD_ASR_TEXT: {
            CHAR_T * asr = (CHAR_T *)args;
            PR_DEBUG("ASR TEXT: %s", asr);
        }
        break;
        case E_AI_CMD_NLG_TEXT: {
            CHAR_T * nlg = (CHAR_T *)args;
            PR_DEBUG("NLG TEXT: %s", nlg);
        }
        break;
        case E_AI_CMD_EMOTION: {
            CHAR_T *emo = (CHAR_T *)args;
            PR_DEBUG("emotion: %s", emo);
        }
        break;
        default: 
            break; 
    }
    
    return ;
}

VOID TUYA_IPC_audio_play_cb(CONST CHAR_T *buf, UINT_T len)
{
    STATIC FILE *s_audio_file = NULL;
    if (buf == NULL) {
        // 如果 buff 为 NULL，表示数据输入结束，关闭文件
        if (s_audio_file) {
            fclose(s_audio_file);
            PR_DEBUG("File closed and saved");
            s_audio_file = NULL;
        }
        return;
    }

    // 如果文件未打开，先创建文件
    if (!s_audio_file) {
        // 获取当前时间，用于生成文件名
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);

        char filename[128];
        snprintf(filename, sizeof(filename), "%s/%04d%02d%02d_%02d%02d%02d.pcm",
                s_ipc_storage,
                tm_info->tm_year + 1900, // 年份需要加 1900
                tm_info->tm_mon + 1,     // 月份需要加 1
                tm_info->tm_mday,       // 日
                tm_info->tm_hour,       // 小时
                tm_info->tm_min,        // 分钟
                tm_info->tm_sec);       // 秒

        // 打开文件
        s_audio_file = fopen(filename, "wb");
        if (!s_audio_file) {
            PR_ERR("failed to open file");
            return;
        }
        PR_DEBUG("file opened: %s\n", filename);
    }

    // 将数据写入文件
    fwrite(buf, 1, len, s_audio_file);
    PR_DEBUG("write %zu bytes to file.\n", len);
    return;
}
