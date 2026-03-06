/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#include <stdio.h>
#include "notifier.h"

void notify(int level, const char *msg)
{
	switch (level) {
	case ERRORLEVEL:
		fprintf(stderr, "[ERROR]");
		break;
	case WARNLEVEL:
		fprintf(stdout, "[WARN ]");
		break;
	case INFOLEVEL:
		fprintf(stdout, "[INFO ]");
		break;
	case DEBUGLEVEL:
		fprintf(stdout, "[DEBUG]");
		break;
	case TRACELEVEL:
		fprintf(stdout, "[TRACE]");
		break;
	}

	fprintf(level == ERRORLEVEL ? stderr : stdout, " : %s\n", msg ? msg : "");
	fflush(stdout);
}
