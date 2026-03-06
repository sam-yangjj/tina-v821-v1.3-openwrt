/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#ifndef _SWUPGRADE_PROGRESS_H
#define _SWUPGRADE_PROGRESS_H

#include "progress_ipc.h"

void swupgrade_progress_init();
void swupgrade_progress_update(unsigned int perc);
void swupgrade_progress_end(RECOVERY_STATUS status);
void *progress_bar_thread(void *data);

#endif
