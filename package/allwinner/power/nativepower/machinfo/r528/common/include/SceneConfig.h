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
#ifndef __SUN8IW20P1_H__
#define __SUN8IW20P1_H__
/* cpu spec files defined */
#define CPU_NUM_MAX     2
#define CPU0LOCK        "/dev/null"
#define ROOMAGE         "/dev/null"
#define CPUFREQ_AVAIL   "/sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies"
#define CPUFREQ         "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"
#define CPUFREQ_MAX     "/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq"
#define CPUFREQ_MIN     "/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq"
#define CPUONLINE       "/sys/devices/system/cpu/online"
#define CPUHOT          "/dev/null"
#define CPU0GOV         "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
/* gpu spec files defined */
#define GPUFREQ         "/dev/null"
#define GPUCOMMAND      "/dev/null"
/* ddr spec files defined */
#define DRAMFREQ_AVAIL  "/dev/null"
#define DRAMFREQ        "/dev/null"
#define DRAMFREQ_MAX    "/dev/null"
#define DRAMFREQ_MIN    "/dev/null"
#define DRAMMODE        "/dev/null"
/* task spec files defined */
#define TASKS           "/dev/null"
/* touch screen runtime suspend */
#define TP_SUSPEND      "/dev/null"

#endif
