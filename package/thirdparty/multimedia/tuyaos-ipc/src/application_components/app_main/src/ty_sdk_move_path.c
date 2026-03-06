#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>

#include "tkl_memory.h"

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "ty_sdk_common.h"

#define  MOVE_PATH_DEMO_ENABLE  0

#if defined(MOVE_PATH_DEMO_ENABLE)&&(MOVE_PATH_DEMO_ENABLE==1)
#include "tuya_ipc_move_path.h"

static INT_T s_mpId = 0;

VOID TUYA_IPC_MOVE_PATH_ADD_DEMO()
{
    OPERATE_RET ret = OPRT_OK;
    INT_T  pointCnt = 3;
    UINT64_T pathId = 0;
    CHAR_T movePathName[32] = {0};
    snprintf(movePathName, sizeof(movePathName), "TEST_PATH_%d", tal_time_get_posix());

    //添加轨迹流程，获取当前路径添加状态
    ret = tuya_ipc_move_path_add(movePathName, &pathId);
    if (OPRT_OK != ret){
        PR_ERR("move path add failed");
        return ;
    }
    PR_DEBUG("add path success pathId[%llu]", pathId);
    TUYA_IPC_MOVE_PATH_T strMovePath = {0};
    strMovePath.pointList = tkl_system_malloc(sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*pointCnt);
    if (NULL == strMovePath.pointList){
        PR_ERR("malloc failed [%d]", pointCnt);
        return ;
    }
    INT_T i = 0;
    for (i = 0 ; i < pointCnt; i++){
        TUYA_IPC_MOVE_POINT_T movePoint = {0};
        movePoint.mpId = s_mpId++;
        PR_ERR("move path step 1");
        snprintf(movePoint.name, sizeof(movePoint.name), "Point_name_%d", tal_time_get_posix()); //暂时写死调试，后续通过APP-DP下发
        snprintf(movePoint.coords, sizeof(movePoint.coords), "Coords_info_%d", tal_time_get_posix());   //暂时写死调试，后续设备上报自己得坐标信息
        ret = tuya_ipc_move_point_add(&movePoint);
        if (OPRT_OK != ret){
            PR_ERR("move point add failed");
            tkl_system_free(strMovePath.pointList);
            return ;
        }
        PR_ERR("move path step 1");
        char snap_addr[100*1024] = {0};
        int snap_size = 100*1024;
        IPC_APP_get_snapshot(snap_addr,&snap_size);
        PR_ERR("move path step 1");
        ret = tuya_ipc_move_point_add_pic(movePoint.mpId, snap_addr, snap_size);
        if (OPRT_OK != ret){
            PR_ERR("move point add mpId[%d] pic failed", movePoint.mpId);
            tkl_system_free(strMovePath.pointList);
            return ;
        }
        PR_ERR("move path step 2");
        sleep(1);
        //添加完节点后需要更新路径
        strMovePath.pathId = pathId;
        strncpy(strMovePath.pathName, movePathName, strlen(movePathName));
        strMovePath.pointList[strMovePath.pointCnt].id = movePoint.id;
        strMovePath.pointList[strMovePath.pointCnt].mpId = movePoint.mpId;
        strMovePath.pointCnt++;
        PR_ERR("move path step 3");
        sleep(1);
        ret = tuya_ipc_move_path_update(&strMovePath);
        if (OPRT_OK != ret){
            PR_ERR("move path update failed");
        }else{
            PR_DEBUG("move path update suc [%llu]",strMovePath.pathId);
        }
        //为了名称时间错开，加休眠
        sleep(2);
    }
    tkl_system_free(strMovePath.pointList);
    return ;
}

