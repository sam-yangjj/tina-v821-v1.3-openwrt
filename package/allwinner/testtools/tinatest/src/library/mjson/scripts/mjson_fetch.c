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
/* It's a API for shell */
#include <stdio.h>
#include <stdlib.h>

#include "mjson.h"

int main(int argc, char *argv[]) {
    for(int num = 1; num < argc; num++) {
        if (argv[num][0] != '/')
            continue;
        if (mjson_load(DEFAULT_JSON_PATH) != 0)
            return -1;

        struct mjson_value val;
        val = mjson_fetch(argv[num]);
        switch(val.type) {
            case mjson_type_null :
                printf("NULL");
                break;
            case mjson_type_boolean :
                printf("%s", val.val.m_boolean == TRUE ? "true" : "false");
                break;
            case mjson_type_double :
                printf("%f", val.val.m_double);
                break;
            case mjson_type_int :
                printf("%d", val.val.m_int);
                break;
            case mjson_type_array :
                for(int cnt = 0, max = val.val.m_array_cnt; cnt < max; cnt++) {
                    if (cnt == (max - 1))
                        printf("%s", val.val.m_array[cnt]);
                    else
                        printf("%s ", val.val.m_array[cnt]);
                }
                break;
            case mjson_type_string :
                printf("%s", val.val.m_string);
                break;
            default :
                break;
        }
        printf("\n");
    }

    return 0;
}
