/**
* @file      tuya_ipc_move_path.h
* @brief     tuya ipc move api
* @version   1.0
* @date      2021-11-18
*
* copyright  Copyright (c) tuya.inc 2025
*/
#ifndef __TUYA_IPC_MOVE_PATH_H__
#define __TUYA_IPC_MOVE_PATH_H__
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
* @struct TUYA_IPC_MOVE_POINT_T
* @brief  
*/
typedef struct {
    UINT64_T        id;    /** id in server */
    CHAR_T          name[32];  /** move point name */
    INT_T           mpId;      /** index ID */
    CHAR_T          coords[256];       /** coordinates info */
}TUYA_IPC_MOVE_POINT_T;

typedef struct {
    UINT64_T        id;    /** id in server */    
    INT_T           mpId;      /** index ID */
}TUYA_IPC_MOVE_POINT_KEY_INFO_T;

typedef struct {
    UINT64_T        pathId;
    CHAR_T          pathName[32];
    INT_T           pointCnt;
    TUYA_IPC_MOVE_POINT_KEY_INFO_T  * pointList;
}TUYA_IPC_MOVE_PATH_T;

typedef enum {
    MOVE_PATH_STAT_IDLE = 0,
    MOVE_PATH_STAT_RUNNING
}TUYA_IPC_MOVE_PATH_STAT_E;

/**
* @brief      add one move point for ipc
* @param[in]  movePoint    point info
* @param[out]  movePoint->id    point id return from cloud
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_point_add(INOUT TUYA_IPC_MOVE_POINT_T *movePoint);


/**
* @brief      del one move point for ipc
* @param[in]  id    TUYA_IPC_MOVE_POINT_T->id
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_point_delete(IN UINT64_T id);

/**
* @brief      get all move positions stored in tuya cloud for ipc
* @param[out] movePoint   move point info ,call tuya_ipc_move_point_list_free to free mem
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_point_list_get(OUT INT_T * cnt, OUT TUYA_IPC_MOVE_POINT_T ** movePoint);

/**
* @brief      free move point mem 
* @param[in] movePoint   move point info 
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_point_list_free(OUT TUYA_IPC_MOVE_POINT_T * movePoint);

/**
* @brief      upload a snapshort for current preset position for ipc
* @param[in]  mpId    used for mapping which motion point the image belongs to
* @param[in]  addr    address of the picture to be uploaded
* @param[in]  size    size of the picture to be uploaded
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_point_add_pic(IN INT_T mpId, IN CHAR_T *addr, IN UINT_T size);

/**
* @brief      add one move path for first time
* @param[in]  pathName    
* @param[out]  pathId    id for app
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_path_add(IN CHAR_T * pathName, OUT UINT64_T * pathId);

/**
* @brief      update one move point for ipc
* @param[in]  pathName    path name 
* @param[in]  pathId    path id
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_path_update(IN TUYA_IPC_MOVE_PATH_T * movePath);

/**
* @brief      del one move path with many point for ipc
* @param[in]  preset_id    preset_id is the "devId field of the JSON of DP TUYA_DP_PRESET_SET"
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_path_delete(IN UINT64_T pathId);

/**
* @brief      get all move path info stored in tuya cloud for ipc
* @param[out] cnt    move path cnt
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_path_list_get(OUT INT_T * cnt, TUYA_IPC_MOVE_PATH_T ** movePath);

/**
* @brief      free move path mem malloc by tuya_ipc_move_path_list_get 
* @param[in]  movePath    move path
* @param[in]  cnt    move path cnt
* @return     OPERATE_RET
* - OPRT_OK   success
* - Others    failed
*/
OPERATE_RET tuya_ipc_move_path_list_free(IN TUYA_IPC_MOVE_PATH_T * movePath, IN INT_T cnt);


#ifdef __cplusplus
}
#endif

#endif

