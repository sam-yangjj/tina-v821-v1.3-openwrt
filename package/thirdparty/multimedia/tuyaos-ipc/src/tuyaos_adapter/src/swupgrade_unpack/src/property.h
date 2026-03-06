/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#ifndef _PROPERTY_H
#define _PROPERTY_H

#define PROP_MAX_LEN 256

int prop_get(const char *key, char *value);

int prop_set(const char *key, const char *value);

int prop_get_int(const char *key);

int prop_set_int_temp(const char *key, int value);

int prop_set_temp(const char *key, const char *value);

#endif
