#include <stdio.h>
#include <unistd.h>

#include "tuya_iot_config.h" 
#include  "ty_sdk_call.h"

#if defined(ENABLE_TUYA_CALL)&&(ENABLE_TUYA_CALL==1)
STATIC void __call_handler(void * data)
{
    /**
     * 收到的呼叫请求或者回复处理。
     * 本demo仅演示收到呼叫请求时的操作
     * -若外设处于空闲状态，可以建立起P2P会话，则回复answer
     * -否则，回复busy状态。
     * 
     * 此回调建议处理所有的事件类型（参考枚举TUYA_TMM_CONTROL_EVT_E）
    */
    if((int)data == TUYA_TMM_CONTROL_EVT_INCOMING) {
        if(tuya_ipc_media_adapter_output_device_is_busy(0,0) == FALSE) {
            tuya_tmm_control_answer();
        } else {
            tuya_tmm_control_busy();
        }
    }
    
    return;
}

OPERATE_RET tmm_control_evt_cb(TUYA_TMM_CONTROL_INFO_S* pinfo, VOID* priv_data)
{
    /**
     * 收到呼叫请求或者回复的处理函数
     * 此处需要使用异步处理，否则会阻塞SDK内部线程的正常运转。
     * 客户可以自己创建线程或者工作队列来处理此异步请求，也可以使用涂鸦SDK内部的workq处理
     * 若使用涂鸦内部workq处理，此部分代码无需修改
    */
    printf("receive evt %d\n", pinfo->event);
    WORKQUEUE_HANDLE ty_wq_hand = tal_workq_get_handle(WORKQ_SYSTEM);
    tal_workqueue_schedule(ty_wq_hand, __call_handler, (VOID *)(pinfo->event)); 
    return OPRT_OK;
}

OPERATE_RET TUYA_IPC_call_init()
{
    tuya_tmm_control_init(tmm_control_evt_cb, NULL, 30);
}

OPERATE_RET TUYA_IPC_call_app()
{ 
    /**
     * key_1对应于app呼叫页面上配置的key_1 or key_2
     * sp_dpsxj为固定值，为双向视频呼叫ipc特有参数，请勿修改。
     * 
    */
    return tuya_tmm_control_call("key_1","sp_dpsxj", TUYA_TMM_CONTROL_STREAM_TYPE_AV);
}

OPERATE_RET TUYA_IPC_hangup()
{
    return tuya_tmm_control_hangup();
}


OPERATE_RET TUYA_IPC_download_background_demo()
{
    /**本示例以默认图片为例
     * 
     * 实际开发过程中，需要在云端配置背景图片的信息（找PM或者产品指导）
     * 配置app和设备间通信的通路，比如dp点。用于告诉设备，下载的图片信息（对应于本demo中的“downname”）
     * 下载后可以在本地缓存
     * */    
    BACKGROUND_PIC_RET_T pic_ret;
    CHAR_T *filename = "0.jpg";
    CHAR_T *downname = "/device/screen_ipc/default";

    memset(&pic_ret, 0, SIZEOF(BACKGROUND_PIC_RET_T));

    tuya_ipc_download_background_pic(downname, &pic_ret);

    printf("pic_len:%d\n",pic_ret.pic_len);

    FILE *fp = fopen(filename,"a+");
    if(fp) {
        fwrite(pic_ret.pic_buf, pic_ret.pic_len, 1, fp);
        fclose(fp);
    }
    if(pic_ret.free_cb){
        printf("free...\n");
        pic_ret.free_cb(pic_ret.pic_buf);
    }

    return OPRT_OK;
}

#endif