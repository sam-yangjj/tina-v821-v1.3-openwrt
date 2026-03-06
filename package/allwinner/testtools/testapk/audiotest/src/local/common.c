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
#include "common.h"
#include <stdio.h>
#include <stdarg.h>

uint16_t little_endian_u8_to_u16(const uint8_t *buf)
{
    uint16_t value = 0;
    int i;
    for (i = 0; i < 2; ++i)
        value += (uint16_t)(*(buf + i)) << (8 * i);
    return value;
}

uint32_t little_endian_u8_to_u32(const uint8_t *buf)
{
    uint32_t value = 0;
    int i;
    for (i = 0; i < 4; ++i)
        value += (uint32_t)(*(buf + i)) << (8 * i);
    return value;
}

void u16_to_little_endian_u8(uint16_t value, uint8_t *buf)
{
    int i;
    for (i = 0; i < 2; ++i)
        buf[i] = (uint8_t)(value >> (8 * i) & 0xff);
}

void u32_to_little_endian_u8(uint32_t value, uint8_t *buf)
{
    int i;
    for (i = 0; i < 4; ++i)
        buf[i] = (uint8_t)(value >> (8 * i) & 0xff);
}

void audiotest_stderr(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "[ERROR] [audiotest] ");
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void audiotest_stdout(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stdout, "[audiotest] ");
    vfprintf(stdout, fmt, va);
    va_end(va);
}
