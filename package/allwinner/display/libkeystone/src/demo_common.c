/*
 * This confidential and proprietary software should be used
 * under the licensing agreement from Allwinner Technology.

 * Copyright (C) 2020-2030 Allwinner Technology Limited
 * All rights reserved.

 * Author:zhengwanyu <liuyuhang@allwinnertech.com>

 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from Allwinner Technology Limited.
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#include "demo_common.h"
unsigned int getDrmPixelFormat(char *name)
{
	if (!strcmp("nv21", name))
		return DRM_FORMAT_NV21;
	else if (!strcmp("nv12", name))
		return DRM_FORMAT_NV12;
	else if (!strcmp("nv16", name))
		return DRM_FORMAT_NV16;
	else if (!strcmp("yuyv", name))
		return DRM_FORMAT_YUYV;
	else if (!strcmp("bgr565", name))
		return DRM_FORMAT_BGR565;
	else if (!strcmp("abgr1555", name))
		return DRM_FORMAT_ABGR1555;
	else if (!strcmp("abgr4444", name))
		return DRM_FORMAT_ABGR4444;
	else if (!strcmp("rgb888", name))
		return DRM_FORMAT_RGB888;
	else if (!strcmp("bgr888", name))
		return DRM_FORMAT_BGR888;
	else if (!strcmp("argb8888", name))
		return DRM_FORMAT_ARGB8888;
	else if (!strcmp("argb8888", name))
		return DRM_FORMAT_ARGB8888;
	else if (!strcmp("abgr8888", name))
		return DRM_FORMAT_ABGR8888;
	else if (!strcmp("rgba8888", name))
		return DRM_FORMAT_RGBA8888;
	else if (!strcmp("bgra8888", name))
		return DRM_FORMAT_BGRA8888;
	else if (!strcmp("yuv420", name))
		return DRM_FORMAT_YUV420;
	else if (!strcmp("yuv444", name))
		return DRM_FORMAT_YUV444;
	else if (!strcmp("afbc_yuv420_8", name))
		return DRM_FORMAT_YUV420_8BIT;
	else if (!strcmp("afbc_yuv420_10", name))
		return DRM_FORMAT_YUV420_10BIT;

	printf("ERROR: have NOT been surpported pixel format:%s\n", name);
	return 0;
}

unsigned int getPictureSize(unsigned int W, unsigned int H,
            unsigned int format)
{
	if (format == DRM_FORMAT_BGRA8888
		|| format == DRM_FORMAT_RGBA8888
		|| format == DRM_FORMAT_ARGB8888
		|| format == DRM_FORMAT_ABGR8888)
		return W * H * 4;
	else if (format == DRM_FORMAT_RGB888
		|| format == DRM_FORMAT_BGR888
		|| format == DRM_FORMAT_P010
		|| format == DRM_FORMAT_P016
		|| format == DRM_FORMAT_YUV444
		|| format == DRM_FORMAT_YUV420_10BIT)
		return W * H * 3;
	else if (format == DRM_FORMAT_NV21
			|| format == DRM_FORMAT_NV12
			|| format == DRM_FORMAT_YUV420
			|| format == DRM_FORMAT_YUV420_8BIT)
		return W * H * 3 / 2;
	else if (format == DRM_FORMAT_NV61
			|| format == DRM_FORMAT_NV16
			|| format == DRM_FORMAT_YUYV
			|| format == DRM_FORMAT_BGR565
			|| format == DRM_FORMAT_ABGR1555
			|| format == DRM_FORMAT_ABGR4444)
		return W * H * 2;

	printf("ERROR: have NOT been surpported pixel format:0x%x\n",
            format);
	return 0;
}

int load_file(struct texture_info *info, const char *path)
{
	int file_fd;
    void *addr_file;

	file_fd = open(path, O_RDWR);
	if (file_fd < 0) {
		printf("fopen %s err.\n", path);
		return -1;
	}

    addr_file = (void *)mmap((void *)0, info->size, PROT_READ | PROT_WRITE,
				 MAP_SHARED, file_fd, 0);

	//copy texture into dma buffer
    memcpy(info->virt, addr_file, info->size);
	printf("memcpy texture into dam buffer successfuly!\n");

	munmap(addr_file, info->size);
	close(file_fd);

	return 0;
}

int dma_buf_open(void)
{
	int fd;

#ifdef USE_IOMMU
	fd = open("/dev/dma_heap/system", O_RDWR, 0);
#else
	fd = open("/dev/dma_heap/reserved", O_RDWR, 0);
#endif

	if (fd <= 0) {
		printf("open dma_buf failed!\n");
		return -1;
	}

	printf("open dma_buf successfullly!\n");
	return fd;
}

void dma_buf_close(int ion_fd)
{
	if (ion_fd > 0)
		close(ion_fd);
}

int dma_alloc(int ion_fd, struct texture_info *info, size_t size)
{
	int ret;
	void *addrmap = NULL;

	//alloc dma buffer,and reture an dma handle
	struct dma_heap_allocation_data heap_data = {0x00};
    heap_data.len = size;
    heap_data.fd_flags = O_RDWR | O_CLOEXEC;

	ret = ioctl(ion_fd, DMA_HEAP_IOCTL_ALLOC, &heap_data);
	if (ret < 0) {
		printf("alloc dma_buf failed,err is %d.\n", ret);
		close(ion_fd);
		return -1;
	}

	/* mmap to user */
	addrmap = mmap(NULL, heap_data.len, PROT_READ | PROT_WRITE, MAP_SHARED,
			heap_data.fd, 0);
	if (MAP_FAILED == addrmap) {
		printf("mmap err!\n");
		addrmap = NULL;
		return -1;
	}

	memset(addrmap, 0, heap_data.len);
	info->phy = 0;
	info->virt = (unsigned char *)addrmap;
	info->size = size;
	info->dmafd = heap_data.fd;

	return info->dmafd;
}

void dma_buf_free(int ion_fd, struct texture_info *info)
{
	if (info->virt) {
		munmap((void*)info->virt, info->size);
		info->virt = 0;
	}

	if (info->dmafd > 0) {
		close(info->dmafd);
		info->dmafd = -1;
	}

}

int output_render_result(struct texture_info *info, char *path)
{
    size_t size = info->size;
	FILE *out_fd = fopen(path, "wb");
	info->virt = (unsigned char *)mmap((void *)0, size, PROT_WRITE, MAP_SHARED, info->dmafd, 0);

	if (out_fd) {
		size_t count = fwrite(info->virt, 1, size, out_fd);
		if (count == size) printf("read output texture succeed!\n");
		else {
			printf("read output texture size is %lu, the origin size is %lu\n", count, size);
		}
	}
    sync();
	fclose(out_fd);
	return 0;
}
