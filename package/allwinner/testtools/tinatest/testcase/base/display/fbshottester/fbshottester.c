/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <string.h>
#include "mjson.h"
#include "sysinfo.h"
#include "interact.h"

typedef unsigned char BYTE;
static int fb_id;
static int fb_num;

typedef struct tagBITMAPFILEHEADER {
	u_int16_t bfType;
	u_int32_t bfSize;
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	u_int32_t biSize;
	u_int32_t biWidth;
	u_int32_t biHeight;
	u_int16_t biPlanes;
	u_int16_t biBitCount;
	u_int32_t biCompression;
	u_int32_t biSizeImage;
	u_int32_t biXPelsPerMeter;
	u_int32_t biYPelsPerMeter;
	u_int32_t biClrUsed;
	u_int32_t biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
}__attribute__((packed)) RGBQUAD;

void parse_configs(void)
{
	fb_id = mjson_fetch_int(
			"/base/display/fbshottester/fb_id");
	if (fb_id != -1) {
		printf("ID of the fb id is %d \n", fb_id);
	} else {
		fb_id = 0;
		printf("get the fb id failed, use the default id 0\n");
	}

	fb_num = mjson_fetch_int(
			"/base/display/fbshottester/fb_num");
	if (fb_num != -1) {
		printf("ID of the fb num is %d \n", fb_num);
	} else {
		fb_num = 1;
		printf("get the fb num failed, use the default id 0\n");
	}
}

int main(int argc, char *argv[])
{
	int i;
	int img_fd, fb_fd;
	int data_size;
	char *img_buf;
	struct fb_var_screeninfo var_info;
	BITMAPFILEHEADER file_head;
	BITMAPINFOHEADER info_head;
	//RGBQUAD rgb_quad;

	parse_configs();

	/*open files*/
	char fbPatch[32] = "/dev/fb", fbNum[5];
	sprintf(fbNum, "%d", fb_id);
	strcat(fbPatch, fbNum);
	fb_fd = open(fbPatch, O_RDWR);
	if (fb_fd < 0) {
		perror("open framebuff");
		return -1;
	}
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info) < 0) {
		perror("ioctl FBIOGET_VSCREENINFO");
		close(img_fd);
		return 0;
	}
	printf("xres %d, yres %d, fbnum %d\n", var_info.xres, var_info.yres, fb_num);

	img_fd = open("/tmp/fbshottester.bmp", O_RDWR | O_CREAT, 0644);
	if (img_fd < 0) {
		perror("open image");
		close(img_fd);
		return -1;
	}

	data_size = var_info.xres * var_info.yres * (var_info.bits_per_pixel / 8) * fb_num;
	/*initialize bmp structs*/
	file_head.bfType = 0x4d42;
	file_head.bfSize = sizeof(file_head) + sizeof(info_head) + data_size;
	file_head.bfReserved1 = file_head.bfReserved2 = 0;
	file_head.bfOffBits = sizeof(file_head) + sizeof(info_head);

	info_head.biSize = sizeof(info_head);
	info_head.biWidth = var_info.xres;
	info_head.biHeight = var_info.yres * fb_num;
	info_head.biPlanes = 1;
	info_head.biBitCount = var_info.bits_per_pixel;
	info_head.biCompression = 0;
	info_head.biSizeImage = 0;
	info_head.biXPelsPerMeter = 0;
	info_head.biYPelsPerMeter = 0;
	info_head.biClrUsed = 0;
	info_head.biClrImportant = 0;

	img_buf = (char *) malloc(data_size);
	if (img_buf == NULL) {
		printf("malloc failed!\n");
		close(fb_fd);
		close(img_fd);
		return -1;
	}

	/*read img data and */
	read(fb_fd, img_buf, data_size);

	write(img_fd, &file_head, sizeof(file_head));
	write(img_fd, &info_head, sizeof(info_head));

	/*revese img and write to file*/
	for (i = 0; i < var_info.yres * fb_num; i++) {
		write(img_fd,
				img_buf
						+ var_info.xres * (var_info.yres * fb_num - i - 1)
								* (var_info.bits_per_pixel / 8),
				var_info.xres * (var_info.bits_per_pixel / 8));
	}

	close(fb_fd);
	close(img_fd);
	if(img_buf != NULL) {
		free(img_buf);
		img_buf = NULL;
	}
	printf("GOOD, The image is saved in /tmp/fbshottester.bmp\n");

	int ret = tshowimg("/usr/share/fbshottester.bmp",
			"Could you see this picture clearly?");
	if (ret == 0) {
		printf("GOOD, picture show success.\n");
		return 0;
	} else {
		printf("ERROR, Failed to upload the image to the computer. You can use the adb pull command to transfer the image to your computer.\n");
		return -1;
	}
}
