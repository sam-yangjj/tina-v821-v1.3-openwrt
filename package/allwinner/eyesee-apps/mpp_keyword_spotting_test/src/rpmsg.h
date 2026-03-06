/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2016, Linaro Ltd.
 */

#ifndef _UAPI_RPMSG_H_
#define _UAPI_RPMSG_H_

#include <linux/ioctl.h>
#include <linux/types.h>

#define RPMSG_ADDR_ANY		0xFFFFFFFF

/**
 * struct rpmsg_endpoint_info - endpoint info representation
 * @name: name of service
 * @src: local address. To set to RPMSG_ADDR_ANY if not used.
 * @dst: destination address. To set to RPMSG_ADDR_ANY if not used.
 */
struct rpmsg_endpoint_info {
	char name[32];
	__u32 src;
	__u32 dst;
};

/**
 * Instantiate a new rmpsg char device endpoint.
 */
#define RPMSG_CREATE_EPT_IOCTL	_IOW(0xb5, 0x1, struct rpmsg_endpoint_info)

/**
 * Destroy a rpmsg char device endpoint created by the RPMSG_CREATE_EPT_IOCTL.
 */
#define RPMSG_DESTROY_EPT_IOCTL	_IO(0xb5, 0x2)

/**
 * Instantiate a new local rpmsg service device.
 */
#define RPMSG_CREATE_DEV_IOCTL	_IOW(0xb5, 0x3, struct rpmsg_endpoint_info)

/**
 * Release a local rpmsg device.
 */
#define RPMSG_RELEASE_DEV_IOCTL	_IOW(0xb5, 0x4, struct rpmsg_endpoint_info)

/**
 * RPMSG_CREATE_EPT_IOCTL:
 *     Create the endpoint specified by info.name,
 *     updates info.id.
 * RPMSG_DESTROY_EPT_IOCTL:
 *     Destroy the endpoint specified by info.id.
 * RPMSG_REST_EPT_GRP_IOCTL:
 *     Destroy all endpoint belonging to info.name
 * RPMSG_DESTROY_ALL_EPT_IOCTL:
 *     Destroy all endpoint
 * RPMSG_CREATE_AF_EPT_IOCTL:
 *     Create the endpoint specified by info.name,
 *     updates info.id.
 *     It will automatically destroy the endpoint when closing file descriptor.
 */
#define RPMSG_CREATE_EPT_IOCTL	_IOW(0xb5, 0x1, struct rpmsg_endpoint_info)
#define RPMSG_DESTROY_EPT_IOCTL	_IO(0xb5, 0x2)
#define RPMSG_REST_EPT_GRP_IOCTL	_IO(0xb5, 0x3)
#define RPMSG_DESTROY_ALL_EPT_IOCTL	_IO(0xb5, 0x4)
#define RPMSG_CREATE_AF_EPT_IOCTL	_IOW(0xb5, 0x5, struct rpmsg_endpoint_info)

#define RPMSG_EPTDEV_DELIVER_PERF_DATA_IOCTL _IOW(0xb6, 0x1, int)

/**
 * struct rpmsg_ctrl_msg - used by rpmsg_master.c
 * @name: user define
 * @id: update by driver
 * @cmd:only can RPMSG_CTRL_OPEN or RPMSG_CTRL_CLOSE
 * */
struct rpmsg_ept_info {
	char name[32];
	uint32_t id;
};

#endif
