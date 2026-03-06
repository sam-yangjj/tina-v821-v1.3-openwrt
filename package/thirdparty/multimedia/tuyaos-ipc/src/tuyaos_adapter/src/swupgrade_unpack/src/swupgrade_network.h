/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com
 */

#ifndef _SWUPGRADE_NETWORK_H
#define _SWUPGRADE_NETWORK_H

void *network_initializer(void *data);

int listener_create(const char *path, int type);

#endif
