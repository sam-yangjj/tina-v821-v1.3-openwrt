/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 *
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <linux/types.h>

#define NOTIFY_BUF_SIZE 	2048

// TODO: set loglevel
//int loglevel = 1;

typedef enum {
	OFF,
	ERRORLEVEL,
	WARNLEVEL,
	INFOLEVEL,
	DEBUGLEVEL,
	TRACELEVEL,
	LASTLOGLEVEL=TRACELEVEL
} LOGLEVEL;

void notify(int level, const char *msg);

#define swupgrade_notify(format, level, arg...) do { \
	if (level >= level) { \
		char tmpbuf[NOTIFY_BUF_SIZE]; \
		snprintf(tmpbuf, sizeof(tmpbuf), \
		     	"%s : %s : %d : " format, \
		       	__FILE__, \
		       	__func__, \
		       	__LINE__, \
			## arg); \
		notify(level, tmpbuf); \
	} \
} while(0)

#define ERROR(format, arg...) \
	swupgrade_notify(format, ERRORLEVEL, ## arg)

#define WARN(format, arg...) \
	swupgrade_notify(format, WARNLEVEL, ## arg)

#define INFO(format, arg...) \
	swupgrade_notify(format, INFOLEVEL, ## arg)

#define TRACE(format, arg...) \
	swupgrade_notify(format, TRACELEVEL, ## arg)

#define DEBUG(format, arg...) \
	swupgrade_notify(format, DEBUGLEVEL, ## arg)

#endif
