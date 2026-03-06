/*
 * This confidential and proprietary software should be used
 * under the licensing agreement from Allwinner Technology.

 * Copyright (C) 2020-2023 Allwinner Technology Limited
 * All rights reserved.

 * Author:zhengwanyu <liuyuhang@allwinnertech.com>

 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from Allwinner Technology Limited.
 */
#ifndef _RENDERENGINE_UAPI_H_
#define _RENDERENGINE_UAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct texture_info
{
	unsigned int phy;     /* physical address */
	unsigned char *virt;  /* virtual address */
	unsigned long size;   /* ion buffer size */
    unsigned int width;   /* texture width (in pixel) */
    unsigned int height;  /* texture hight (in pixel) */
    unsigned int format;  /* texture format */
	int dmafd;            /* ion dmabuf fd */
}TextureInfo;


typedef struct {
    /* renderengine core  */
    void *core;

    /* init a render engine  */
    int (*render_init)(void *render_engine);

	/* set up input texture and output target image */
	int (*set_textrue)(void *render_engine, TextureInfo *input_texture,
            TextureInfo *output_texture, float *keystone_vertex);
    /* delet input/output texture  */
    int (*delete_input_texture)(void *render_engine);
    int (*delete_output_texture)(void *render_engine);

	/* draw the surface */
	int (*draw)(void *render_engine, bool is_block);

    int (*render_deinit)(void *render_engine);
} RenderEngine_t;

/*
 * create a renderengine(e.g. init EGL, init shader ...)
 */
RenderEngine_t *renderEngineCreate(bool onScreen, unsigned char debug);

/*destroy an renderengine*/
void renderEngineDestroy(RenderEngine_t *render_engine);

#ifdef __cplusplus
}
#endif

#endif
