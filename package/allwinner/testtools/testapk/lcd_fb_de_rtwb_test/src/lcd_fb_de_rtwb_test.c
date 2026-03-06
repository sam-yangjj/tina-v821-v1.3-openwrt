/**
 * @file lcd_fb_de_rtwb_test.c
 * @brief LCD Framebuffer and RTWB Display Interface Test Program
 *
 * This program tests the LCD framebuffer and RTWB (Real-Time Write-Back)
 * display interface functionality on Allwinner platforms. It demonstrates
 * how to configure the display device, retrieve DMA buffer file descriptors,
 * set up capture parameters, and perform display operations.
 *
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the people's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
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

#include <stdio.h>         // Standard input/output functions
#include <stdlib.h>        // Standard library functions
#include <unistd.h>        // POSIX operating system API
#include <sys/mman.h>      // Memory mapping functions
#include <sys/ioctl.h>     // IO control operations
#include <linux/fb.h>      // Framebuffer device definitions
#include <string.h>        // String handling functions
#include <endian.h>        // Endianness conversion utilities
#include <errno.h>         // Error number definitions
#include <fcntl.h>         // File control options
#include <getopt.h>        // Command line argument parsing
#include <pthread.h>       // POSIX threads
#include <signal.h>        // Signal handling
#include <stdint.h>        // Fixed-size integer types
#include <stdbool.h>       // Boolean data type
#include <math.h>          // Mathematical functions
#include <time.h>          // Time handling
#include <strings.h>       // String operations
#include <sys/stat.h>      // File status functions
#include <sys/types.h>     // Data types
#include <linux/kernel.h>  // Kernel-specific definitions

#include <video/uapi_lcd_fb.h>

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#include <video/sunxi_display2.h>

/** @def WIDTH
 *  @brief Default display width in pixels
 */
#define WIDTH 240

/** @def HEIGHT
 *  @brief Default display height in pixels
 */
#define HEIGHT 320

/**
 * @brief Main function for LCD framebuffer and RTWB display test
 *
 * This function demonstrates how to configure and test the LCD framebuffer
 * and RTWB (Real-Time Write-Back) display interface. It opens the framebuffer
 * and display devices, configures display settings, retrieves DMA buffer file
 * descriptors, sets up capture parameters, and performs display operations.
 *
 * @return 0 on success, negative value on error
 */
int main()
{
	int fb_fd, disp_fd;          // File descriptors for framebuffer and display devices
	int ret;                     // Return value for error checking
	struct fb_var_screeninfo vinfo;           // Framebuffer variable screen info
	struct fb_fix_screeninfo finfo;           // Framebuffer fixed screen info
	struct lcd_fb_dmabuf_export dmabuf_info;  // DMA buffer export information
	struct disp_device_config conf;           // Display device configuration

	// Initialize display configuration structure
	memset(&conf, 0, sizeof(conf));
	conf.type = DISP_OUTPUT_TYPE_RTWB;        // Set output type to RTWB
	conf.mode = DISP_TV_MOD_480P;             // Set TV mode to 480P
	conf.timing.x_res = WIDTH;                // Set horizontal resolution
	conf.timing.y_res = HEIGHT;               // Set vertical resolution
	conf.timing.frame_period = 16666667;      // Frame period (60Hz)
	conf.format = DISP_CSC_TYPE_RGB;          // Set color space conversion type to RGB
	conf.cs = DISP_BT709;                     // Set color standard to BT.709
	conf.bits = DISP_DATA_8BITS;              // Set data bits to 8 bits
	conf.eotf = DISP_EOTF_GAMMA22;            // Set EOTF to Gamma 2.2
	conf.range = DISP_COLOR_RANGE_16_235;     // Set color range to 16-235
	conf.dvi_hdmi = DISP_HDMI;                // Set interface type to HDMI
	conf.scan = DISP_SCANINFO_NO_DATA;        // Set scan information

	// Open the framebuffer device (assuming it's located at /dev/fb0)
	fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd == -1) {
		perror("Failed to open framebuffer device");
		return -1;
	}

	// Open the display device
	disp_fd = open("/dev/disp", O_RDWR);
	if (disp_fd == -1) {
		perror("Failed to open disp device");
		return -1;
	}

	// Set display device configuration
	unsigned long arg[4] = { 0 };
	arg[0] = 0;                             // Display channel 0
	arg[1] = (unsigned long)&conf;          // Configuration structure
	ioctl(disp_fd, DISP_DEVICE_SET_CONFIG, (void *)arg);

	// Call the ioctl to get the DMA buffer file descriptor
	ret = ioctl(fb_fd, LCDFB_IO_GET_DMABUF_FD, &dmabuf_info);
	if (ret == -1) {
		perror("ioctl failed");
		goto err;  // Jump to error handling if ioctl fails
	}

	// Get framebuffer variable screen information
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
		perror("Error reading variable information");
		goto err;
	}

	// Output the DMA buffer file descriptor for debugging
	printf("fb_fd: %d, DMA buffer FD: %d, disp_fd: %d\n", fb_fd, dmabuf_info.fd, disp_fd);

	// Initialize display capture
	struct disp_capture_init_info capture_info;
	capture_info.port = DISP_CAPTURE_AFTER_DEP;  // Capture after display engine processing

	memset(arg, 0x0, sizeof(arg));
	arg[0] = 0;  // Display channel 0
	arg[1] = (unsigned long)&capture_info;
	ioctl(disp_fd, DISP_CAPTURE_START, (void *)arg);

	// Configure capture parameters
	struct disp_capture_info2 info2 = { 0 };
	info2.window.x = 0;                       // Capture window X position
	info2.window.y = 0;                       // Capture window Y position
	info2.window.width = WIDTH;               // Capture window width
	info2.window.height = HEIGHT;             // Capture window height
	info2.out_frame.format = DISP_FORMAT_ARGB_8888;  // Output frame format
	info2.out_frame.size[0].width = WIDTH;    // Output frame width
	info2.out_frame.size[0].height = HEIGHT;  // Output frame height
	info2.out_frame.crop.x = 0;               // Crop X position
	info2.out_frame.crop.y = 0;               // Crop Y position
	info2.out_frame.crop.width = WIDTH;       // Crop width
	info2.out_frame.crop.height = HEIGHT;     // Crop height
	info2.out_frame.fd = dmabuf_info.fd;      // DMA buffer file descriptor

	// Set RTWB sync file descriptor
	memset(arg, 0x0, sizeof(arg));
	arg[0] = 0;  // Display channel 0
	arg[1] = (unsigned long)&info2;
	ret = ioctl(disp_fd, DISP_RTWB_SYNC_FD, (void *)(arg));
	if (ret != 0) {
		printf("capture commit err %d\n", ret);
	}
	// Set framebuffer pan display parameters
	vinfo.xoffset = 0;
	vinfo.yoffset = 0;
	if (ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo) < 0) {
		perror("Error: FBIOPAN_DISPLAY fail");
	}

err:
	// Close the framebuffer device
	close(fb_fd);
	// Close the display device
	close(disp_fd);

	return 0;
}
