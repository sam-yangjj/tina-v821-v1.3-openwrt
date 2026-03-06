/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 *
 */

#ifndef _SWUPGRADE_ASYNC_H
#define _SWUPGRADE_ASYNC_H

#include "tuya_unpack.h"

int swupgrade_async_start(ota_stream *obj);
int swupgrade_async_write(ota_stream *obj, char *data, int data_len);
int swupgrade_async_close(ota_stream *obj);

#endif
