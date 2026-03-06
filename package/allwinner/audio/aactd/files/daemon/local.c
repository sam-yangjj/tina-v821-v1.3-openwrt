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
#include "local.h"

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "aactd/common.h"
#include "aactd/communicate.h"
#include "eq_sw.h"
#include "drc_hw.h"

unsigned int com_buf_len_max = COM_BUF_LEN_MAX_DEFAULT;

int verbose_level = -1;

int local_process_init(void)
{
    int ret;

    ret = eq_sw_local_init();
    if (ret < 0) {
        aactd_error("eq_sw_local_init failed\n");
        goto err_out;
    }

    ret = drc_hw_local_init();
    if (ret < 0) {
        aactd_error("drc_hw_local_init failed\n");
        goto release_eq_sw;
    }

    return 0;

release_eq_sw:
    eq_sw_local_release();
err_out:
    return ret;
}

int local_process_release(void)
{
    eq_sw_local_release();
    drc_hw_local_release();
    return 0;
}

int read_com_from_local(struct aactd_com *com)
{
    // TODO: implement this
    aactd_error("Not supported\n");
    return -1;

    switch (com->header.type) {
    case EQ_SW:
        break;
    case DRC_HW:
        break;
    default:
        aactd_error("Unknown type\n");
        break;
    }
    return 0;
}

int write_com_to_local(const struct aactd_com *com)
{
    int ret;

    switch (com->header.type) {
    case EQ_SW:
        ret = eq_sw_write_com_to_local(com);
        if (ret < 0) {
            aactd_error("Failed to write EQ_SW com to local\n");
            goto out;
        }
        break;
    case DRC_HW:
        ret = drc_hw_write_com_to_local(com);
        if (ret < 0) {
            aactd_error("Failed to write DRC_HW com to local\n");
            goto out;
        }
        break;
    default:
        aactd_error("Unknown type\n");
        break;
    }

    ret = 0;
out:
    return ret;
}
