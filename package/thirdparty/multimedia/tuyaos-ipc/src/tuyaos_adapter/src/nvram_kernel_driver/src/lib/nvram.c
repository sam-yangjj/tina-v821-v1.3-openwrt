/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bcmnvram.h"

#define PATH_DEV_NVRAM	"/dev/nvram"

#ifdef USE_RET_VAL_MODE

#define MAX_CACHE_LEN	(2*NVRAM_MAX_VALUE_LEN)
#define MIN_CACHE_RES	32

char *nvram_get_(const char *name)
{
	static int  nvr_index = 0;
	static char nvr_cache[MAX_CACHE_LEN] = {0};
	int ret, i_cache, nvram_fd;
	anvram_ioctl_t nvr;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return NULL;
	}

	i_cache = nvr_index;
	if ((MAX_CACHE_LEN - i_cache) < MIN_CACHE_RES)
		i_cache = 0;

	nvr.size = sizeof(nvr);
	nvr.len_param = strlen(name);
	nvr.len_value = MAX_CACHE_LEN - i_cache;
	nvr.param = (char*)name;
	nvr.value = &nvr_cache[i_cache];

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	if (ret < 0) {
		if (errno == EOVERFLOW && nvr.len_value > 0 && nvr.len_value <=
				MAX_CACHE_LEN) {
			if ((MAX_CACHE_LEN - i_cache) < nvr.len_value)
				i_cache = 0;
			nvr.len_value = MAX_CACHE_LEN - i_cache;
			nvr.value = &nvr_cache[i_cache];
			ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
		}
		if (ret < 0) {
			perror(PATH_DEV_NVRAM);
			close(nvram_fd);
			return NULL;
		}
	}

	if (nvr.len_value < 1)
		nvr.value = NULL;
	else
		nvr_index = i_cache + nvr.len_value;

	close(nvram_fd);

	return nvr.value;
}

char *nvram_get(const char *name)
{
	return nvram_get_(name);
}

char *nvram_safe_get(const char *name)
{
	static char nvram_empty[4] = {0};

	char *value = nvram_get_(name);
	if (value)
		return value;
	else
		return nvram_empty;
}

int nvram_get_int(const char *name)
{
	char *value = nvram_get_(name);
	if (value)
		return atoi(value);
	else
		return 0;
}

int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max)
{
	int i_value = nvram_get_int(name);
	if (i_value < val_min)
		i_value = val_def;
	else if (i_value > val_max)
		i_value = val_def;
	return i_value;
}

#else

int nvram_get_(const char *name, char *value)
{
	int ret, nvram_fd;
	anvram_ioctl_t nvr;

	if (!name || !value)
		return -1;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	nvr.size = sizeof(nvr);
	nvr.len_param = strlen(name);
	nvr.len_value = NVRAM_MAX_VALUE_LEN;
	nvr.param = (char*)name;
	nvr.value = value;

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	if (ret < 0) {
		perror(PATH_DEV_NVRAM);
		close(nvram_fd);
		return -1;
	}

	if (nvr.len_value < 1) {
		nvr.value = NULL;
		close(nvram_fd);
		return -1;
	}

	close(nvram_fd);

	return 0;
}

int nvram_get(const char *name, char *value)
{
	return nvram_get_(name, value);
}

int nvram_get_int(const char *name)
{
	int ret;
	char value[NVRAM_MAX_VALUE_LEN] = {0};

	ret = nvram_get_(name, value);
	if (!ret)
		return atoi(value);
	else
		return 0;
}

int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max)
{
	int i_value = nvram_get_int(name);
	if (i_value < val_min)
		i_value = val_def;
	else if (i_value > val_max)
		i_value = val_def;
	return i_value;
}

#endif

int nvram_getall(char *buf, int count, int include_temp)
{
	int ret, nvram_fd;
	anvram_ioctl_t nvr;

	if (!buf || count < 1)
		return 0;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	/* Get all variables */
	*buf = '\0';

	nvr.size = sizeof(nvr);
	nvr.len_param = 0;
	nvr.len_value = count;
	nvr.param = NULL;
	nvr.value = buf;
	nvr.is_temp = include_temp;

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}

int _nvram_set(const char *name, const char *value, int is_temp)
{
	int ret, nvram_fd;
	anvram_ioctl_t nvr;

	if (value && strlen(value) > NVRAM_MAX_VALUE_LEN)
		return -1;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	nvr.size = sizeof(nvr);
	nvr.len_param = strlen(name);
	nvr.len_value = 0;
	nvr.param = (char*)name;
	nvr.value = (char*)value;
	nvr.is_temp = is_temp;
	if (value)
		nvr.len_value = strlen(value);

//	printf("%s: name: %s value: %s attr: 0x%x\n", __func__, name, value, is_temp);

	ret = ioctl(nvram_fd, NVRAM_IOCTL_SET, &nvr);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}

int nvram_set(const char *name, const char *value)
{
	return _nvram_set(name, value, 0);
}

int nvram_set_temp(const char *name, const char *value)
{
	return _nvram_set(name, value, ATTR_TEMP);
}

int nvram_set_with_attr(const char *name, const char *value, int attr)
{
	return _nvram_set(name, value, attr);
}

int nvram_set_int(const char *name, int value)
{
	char int_str[16];
	sprintf(int_str, "%d", value);
	return _nvram_set(name, int_str, 0);
}

int nvram_set_int_temp(const char *name, int value)
{
	char int_str[16];
	sprintf(int_str, "%d", value);
	return _nvram_set(name, int_str, ATTR_TEMP);
}

int nvram_unset(const char *name)
{
	return _nvram_set(name, NULL, 0);
}

#ifdef USE_RET_VAL_MODE

int nvram_match(const char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

int nvram_invmatch(const char *name, char *invmatch)
{
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

#else

int nvram_match(const char *name, char *match)
{
	char value[NVRAM_MAX_VALUE_LEN] = {0};

	if (nvram_get(name, value))
		return 0;

	return !strcmp(value, match);
}

int nvram_invmatch(const char *name, char *invmatch)
{
	char value[NVRAM_MAX_VALUE_LEN] = {0};

	if (nvram_get(name, value))
		return 1;

	return strcmp(value, invmatch);
}

#endif

int nvram_commit(void)
{
	int ret, nvram_fd;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	ret = ioctl(nvram_fd, NVRAM_IOCTL_COMMIT, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}

int nvram_erase(void)
{
	int ret, nvram_fd;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	ret = ioctl(nvram_fd, NVRAM_IOCTL_ERASE, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}

int nvram_clear(void)
{
	int ret, nvram_fd;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	ret = ioctl(nvram_fd, NVRAM_IOCTL_CLEAR, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}
