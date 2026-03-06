#ifndef __AWISPAPI_H__
#define __AWISPAPI_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	int (*ispApiInit)(void);
	int (*ispGetIspId)(int video_id);
	int (*ispStart)(int isp_id);
	int (*ispStop)(int isp_id);
	int (*ispWaitToExit)(int isp_id);
	int (*ispApiUnInit)(void);
	int (*ispSetAeTarget)(int isp_id, void *target_val);
	int (*ispSetAeWeight)(int isp_id, void *msg);
	int (*ispSetAeTable)(int isp_id, void *msg);
	int (*ispSetSync)(int mode);


}AWIspApi;

AWIspApi *CreateAWIspApi(void);
void DestroyAWIspApi(AWIspApi *hdl);

#ifdef  __cplusplus
}
#endif

#endif  /* __AWISPAPI_H__ */
