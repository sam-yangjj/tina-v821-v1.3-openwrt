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
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/errno.h>

#ifndef __APPLE__
#include <linux/limits.h>
#else
#define PATH_MAX 1024
#endif

#include "debug.h"
#include "utils.h"

#define DEFAULT_TEMPERATURE 100

static char thermal_path_[PATH_MAX] = {0};

int thermal_init(const char* path)
{
    int temp = readIntFromFile(path);
    if (temp == -1) {
        INFO("read temperature from '%s' error!", path);
        return -1;
    }

    sprintf(thermal_path_, "%s", path);
    INFO("using thermal device: %s, current temperature [%f]", thermal_path_, temp/1000.0f);
    return 0;
}

uint32_t thermal_temperature_get()
{
    int temp = readIntFromFile(thermal_path_);
    if (temp == -1) {
        INFO("read temperature from '%s' error!", thermal_path_);
        return DEFAULT_TEMPERATURE;
    }

    return floor(temp/1000.0f);
}

/* default duty_cycle is 1us (1000 ns) */
#define DEFAULT_DUTY_CYCLE 1000
#define PWM_CHIP        "/sys/class/pwm/pwmchip0"
#define PWM_CHIP_EXPORT "/sys/class/pwm/pwmchip0/export"

static int path_accessable(const char* path) {
    if (access(path, R_OK) == 0)
        return 1;
    INFO("path '%s' cannot access, %s", path, strerror(errno));
    return 0;
}

static int pwm_id_ = 0;
static char pwm_path_[PATH_MAX] = {0};

int pwm_init(int id, int period /* nanoseconds */)
{
    char pathstr[PATH_MAX + 32] = {0};

    if (!path_accessable(PWM_CHIP_EXPORT)) {
        return -1;
    }

    sprintf(pwm_path_, "%s/pwm%d", PWM_CHIP, id);

    if (!path_accessable(pwm_path_)) {
        INFO("pwm channel [%d] not export yet, try to export...\n", id);
        if (writeIntToFile(PWM_CHIP_EXPORT, id) != 0) {
            INFO("failed to export pwm channel %d!", id);
            return -1;
        }
    }

    sprintf(pathstr, "%s/enable", pwm_path_);
    if (writeIntToFile(pathstr, 0) != 0) {
        INFO("failed to enable pwm(%s)!", pathstr);
        return -1;
    }

    sprintf(pathstr, "%s/period", pwm_path_);
    if (writeIntToFile(pathstr, period) != 0) {
        INFO("failed to config pwm(%s) period to %d ns!", pathstr, period);
        return -1;
    }

    sprintf(pathstr, "%s/duty_cycle", pwm_path_);
    if (writeIntToFile(pathstr, DEFAULT_DUTY_CYCLE) != 0) {
        INFO("failed to config pwm(%s) duty_cycle to %d ns!", pathstr, DEFAULT_DUTY_CYCLE);
        return -1;
    }

    sprintf(pathstr, "%s/enable", pwm_path_);
    if (writeIntToFile(pathstr, 1) != 0) {
        INFO("failed to enable pwm(%s)!", pathstr);
        return -1;
    }

    return 0;
}

int pwm_config_duty(int duty /* nanoseconds */)
{
    char pathstr[PATH_MAX + 32] = {0};

    sprintf(pathstr, "%s/duty_cycle", pwm_path_);
    if (writeIntToFile(pathstr, duty) != 0) {
        INFO("failed to config pwm(%s) duty_cycle to %d ns!", pathstr, DEFAULT_DUTY_CYCLE);
        return -1;
    }
    return 0;
}

