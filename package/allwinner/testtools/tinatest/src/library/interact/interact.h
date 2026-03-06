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
/*
 * It's external *.h, including apis for testcase/tinatest/outlog
 */
#ifndef __INTERACT_H
#define __INTERACT_H

#define MAX_PATH 256
#define MAX_TEXT 2048
#define MAX_KEY 512
#define MAX_FPATH 512
#define MAX_TEXT_DFPATH (MAX_TEXT - MAX_FPATH)
#define MAX_TEXT_KEY (MAX_TEXT - MAX_KEY)
#define STR_TRUE "true"
#define STR_FALSE "false"

/*
 * api for testcase
 */
int task(const char *ask, char *reply, int len);
int ttips(const char *tips);
int ttrue(const char *tips);
int tscan(const char *key, const char *tips);
int tupfile(const char *filepath, const char *tips);
int tdownfile(const char *filename, const char *tips);
int tshowimg(const char *filepath, const char *tips);

/*
 * api for TinaTest
 */
int interact_init();
int interact_exit();

/*
 * api for outlog(actor)
 */
typedef int (*f_ask) (const char *testcase, const char *ask,
        char *reply, int len);
typedef int (*f_tips) (const char *testcase, const char *tips);
typedef int (*f_istrue) (const char *testcase, const char *tips);
typedef int (*f_scan) (const char *testcase, const char *tips, const char *key);
typedef int (*f_upfile) (const char *testcase, const char *filepath,
        const char *tips);
typedef int (*f_downfile) (const char *testcase, const char *filename,
        const char *tips);
typedef int (*f_showimg) (const char *testcase, const char *filepath,
        const char *tips);
int interact_register(
        f_ask ask,
        f_tips tips,
        f_istrue istrue,
        f_scan scan,
        f_upfile upfile,
        f_downfile downfile,
        f_showimg showimg);

#endif
