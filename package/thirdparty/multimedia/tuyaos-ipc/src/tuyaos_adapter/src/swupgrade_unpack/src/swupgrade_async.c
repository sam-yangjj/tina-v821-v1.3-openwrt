/*
 * (C) Copyright 2020
 * Cheng Lei, TUYA Software Engineering, chenglei@tuya.com.
 * Tuya pack code
 */

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "tuya_unpack.h"
#include "notifier.h"
#include "bsdqueue.h"
#include "swupgrade_async.h"

struct data_slice {
	SIMPLEQ_ENTRY(data_slice) next;
	char *buf;
	int len;
};

SIMPLEQ_HEAD(datalist, data_slice);

static struct datalist fw_slices;

static pthread_t swupgrade_daemon;

static pthread_t start_thread(void *(* start_routine) (void *), void *arg)
{
	int ret;
	pthread_t id;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	//pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024 * 1024);

	ret = pthread_create(&id, &attr, start_routine, arg);
	if (ret) {
		ERROR("create thread failed.");
		return -1;
	}

	pthread_attr_destroy(&attr);

	return id;
}

static void *swupgrade_async_thread(void *data)
{
	ota_stream *obj;
	struct data_slice *slice, *tmp;
	int ret = 0;

	obj = (ota_stream *)data;

	while (!obj->abort) {
		pthread_mutex_lock(&obj->ota_mutex);
		//pthread_cond_wait(&obj->ota_cond, &obj->ota_mutex);
		//TRACE("wakeup...");
		SIMPLEQ_FOREACH_SAFE(slice, &fw_slices, next, tmp) {
			SIMPLEQ_REMOVE(&fw_slices, slice, data_slice, next);
			pthread_mutex_unlock(&obj->ota_mutex);
			ret = device_upgrade_feed_internal(obj, slice->buf, slice->len);
			free(slice->buf);
			free(slice);
			slice = NULL;
			pthread_mutex_lock(&obj->ota_mutex);
			if (ret)
				break;
		}
		pthread_mutex_unlock(&obj->ota_mutex);
		if (ret)
			break;
		//TRACE("in while end ....");
	}

	TRACE("async thread done.");
	return data;
}

int swupgrade_async_start(ota_stream *obj)
{
	SIMPLEQ_INIT(&fw_slices);
	pthread_mutex_init(&obj->ota_mutex, NULL);
	pthread_cond_init(&obj->ota_cond, NULL);

	swupgrade_daemon = start_thread(swupgrade_async_thread, obj);
	if (swupgrade_daemon < 0)
		return -1;

	return 0;
}

int swupgrade_async_write(ota_stream *obj, char *data, int data_len)
{
	struct data_slice *slice;

	slice = calloc(1, sizeof(struct data_slice));
	if (!slice) {
		ERROR("no memory: alloc failed");
		return -ENOMEM;
	}
	slice->len = data_len;
	slice->buf = calloc(1, data_len);
	if (!slice->buf) {
		ERROR("no memory: alloc failed");
		free(slice);
		return -ENOMEM;
	}
	memcpy(slice->buf, data, data_len);
	//TRACE("slice len %d data len %d", slice->len, data_len);

	pthread_mutex_lock(&obj->ota_mutex);
	SIMPLEQ_INSERT_TAIL(&fw_slices, slice, next);
	//pthread_cond_signal(&obj->ota_cond);
	pthread_mutex_unlock(&obj->ota_mutex);

	return 0;
}

int swupgrade_async_close(ota_stream *obj)
{
	struct data_slice *slice, *tmp;

	//pthread_mutex_lock(&obj->ota_mutex);
	//pthread_cond_signal(&obj->ota_cond);
	//pthread_mutex_unlock(&obj->ota_mutex);

	pthread_join(swupgrade_daemon, NULL);

	SIMPLEQ_FOREACH_SAFE(slice, &fw_slices, next, tmp) {
		SIMPLEQ_REMOVE(&fw_slices, slice, data_slice, next);
		free(slice->buf);
		free(slice);
		slice = NULL;
	}

	pthread_mutex_destroy(&obj->ota_mutex);
	pthread_cond_destroy(&obj->ota_cond);

	return 0;
}
