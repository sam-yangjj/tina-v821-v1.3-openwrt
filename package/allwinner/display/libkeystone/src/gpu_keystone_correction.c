/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Hello triangle, adapted for native display on libMali.so.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/fcntl.h> // include O_RDWR
#include <sys/ioctl.h> // include ioctl()
#include <sys/mman.h> // include mmap()
#include <string.h> // include memcpy()

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>

#include "demo_common.h"
#include "renderengine_uapi.h"
#define WIDTH 224
#define HEIGHT 240

#define TEXTURE_WIDTH 224
#define TEXTURE_HEIGHT 240

/* This array is the offset of the four vertices of a quadrilateral.
 * The order of vertices must follow.
 * keystone_vertex[0] = left bottoim corner x-coordinate offset;
 * keystone_vertex[1] = left bottom corner y-coordinate offset;
 * keystone_vertex[2] = left top corner x-coordinate offset;
 *
*/
static GLfloat keystone_vertex[] = {
	0.0f, 0.4583f,  //left bottom
	0.0f, 0.25f,    //left top
	0.0f, 0.133f,   //right top
	0.0f, 0.05f,    //right bottom
};

int main(int argc, char *argv[])
{
    bool countine_test = false;
	int ret = 0;
    int dma_fd = 0;
	struct timeval tv;
	uint32_t last_time = 0;
	uint32_t current_time = 0;
	uint32_t inputTexFormat = DRM_FORMAT_NV21;
	uint32_t outputTexFormat = DRM_FORMAT_NV21;

    TextureInfo input_texture = {0x00};
    TextureInfo output_texture = {0x00};

	dma_fd = dma_buf_open();
	if (dma_fd < 0)
		exit(1);

    //set input_texture info
    input_texture.dmafd = -1;
    input_texture.width = TEXTURE_WIDTH;
    input_texture.height = TEXTURE_HEIGHT;
    input_texture.format = inputTexFormat;
    input_texture.size = getPictureSize(TEXTURE_WIDTH, TEXTURE_HEIGHT, inputTexFormat);
    dma_alloc(dma_fd, &input_texture, input_texture.size);
    //load picture to input_texture's dma_buf
    ret = load_file(&input_texture, "/usr/share/res/224x240.nv21");
	if (ret < 0) {
        printf("load input_texture failed!\n");
		exit(1);
    }
    
    //可以使用这个接口，dump输入输出的纹理
	//output_render_result(&input_texture,"/mnt/input_texture.bin");
    //set output_texture info
    output_texture.dmafd = -1;
    output_texture.width = WIDTH;
    output_texture.height = HEIGHT;
    output_texture.format = outputTexFormat;
    output_texture.size = getPictureSize(WIDTH, HEIGHT, outputTexFormat);
    ret = dma_alloc(dma_fd, &output_texture, output_texture.size);
	if (ret < 0) {
        printf("alloc fbo dma_buf failed!\n");
		exit(1);
    }

    // create render engine
    RenderEngine_t *render = renderEngineCreate(0, 0);
    if (!render) {
        printf("create render engine failed!\n");
        exit(1);
    }

    render->render_init(render->core);

	while (1) {
        render->set_textrue(render->core, &input_texture, &output_texture,
                keystone_vertex);

		gettimeofday(&tv,NULL);
		last_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        render->draw(render->core, 1);

		gettimeofday(&tv,NULL);
		current_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		printf("render time is %f ms,  %f fps\n", (float) ((current_time - last_time)),
						(float) 1000 / (current_time - last_time));

		//output_render_result(&output_texture,"/mnt/render_result.yuv");

        //After each rendering, delete the entered texture, otherwise memory leak will occur
		render->delete_input_texture(render->core);
        render->delete_output_texture(render->core);

		if (!countine_test) {
			char c = getchar();
			if (c == 'b')
				break;
			else if (c == 'n')
				countine_test = true;
		}

	}
	output_render_result(&output_texture,"/mnt/render_result.yuv");

    render->render_deinit(render->core);

	dma_buf_free(input_texture.dmafd, &input_texture);
	dma_buf_free(output_texture.dmafd, &output_texture);
	close(dma_fd);

	printf("Render exit!\n");
	return 0;
}
