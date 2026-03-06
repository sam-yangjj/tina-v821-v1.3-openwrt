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
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>

#include "debug.h"
#include "platform.h"
#include "pid.h"

static const struct option long_opts[] = {
    {"thermal",     required_argument, NULL, 't'},
    {"pwm",         required_argument, NULL, 'p'},
    {"temperature", required_argument, NULL, 'e'},
    {"help",        no_argument,       NULL, 'h'},
    {"verbose",     no_argument,       NULL, 'v'},
    {0, 0, 0}
};

static const char* thermal_ = "/sys/class/thermal/thermal_zone0/temp";
static int pwm_channel_ = 0;
static int target_temp_ = 80;
static int verbose_ = 0;

static void usage(const char* program)
{
    printf("thermal-pwm V0.1 BY yajianz@allwinnertech.com\n\n");
    printf("Usage:\n");
    printf("  %s [-t thermal] [-p pwm] [-e temp] [-v] [-h]\n", program);
    printf("       -t,--thermal      thermal device path, default: /sys/class/thermal/thermal_zone0/temp\n"
           "       -p,--pwm          pwm output channel, default pwm0\n"
           "       -e,--temperature  target temperature, default 80 C\n"
           "       -v,--verbose      verbose mode\n"
           "       -h,--help         show this usage message\n");
}

static void parse_opt(int argc, char** argv)
{
    int  long_index,c,ret;
    while ((c = getopt_long(argc, argv, "t:p:e:hv", long_opts, &long_index)) != EOF) {
        switch (c) {
            case 't': thermal_ = optarg; break;
            case 'p': pwm_channel_ = strtoul(optarg, 0, 0); break;
            case 'e': target_temp_ = strtoul(optarg, 0, 0); break;
            case 'v': verbose_ = 1; break;
            case 'h':
            default : usage(argv[0]); exit(1); break;
        }
    }
}

#define PWM_PERIOD_NS  (128 * 1000)
#define PWM_SAFETY_GAP (500)

int cycle_to_duty(int cycle)
{
    if (cycle > 3072)
        cycle = 3072;
    else if(cycle < 0)
        cycle = 0;

    /* nanoseconds */
    const float plus = 41.67f;
    float active_ns = cycle * plus;

    int duty = floor(active_ns);

    if (duty > (PWM_PERIOD_NS - PWM_SAFETY_GAP))
        duty = PWM_PERIOD_NS - PWM_SAFETY_GAP;

    return duty;
}

int main(int argc, char** argv)
{
    parse_opt(argc, argv);

    if (thermal_init(thermal_) != 0) {
        INFO("thermal '%s' init error!", thermal_);
        return -1;
    }

    if (pwm_init(pwm_channel_, PWM_PERIOD_NS /* nanoseconds */) != 0) {
        INFO("pwm channel [%d] init error!", pwm_channel_);
        return -1;
    }

    INFO("thermal-pwm start: pwm channel [%d], target temperature: %d C", pwm_channel_, target_temp_);

    InitPID();

    while (1) {
        int temperature = thermal_temperature_get();
        int duty  = 0;
        int cycle = 0;
        if (temperature > target_temp_) {
            pid.SetTemperature    = target_temp_;
            pid.ActualTemperature = temperature;

            cycle = LocPIDCalc();
            duty  = cycle_to_duty(cycle);
        }
        else
            duty = 0;

        pwm_config_duty(duty);
        INFO("current temperature [%d C] cycle=%d duty=%d ns", temperature, cycle, duty);

        usleep(1000 * 1000);
    }

    return 0;
}
