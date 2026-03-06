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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "mjson.h"
#include "sysinfo.h"
#include "interact.h"

#include "../common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../common/sunxi_display_v2.h"
#else
#include "../common/sunxi_display_v1.h"
#endif

#define BRIGHTNESS_MIN      20
#define BRIGHTNESS_MAX      220

unsigned int brightness;
int tmp = 20;

struct test_lcd_brightness_info {
	int screen_id;
	int brightness;
};

struct test_lcd_brightness_info test_info;

void parse_configs(void)
{
	int sid;
	int brightness;

	sid = mjson_fetch_int(
			"/base/production/displaytester/brightnesstester/screen_id");
	if (sid != -1) {
		test_info.screen_id = sid;
		printf("id of the screen is %d \n", sid);
	} else {
		test_info.screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}

	brightness = mjson_fetch_int(
			"/base/production/displaytester/brightnesstester/brightness");
	if (brightness != -1) {
		test_info.brightness = brightness;
		printf("brightness of the screen is %d\n", brightness);
	} else {
		test_info.brightness = 100;
		printf(
				"get the brightness of the screen failed, use default brightness 100\n");
	}
}

int main(int argc, int *argv[])
{
	int disphd;
	int getBright;
	unsigned int i;

	memset(&test_info, 0, sizeof(struct test_lcd_brightness_info));
	disphd = open("/dev/disp", O_RDWR);
	if (disphd == -1) {
		printf("open display device faild ( %s )\n", strerror(errno));
		goto err;
	}

	parse_configs();
	brightness = test_info.brightness;

	for (i = 0; i < 25; i++) {
		brightness += tmp;
		if (brightness > BRIGHTNESS_MAX) {
			brightness = BRIGHTNESS_MAX;
			tmp = -20;
		} else if (brightness < BRIGHTNESS_MIN) {
			brightness = BRIGHTNESS_MIN;
			tmp = 20;
		}

		if (i == 24)
			brightness = test_info.brightness;

		if (DispSetBrightness(disphd, test_info.screen_id, brightness) < 0)
			goto err;
		usleep(200000);

		getBright = DispGetBrightness(disphd, test_info.screen_id);
		if (getBright < 0)
			goto err;
		printf("current brightness is %d\n", getBright);
	}

	close(disphd);
	int ret = ttrue("Can you see the screen brightness change?");
	if (ret == 1) {
		printf("GOOD, brightness test success.\n");
		return 0;
	} else {
		printf("ERROR, brightness test failed.\n");
		return -1;
	}

	err: close(disphd);
	printf("ERROR, brightness test failed.\n");
	return -1;
}
