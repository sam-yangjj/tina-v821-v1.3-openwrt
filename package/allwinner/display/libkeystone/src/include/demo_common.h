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
#ifndef _DEMO_COMMON_H_
#define _DEMO_COMMON_H_

#include <drm_fourcc.h>
#include <dma-heap.h>
#include "renderengine_uapi.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned int getDrmPixelFormat(char *name);
unsigned int getPictureSize(unsigned int W, unsigned int H,
            unsigned int format);

int load_file(struct texture_info *info, const char *path);

int dma_buf_open(void);
void dma_buf_close(int ion_fd);

int dma_alloc(int ion_fd, struct texture_info *info, size_t size);
void dma_buf_free(int ion_fd, struct texture_info *info);
int output_render_result(struct texture_info *info, char *path);

#ifdef __cplusplus
}
#endif
#endif
