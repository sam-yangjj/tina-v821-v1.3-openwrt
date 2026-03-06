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
#ifndef __DISP_TEST_HEAD_H
#define __DISP_TEST_HEAD_H

#include <stdio.h>

struct fb {
	int fd;
	unsigned int w;
	unsigned int h;
	unsigned int bpp;
	unsigned int bpp_byte;
	unsigned int mem_size;
	int b_map;
	void *mem;
};

#define PR_DISP_TEST_HEAD "[disptest]: "
#define __DISP_TEST_DEBUG
#ifdef __DISP_TEST_DEBUG
#define DISP_TEST_OK(x) {printf("\x1b[32m");printf(PR_DISP_TEST_HEAD);printf("OK   ");printf x;printf("\e[0m\n");}
#define DISP_TEST_FAIL(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf("FAIL ");printf x;printf("\e[0m\n");}
#define DISP_TEST_WARN(x) {printf("\x1b[33m");printf(PR_DISP_TEST_HEAD);printf("WARN ");printf x;printf("\e[0m\n");}
#define DISP_TEST_DBG(x) {printf x;}
#define DISP_TEST_PUT(x) {printf x;}
#define DISP_TEST_INFO(x) {printf(PR_DISP_TEST_HEAD);printf("INFO ");printf x;}
#define DISP_TEST_ERROR(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf x;printf("\e[0m\n");}
#else
#define DISP_TEST_OK(x) {printf("\x1b[32m");printf(PR_DISP_TEST_HEAD);printf("OK   ");printf x;printf("\e[0m\n");}
#define DISP_TEST_FAIL(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf("FAIL ");printf x;printf("\e[0m\n");}
#define DISP_TEST_WARN(x) {printf("\x1b[33m");printf(PR_DISP_TEST_HEAD);printf("WARN ");printf x;printf("\e[0m\n");}
#define DISP_TEST_DBG(x) do{}while(0)
#define DISP_TEST_PUT(x) {printf x;}
#define DISP_TEST_INFO(x) {printf x;}
#endif

#endif
