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
#include "aactd/common.h"

#include <string.h>
#include <errno.h>
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#define IS_BIG_ENDIAN
#else
#error "Unknown byte order"
#endif

void aactd_print_original_buf(const uint8_t *buf, int buf_len)
{
    int i, quotient, remain;
    const uint8_t *p = buf;

    printf("\n");
    printf("Original data (%d bytes):\n", buf_len);
    quotient = buf_len / 4;
    remain = buf_len % 4;
    for (i = 0; i < quotient; ++i) {
        printf("  0x%02x  0x%02x  0x%02x  0x%02x\n",
                p[i*4], p[i*4+1], p[i*4+2], p[i*4+3]);
    }
    p = buf + 4 * quotient;
    for (i = 0; i < remain; ++i) {
        printf("  0x%02x", p[i]);
    }
    printf("\n");
    printf("\n");
}

ssize_t aactd_readn(int fd, void *buf, size_t bytes)
{
    size_t left_bytes = bytes;
    uint8_t *ptr = buf;
    ssize_t ret_bytes;

    while (left_bytes > 0) {
        ret_bytes = read(fd, ptr, left_bytes);
        if (ret_bytes < 0) {
            aactd_error("Read error: %s\n", strerror(errno));
            return -1;
        } else if (ret_bytes == 0) {
            aactd_info("Read EOF\n");
            break;
        }

        left_bytes -= ret_bytes;
        ptr += ret_bytes;
    }

    return (bytes - left_bytes);
}

ssize_t aactd_writen(int fd, void *buf, size_t bytes)
{
    size_t left_bytes = bytes;
    uint8_t *ptr = buf;
    ssize_t ret_bytes;

    while (left_bytes > 0) {
        ret_bytes = write(fd, ptr, left_bytes);
        if (ret_bytes < 0){
            aactd_error("Write error: %s\n", strerror(errno));
            return -1;
        } else if (ret_bytes == 0) {
            /*end of file*/
            break;
        }

        left_bytes -= ret_bytes;
        ptr += ret_bytes;
    }

    return (bytes - left_bytes);
}

uint16_t aactd_le_buf_to_uint16(const uint8_t *buf)
{
    union {
        uint16_t v;
        uint8_t u8[2];
    } u;
#ifdef IS_LITTLE_ENDIAN
    memcpy(u.u8, buf, 2);
#else
    u.u8[0] = buf[1];
    u.u8[1] = buf[0];
#endif
    return u.v;
}

int16_t aactd_le_buf_to_int16(const uint8_t *buf)
{
    union {
        int16_t v;
        uint8_t u8[2];
    } u;
#ifdef IS_LITTLE_ENDIAN
    memcpy(u.u8, buf, 2);
#else
    u.u8[0] = buf[1];
    u.u8[1] = buf[0];
#endif
    return u.v;
}

uint32_t aactd_le_buf_to_uint32(const uint8_t *buf)
{
    union {
        uint32_t v;
        uint8_t u8[4];
    } u;
#ifdef IS_LITTLE_ENDIAN
    memcpy(u.u8, buf, 4);
#else
    u.u8[0] = buf[3];
    u.u8[1] = buf[2];
    u.u8[2] = buf[1];
    u.u8[3] = buf[0];
#endif
    return u.v;
}

int32_t aactd_le_buf_to_int32(const uint8_t *buf)
{
    union {
        int32_t v;
        uint8_t u8[4];
    } u;
#ifdef IS_LITTLE_ENDIAN
    memcpy(u.u8, buf, 4);
#else
    u.u8[0] = buf[3];
    u.u8[1] = buf[2];
    u.u8[2] = buf[1];
    u.u8[3] = buf[0];
#endif
    return u.v;
}

void aactd_uint16_to_le_buf(uint16_t value, uint8_t *buf)
{
#ifdef IS_LITTLE_ENDIAN
    memcpy(buf, &value, 2);
#else
    union {
        uint16_t v;
        uint8_t u8[2];
    } u;
    u.v = value;
    buf[0] = u.u8[1];
    buf[1] = u.u8[0];
#endif
}

void aactd_int16_to_le_buf(int16_t value, uint8_t *buf)
{
#ifdef IS_LITTLE_ENDIAN
    memcpy(buf, &value, 2);
#else
    uinon {
        int16_t v;
        uint8_t u8[2];
    } u;
    u.v = value;
    buf[0] = u.u8[1];
    buf[1] = u.u8[0];
#endif
}

void aactd_uint32_to_le_buf(uint32_t value, uint8_t *buf)
{
#ifdef IS_LITTLE_ENDIAN
    memcpy(buf, &value, 4);
#else
    uinon {
        uint32_t v;
        uint8_t u8[4];
    } u;
    u.v = value;
    buf[0] = u.u8[3];
    buf[1] = u.u8[2];
    buf[2] = u.u8[1];
    buf[3] = u.u8[0];
#endif
}

void aactd_int32_to_le_buf(int32_t value, uint8_t *buf)
{
#ifdef IS_LITTLE_ENDIAN
    memcpy(buf, &value, 4);
#else
    uinon {
        int32_t v;
        uint8_t u8[4];
    } u;
    u.v = value;
    buf[0] = u.u8[3];
    buf[1] = u.u8[2];
    buf[2] = u.u8[1];
    buf[3] = u.u8[0];
#endif
}

uint8_t aactd_calculate_checksum(const uint8_t *buf, int buf_len)
{
    uint8_t sum = 0;
    int i;

    for (i = 0; i < buf_len; ++i) {
        sum += *(buf + i);
    }
    return sum;
}