VOID TUYA_IPC_MOVE_PATH_DELETE_DEMO()
{
    OPERATE_RET ret = OPRT_OK;

    INT_T cnt = 0;
    TUYA_IPC_MOVE_PATH_T * movePath = NULL;
    ret = tuya_ipc_move_path_list_get(&cnt, &movePath);
    if (OPRT_OK != ret || 0 == cnt){
        PR_ERR("move path list get failed or cnt[%d]",cnt);
    }else{
        PR_DEBUG("get path info cnt[%d]",cnt);
        INT_T i = 0;
        for (i = 0; i < cnt; i++){
            PR_DEBUG("move path info pathId-name-pointCnt[%llu-%s-%d]", movePath[i].pathId,movePath[i].pathName, movePath[i].pointCnt);
            INT_T  j = 0;
            for (j = 0; j < movePath[i].pointCnt; j++){
                PR_DEBUG("move path info pathId[%llu][%llu-%d]", movePath[i].pathId, movePath[i].pointList[j].id, movePath[i].pointList[j].mpId);
            }
            PR_DEBUG("delete pathId [%llu]", movePath[i].pathId);
            ret = tuya_ipc_move_path_delete(movePath[i].pathId);
            if (OPRT_OK != ret){
                PR_ERR("delete pathId [%s] failed", movePath[i].pathId);
            }
        }
        ret = tuya_ipc_move_path_list_free(movePath, cnt);
        if (OPRT_OK != ret){
            PR_ERR("move path list free failed");
        }
    }

    return ;

}
VOID TUYA_IPC_MOVE_PATH_UPDATE_DEMO()
{
    OPERATE_RET ret = OPRT_OK;

    INT_T cnt = 0;
    TUYA_IPC_MOVE_PATH_T * movePath = NULL;
    ret = tuya_ipc_move_path_list_get(&cnt, &movePath);
    if (OPRT_OK != ret || 0 == cnt){
        PR_ERR("move path list get failed or cnt[%d]",cnt);
    }else{
        PR_DEBUG("get path info cnt[%d] pathName[%s]",cnt, movePath[0].pathName);
        TUYA_IPC_MOVE_POINT_T movePoint = {0};
        movePoint.mpId = s_mpId++;
        PR_ERR("move path step 1");
        snprintf(movePoint.name, sizeof(movePoint.name), "Point_name_%d", tal_time_get_posix()); //暂时写死调试，后续通过APP-DP下发
        snprintf(movePoint.coords, sizeof(movePoint.coords), "Coords_info_%d", tal_time_get_posix());   //暂时写死调试，后续设备上报自己得坐标信息
        ret = tuya_ipc_move_point_add(&movePoint);
        if (OPRT_OK != ret){
            PR_ERR("move point add failed");
            return ;
        }
        PR_ERR("move path step 1");
        char snap_addr[100*1024] = {0};
        int snap_size = 100*1024;
        IPC_APP_get_snapshot(snap_addr,&snap_size);
        PR_ERR("move path step 1");
        ret = tuya_ipc_move_point_add_pic(movePoint.mpId, snap_addr, snap_size);
        if (OPRT_OK != ret){
            PR_ERR("move point add mpId[%d] pic failed", movePoint.mpId);
            return ;
        }
        TUYA_IPC_MOVE_PATH_T strMovePath = {0};
        strMovePath.pointList = tkl_system_malloc(sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*(movePath[0].pointCnt+1));
        if (NULL == strMovePath.pointList){
            PR_ERR("malloc failed [%d]", movePath[0].pointCnt);
        }
        strMovePath.pathId = movePath[0].pathId;
        strncpy(strMovePath.pathName, movePath[0].pathName, strlen(movePath[0].pathName));
        strMovePath.pointCnt = movePath[0].pointCnt + 1;
        memcpy(strMovePath.pointList, movePath[0].pointList, sizeof(TUYA_IPC_MOVE_POINT_KEY_INFO_T)*movePath[0].pointCnt);
        //最后以为新增信息
        strMovePath.pointList[movePath[0].pointCnt].id = movePoint.id;
        strMovePath.pointList[movePath[0].pointCnt].mpId = movePoint.mpId;
        ret = tuya_ipc_move_path_update(&strMovePath);
        if (OPRT_OK != ret){
            PR_ERR("move path update failed");
        }else{
            PR_DEBUG("move path update suc [%llu]",strMovePath.pathId);
        }
        tkl_system_free(strMovePath.pointList);
    }

    return ;
}
//已经存在的路径上面添加节点
VOID TUYA_IPC_MOVE_PATH_ADD_POINT_DEMO()
{
    OPERATE_RET ret = OPRT_OK;

    INT_T cnt = 0;
    TUYA_IPC_MOVE_PATH_T * movePath = NULL;
    ret = tuya_ipc_move_path_list_get(&cnt, &movePath);
    if (OPRT_OK != ret || 0 == cnt){
        PR_ERR("move path list get failed or cnt[%d]",cnt);
    }else{
        PR_DEBUG("get path info cnt[%d] pathName[%s]",cnt, movePath[0].pathName);
        memset(movePath[0].pathName, 0x00, sizeof(movePath[0].pathName));
        snprintf(movePath[0].pathName, sizeof(movePath[0].pathName), "UPDATE_pathName_test");
        ret = tuya_ipc_move_path_update(&movePath[0]);
        if (OPRT_OK != ret){
            PR_ERR("move path update failed");
        }
    }

    return ;
}

VOID TUYA_IPC_MOVE_POINT_DELETE_DEMO()
{
    OPERATE_RET ret = OPRT_OK;
    INT_T cnt = 0;
    TUYA_IPC_MOVE_POINT_T * movePoint = NULL;
    ret = tuya_ipc_move_point_list_get(&cnt, &movePoint);
    if (OPRT_OK != ret || 0 == cnt){
        PR_ERR("move point list get failed or cnt == 0");
    }else{
        PR_DEBUG("get point info cnt[%d]",cnt);
        INT_T i = 0;
        for (i = 0; i < cnt; i++){
            PR_DEBUG("move point info id-name-mpId[%llu-%s-%llu]", movePoint[i].id,movePoint[i].name, movePoint[i].mpId);
            PR_DEBUG("delete point [%llu]", movePoint[i].id);
            ret = tuya_ipc_move_point_delete(movePoint[i].id);
            if (OPRT_OK != ret){
                PR_ERR("delete point [%s] failed", movePoint[i].id);
            }
        } 
        ret = tuya_ipc_move_point_list_free(movePoint);
        if (OPRT_OK != ret){
            PR_ERR("delete point list free failed");
        }     
    } 
    //再次获取list
    ret = tuya_ipc_move_point_list_get(&cnt, &movePoint);
    if (OPRT_OK != ret){
        PR_ERR("move point list get failed");
    }else{
        PR_DEBUG("get point info cnt[%d]",cnt);
    }

    return ;
}
#endif