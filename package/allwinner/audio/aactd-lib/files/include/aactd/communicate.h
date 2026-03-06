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
#ifndef __AACTD_COMMUNICATE_H__
#define __AACTD_COMMUNICATE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AACTD_COM_HEADER_FLAG 0xAA

struct aactd_com_header {
    uint8_t flag;
    uint8_t version;
    uint8_t command;
    uint8_t type;
    uint32_t data_len;
} __attribute__((packed));

struct aactd_com {
    struct aactd_com_header header;
    uint8_t *data;
    uint8_t checksum;
} __attribute__((packed));

enum aactd_com_command {
    CMD_READ = 0,
    CMD_WRITE = 1,
};

enum aactd_com_type {
    TYPE_RESERVED = 0,
    EQ_SW = 1,
    DRC_HW = 2,
};

int aactd_com_buf_to_header(const uint8_t *buf, struct aactd_com_header *header);
int aactd_com_header_to_buf(const struct aactd_com_header *header, uint8_t *buf);

void aactd_com_print_content(const struct aactd_com *com);
void aactd_com_copy(const struct aactd_com *src_com, struct aactd_com *dst_com);

/* ============ EQ_SW ============ */

struct aactd_com_eq_sw_filter_arg {
    uint8_t type;
    int32_t frequency;
    int32_t gain;
    int32_t quality;    /* the original quality value (float) * 100 */
    uint8_t enabled;
} __attribute__((packed));

struct aactd_com_eq_sw_data {
    uint8_t global_enabled;
    uint8_t filter_num;
    struct aactd_com_eq_sw_filter_arg *filter_args;
} __attribute__((packed));

int aactd_com_eq_sw_buf_to_filter_arg(
        const uint8_t *buf, struct aactd_com_eq_sw_filter_arg *arg);
int aactd_com_eq_sw_filter_arg_to_buf(
        const struct aactd_com_eq_sw_filter_arg *arg, uint8_t *buf);

int aactd_com_eq_sw_buf_to_data(
        const uint8_t *buf, struct aactd_com_eq_sw_data *data);
int aactd_com_eq_sw_data_to_buf(
        const struct aactd_com_eq_sw_data *data, uint8_t *buf);

/* ============ DRC_HW ============ */

struct aactd_com_drc_hw_reg_arg {
    uint32_t offset;
    uint32_t value;
} __attribute__((packed));

struct aactd_com_drc_hw_data {
    uint16_t reg_num;
    struct aactd_com_drc_hw_reg_arg *reg_args;
} __attribute__((packed));

int aactd_com_drc_hw_buf_to_reg_arg(
        const uint8_t *buf, struct aactd_com_drc_hw_reg_arg *arg);
int aactd_com_drc_hw_reg_arg_to_buf(
        const struct aactd_com_drc_hw_reg_arg *arg, uint8_t *buf);

int aactd_com_drc_hw_buf_to_data(
        const uint8_t *buf, struct aactd_com_drc_hw_data *data);
int aactd_com_drc_hw_data_to_buf(
        const struct aactd_com_drc_hw_data *data, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __AACTD_COMMUNICATE_H__ */
