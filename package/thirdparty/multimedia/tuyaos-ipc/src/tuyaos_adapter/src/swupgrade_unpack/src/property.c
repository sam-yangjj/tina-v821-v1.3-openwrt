/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#include "property.h"
#include "bcmnvram.h"

int prop_get(const char *key, char *value)
{
	if (!key || !value)
		return -1;

	return nvram_get(key, value);
}

int prop_set(const char *key, const char *value)
{
	int ret;

	if (!key || !value)
		return -1;

	ret = nvram_set(key, value);
	if (ret)
		return -1;

	return nvram_commit();
}

int prop_get_int(const char *key)
{
	return nvram_get_int(key);
}

int prop_set_int_temp(const char *key, int value)
{
	return nvram_set_int_temp(key, value);
}

int prop_set_temp(const char *key, const char *value)
{
	return nvram_set_temp(key, value);
}
