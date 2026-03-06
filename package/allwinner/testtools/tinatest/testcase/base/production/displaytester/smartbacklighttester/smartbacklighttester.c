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
#include "../common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../common/sunxi_display_v2.h"
#else
#include "../common/sunxi_display_v1.h"
#endif

struct test_smart_backlight_info {
	int screen_id;
	luapi_disp_window window;
};

luapi_disp_window window;

struct test_smart_backlight_info smbl_info;

void parse_configs(void)
{
	int sid;

	sid = mjson_fetch_int(
			"/base/production/displaytester/smartbacklighttester/screen_id");
	if (sid != -1) {
		smbl_info.screen_id = sid;
		printf("id of the screen is %d \n", sid);
	} else {
		smbl_info.screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}
}

int main(int argc, int *argv[])
{
	int disphd;
	int ret;
	int width;
	int height;
	int setwin;
	int enable;
	int disable;

	memset(&smbl_info, 0, sizeof(struct test_smart_backlight_info));
	disphd = open("/dev/disp", O_RDWR);
	if (disphd == -1) {
		printf("open display device faild ( %s )\n", strerror(errno));
		return -1;
	}

	parse_configs();

	smbl_info.window.x = 0;
	smbl_info.window.y = 0;
	width = DispGetScrWidth(disphd, smbl_info.screen_id);
	if (width < 0) {
		width = 300;
		printf("DispGetScrWidth FAILED\n");
	}
	height = DispGetScrHeight(disphd, smbl_info.screen_id);
	if (height < 0) {
		height = 300;
		printf("DispGetScrHeight FAILED\n");
	}
	smbl_info.window.width = width / 2;
	smbl_info.window.height = height;
	printf("the width of screen is %d\n", width);
	printf("the height of screen is %d\n", height);

	window.x = 0;
	window.y = 0;
	window.width = width;
	window.height = height;

	setwin = DispSetSMBLWindow(disphd, smbl_info.screen_id, window);

	if (setwin < 0) {
		printf("DispSetSMBLWindow FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLWindow SUCCESS\n");
	}

	enable = DispSetSMBLEnable(disphd, smbl_info.screen_id, 1);
	if (enable < 0) {
		printf("DispSetSMBLEnable enable FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLEnable enable SUCCESS\n");
	}

	sleep(3);

	disable = DispSetSMBLEnable(disphd, smbl_info.screen_id, 0);
	if (disable < 0) {
		printf("DispSetSMBLEnable disable FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLEnable disable SUCCESS\n");
	}

	close(disphd);
	printf("GOOD, smart back light test success.\n");
	return 0;

	err: close(disphd);
	printf("GOOD, smart back light test failed.\n");
	return -1;
}
