/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */


#ifndef _HANDLER_H
#define _HANDLER_H

#include "tuya_unpack.h"

typedef int (*handler)(char *dev, int offs, ota_image *pimg);
typedef int (*handler_wh)(char *dev, ota_image *pimg, int offs, char *data, int len);
typedef int (*handler_bk)(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);

typedef struct installer_handler {
	//char	desc[64];
	char	*desc;
	handler installer;
    handler_wh installer_wh;
    handler_bk installer_bk;
	void	*data;
} installer_handler;

int register_handler(const char *desc, handler installer);

installer_handler *find_handler(ota_image *pimg);

void print_registered_handlers(void);

#endif
