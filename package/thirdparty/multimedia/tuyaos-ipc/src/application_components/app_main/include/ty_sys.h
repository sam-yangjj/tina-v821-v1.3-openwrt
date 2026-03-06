/*
 * Copyright (C), 2020-2022, tuya. All rights reserved.
 * File name  : ty_sys.h
 * Author     : tt
 * Date       : 2020/07/24
 * Description: main proc
 * Others     :
 */

#ifndef _TY_SYS_H_
#define _TY_SYS_H_

#if defined(__cplusplus)
extern "C" {
#endif

int ty_sys_start(int mode, char * token);

int ty_sys_loop();

int ty_sys_stop();


#if defined(__cplusplus)
}
#endif

#endif


