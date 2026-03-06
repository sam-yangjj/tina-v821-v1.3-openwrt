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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

int watchdog_init(int timeout)
{
    int ret = 0;
    int option = 0;
    int wdt_fd = -1;

    printf("[%s %d] Init watchdog, timeout:%ds\n", __func__, __LINE__, timeout);

    /* Open watchdog */
    wdt_fd = open("/dev/watchdog", O_WRONLY);
    if (wdt_fd == -1) {
        printf("[%s %d] Open /dev/watchdog failed\n", __func__, __LINE__);
        return -1;
    }

    /* After opening, the watchdog timer will start, stop it first */
    option = WDIOS_DISABLECARD;
    ret = ioctl(wdt_fd, WDIOC_SETOPTIONS, &option);
    if (ret < 0) {
        printf("[%s %d] Ioctl error, return %d\n", __func__, __LINE__, ret);
        return -1;
    }

    /* Set watchdog timeout */
    ret = ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);
    if (ret < 0) {
        printf("[%s %d] Ioctl error, return %d\n", __func__, __LINE__, ret);
        return -1;
    }

    /* Enable the watchdog timer */
    option = WDIOS_ENABLECARD;
    ret = ioctl(wdt_fd, WDIOC_SETOPTIONS, &option);
    if (ret < 0) {
        printf("[%s %d] Ioctl error, return %d\n", __func__, __LINE__, ret);
        return -1;
    }

    return wdt_fd;
}

int watchdog_deinit(int wdt_fd)
{
    int ret = 0;

    if (wdt_fd <= 0) {
        printf("[%s %d] Watchdog hasn't been inited\n", __func__, __LINE__);
        return -1;
    }

    int option = WDIOS_DISABLECARD;
    ret = ioctl(wdt_fd, WDIOC_SETOPTIONS, &option);
    if (ret < 0) {
        printf("[%s %d] Ioctl error, return %d\n", __func__, __LINE__, ret);
        return -1;
    }

    if (wdt_fd != -1) {
        close(wdt_fd);
        wdt_fd = -1;
    }

    return 0;
}

int watchdog_keepalive(int wdt_fd)
{
    int ret = 0;

    if (wdt_fd < 0) {
        printf("[%s %d] Watchdog hasn't been inited\n",__func__, __LINE__);
        return -1;
    }

    ret = ioctl(wdt_fd, WDIOC_KEEPALIVE, 0);
    if (ret < 0) {
        printf("[%s %d] Ioctl error, return %d\n", __func__, __LINE__, ret);
        return -1;
    }

    return 0;
}

void show_usage(void)
{
    printf("===================================================================\n");
    printf("Usage: watchdog_test <feed_conut> <feed_interval> <watchdog_timeout>\n");
    printf(" --- feed_conut: mumber of times feed dog\n");
    printf(" --- feed_interval: feed dog interval\n");
    printf(" --- watchdog_timeout: unit is second, between 0 ~ 16\n");
    printf("     example: watchdog_test 10 1 5\n");
}

int main(int argc, char *argv[])
{
    int loop = 0;
    int feed_conut = 0;
    int feed_interval = 0;
    int timeout = 0;
    int wdt_fd = -1;

    if (argc != 4) {
        show_usage();
        return 0;
    }

    feed_conut = atoi(argv[1]);
    feed_interval = atoi(argv[2]);
    timeout = atoi(argv[3]);

    printf("[watchdog_test] feed_conut:%d, feed_interval:%d, timeout%d\n", feed_conut, feed_interval, timeout);

    if (timeout > 16) {
        printf("[watchdog_test:%d] error, watchdog timeout is too large\n", __LINE__);
        show_usage();
        return -1;
    }

    wdt_fd = watchdog_init(timeout);
    if (wdt_fd < 0) {
        printf("[watchdog_test:%d] Watchdog init fail\n", __LINE__);
        return -1;
    }

    while(loop < feed_conut) {
        sleep(feed_interval);
        printf("[watchdog_test:%d] Feeding the dog, feed_conut:%d\n", __LINE__, loop);
        watchdog_keepalive(wdt_fd);
        loop++;
    }

    printf("[watchdog_test:%d] Stop to feed dog, system will reboot after %d second\n", __LINE__, timeout);

    return 0;
}