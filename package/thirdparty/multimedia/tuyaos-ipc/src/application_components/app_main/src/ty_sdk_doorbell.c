/*********************************************************************************
  *Copyright(C),2015-2020, TUYA www.tuya.comm
  *FileName:    tuya_ipc_doorbell_demo
**********************************************************************************/
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#include "utilities/uni_log.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_video_msg.h"
#include "tuya_ipc_event.h"
#include "ty_sdk_common.h"
#include "tal_sw_timer.h"

#define DOORBELL_ALARM_DURATION         25*1000 //arlarm time
#define DOORBELL_NOTIFY_MSG_DURATION    6*1000  //video msg trigger timeout
#define DOORBELL_MESSAGE_DURATION       6*1000  //video msg duration
#define DOORBELL_HEARTBEAT_DURATION     30*1000 //heart beat timeout
#define PIC_MAX_SIZE 100 * 1024

typedef void *(*TIMER_CB)(void *arg);

typedef enum
{
    DOORBELL_LISTEN = 0,
    DOORBELL_RECORD,
    DOORBELL_RECORDING,
    DOORBELL_TALKING,
    DOORBELL_MAX
}DOORBELL_STATUS_E;

typedef struct
{
    DOORBELL_STATUS_E status;
    INT_T first_press; //15s timer only set once
    time_t press_time;
    CHAR_T pic_buffer[PIC_MAX_SIZE];
    INT_T pic_size;
    TIMER_ID timer;
}DOOR_BELL_MANAGER;


/*****************util API*************************/




DOOR_BELL_MANAGER * doorbell_get_handler()
{
    STATIC DOOR_BELL_MANAGER doorbell_manager;

    return &doorbell_manager;
}

//IMPORTANT!!!!!! if needed , use mutex to protect!!!!!!
VOID* timer_proc(UINT_T timerID, PVOID_T pTimerArg)
{
    DOOR_BELL_MANAGER * phdl = doorbell_get_handler();

    if(phdl->status == DOORBELL_LISTEN)
    {
        phdl->status = DOORBELL_RECORD;
        phdl->first_press = 0;
        PR_DEBUG("##############LEAVE msg?\n");
        tal_sw_timer_start(phdl->timer, DOORBELL_NOTIFY_MSG_DURATION, TAL_TIMER_ONCE);

		TUYA_ALARM_T alarm = {0};
	    alarm.type = E_ALARM_UNCONNECTED;
	    alarm.resource_type = RESOURCE_PIC;
	    alarm.is_notify = 1;
	    alarm.pic_buf = phdl->pic_buffer;
	    alarm.pic_size = phdl->pic_size;
	    alarm.valid = 1;
 		tuya_ipc_trigger_alarm_without_event(&alarm);  
 	}
    else if(phdl->status == DOORBELL_RECORD)
    {
        PR_DEBUG("no message leave\n");
        phdl->status = DOORBELL_LISTEN;
    }
    else if(phdl->status == DOORBELL_RECORDING)
    {
        PR_DEBUG("upload message\n");
        phdl->status = DOORBELL_LISTEN;
    }
    else if(phdl->status == DOORBELL_TALKING)
    {
        //talking timeout, reset doorbell ms
        PR_DEBUG("talking timeout ,reset doorbell ms\n");
        phdl->first_press = 0;
        phdl->status = DOORBELL_LISTEN;
    }
    return NULL;
}


void doorbell_mqtt_handler(int status)
{
    DOOR_BELL_MANAGER * phdl = doorbell_get_handler();

    //mqtt_cb_status : <0:accept 1:stop 2:heartbeat>
    if(status == 0)
    {
        if(phdl->first_press && phdl->status == DOORBELL_LISTEN)
        {  
            TUYA_ALARM_T alarm = {0};
		    alarm.type = E_ALARM_CONNECTED;
		    alarm.resource_type = RESOURCE_PIC;
		    alarm.is_notify = 1;
		    alarm.pic_buf = phdl->pic_buffer;
		    alarm.pic_size = phdl->pic_size;
		    alarm.valid = 1;
     		tuya_ipc_trigger_alarm_without_event(&alarm);
			
            phdl->status = DOORBELL_TALKING;
            tal_sw_timer_start(phdl->timer, DOORBELL_HEARTBEAT_DURATION, TAL_TIMER_ONCE);
        }
    }
    else if(status == 1)
    {
        if(phdl->status == DOORBELL_TALKING)
        {
            phdl->first_press = 0;
            phdl->status = DOORBELL_LISTEN;
            tal_sw_timer_trigger(phdl->timer);
            
        }
    }
    else if(status == 2)
    {
        if(phdl->status == DOORBELL_TALKING)
        {
            tal_sw_timer_start(phdl->timer, DOORBELL_HEARTBEAT_DURATION, TAL_TIMER_ONCE);
        }
    }
}


VOID doorbell_handler()
{
    DOOR_BELL_MANAGER * phdl = doorbell_get_handler();
    INT_T ret = 0;
    time_t cur_time = 0;

    if(phdl->status == DOORBELL_LISTEN)
    {
        if(phdl->timer == NULL)
        {
            if(OPRT_OK != tal_sw_timer_create((TAL_TIMER_CB)timer_proc, NULL, &(phdl->timer))){
                return;
            }
        }
 
        if(phdl->first_press == 0)
        {
            phdl->pic_size = PIC_MAX_SIZE;
            phdl->press_time = time(NULL);
            
            IPC_APP_get_snapshot(phdl->pic_buffer,&(phdl->pic_size));
            tuya_ipc_door_bell_press(DOORBELL_AC, phdl->pic_buffer, phdl->pic_size, NOTIFICATION_CONTENT_JPEG);
            tal_sw_timer_start(phdl->timer, DOORBELL_ALARM_DURATION, TAL_TIMER_ONCE);
            phdl->first_press = 1;
        }
    }
    else if(phdl->status == DOORBELL_RECORD)
    {
        phdl->pic_size = PIC_MAX_SIZE;
        IPC_APP_get_snapshot(phdl->pic_buffer,&phdl->pic_size);
        
		ret = tuya_ipc_leave_video_msg(phdl->pic_buffer,phdl->pic_size);
		if(ret != 0)
		{
			PR_ERR("#####tuya_ipc_doorbell_record_start failed \n");
		}
		else
		{
			phdl->status = DOORBELL_RECORDING;
			tal_sw_timer_start(phdl->timer, DOORBELL_MESSAGE_DURATION, TAL_TIMER_ONCE);
		}
        
    }
    else if(phdl->status == DOORBELL_RECORDING)
    {
        PR_DEBUG("recording...ignore!\n");
    }
  
    return;
}

VOID TUYA_APP_doorbell_event_cb(char* action)
{
     int status = 0;

    if(0 == memcmp(action,"accept",6))
    {
        status = 0;
    }
    else if(0 == memcmp(action,"stop",4))
    {
        status = 1;
    }
    else if(0 == memcmp(action,"heartbeat",9))
    {
        status = 2;
    }
    doorbell_mqtt_handler(status);
    return;
}
OPERATE_RET TUYA_APP_Enable_Video_Msg(TUYA_IPC_SDK_VIDEO_MSG_S* p_video_msg_info)
{
    return tuya_ipc_video_msg_init(p_video_msg_info->type, p_video_msg_info->msg_duration);
}
