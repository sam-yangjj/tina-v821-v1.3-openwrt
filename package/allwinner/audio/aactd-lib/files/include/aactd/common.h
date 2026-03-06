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
#ifndef __AACTD_COMMON_H__
#define __AACTD_COMMON_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define aactd_info(format, arg...) \
    do { \
        printf("[INFO](%s:%d): "format, __func__, __LINE__, ##arg); \
    } while (0)

#define aactd_error(format, arg...) \
    do { \
        printf("[ERROR](%s:%d): "format, __func__, __LINE__, ##arg); \
    } while (0)

void aactd_print_original_buf(const uint8_t *buf, int buf_len);

ssize_t aactd_readn(int fd, void *buf, size_t bytes);
ssize_t aactd_writen(int fd, void *buf, size_t bytes);

uint16_t aactd_le_buf_to_uint16(const uint8_t *buf);
int16_t aactd_le_buf_to_int16(const uint8_t *buf);
uint32_t aactd_le_buf_to_uint32(const uint8_t *buf);
int32_t aactd_le_buf_to_int32(const uint8_t *buf);

void aactd_uint16_to_le_buf(uint16_t value, uint8_t *buf);
void aactd_int16_to_le_buf(int16_t value, uint8_t *buf);
void aactd_uint32_to_le_buf(uint32_t value, uint8_t *buf);
void aactd_int32_to_le_buf(int32_t value, uint8_t *buf);

uint8_t aactd_calculate_checksum(const uint8_t *buf, int buf_len);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __AACTD_COMMON_H__ */
