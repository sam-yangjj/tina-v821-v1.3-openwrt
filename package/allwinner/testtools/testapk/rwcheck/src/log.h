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
// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __LOG_H_
#define __LOG_H_

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

/* enable this macro if you really need, to show all debug log */
#define DEBUG_LOG 0

//delay_errlog may have bug
#if 0
void delay_errlog(int err_no, const char *file, unsigned int line,
		const char *func_name, const char *fmt, ...);
void show_errlog(void);

#define INFO(fmt, ...)							\
	do {								\
		fprintf(stdout, fmt, ## __VA_ARGS__);			\
		fflush(stdout);						\
	} while (0)

#define ERROR(fmt, ...)							\
	do {								\
		delay_errlog(errno, __FILE__, __LINE__, __func__,	\
				fmt, ## __VA_ARGS__);			\
	} while (0)

#if DEBUG_LOG

#define DEBUG(fmt, ...)							\
	do {								\
		delay_errlog(errno, __FILE__, __LINE__, __func__,	\
				fmt, ## __VA_ARGS__);			\
		show_errlog();						\
	} while (0)

#else /* DEBUG_LOG */

#define DEBUG(fmt, ...)

#endif /* DEBUG_LOG */

#else

#if DEBUG_LOG
#define DEBUG(fmt, ...)		do{printf("<%s:%d>"fmt, __func__, __LINE__, ## __VA_ARGS__);fflush(stdout);}while(0)
#define INFO(fmt, ...)		do{printf("<%s:%d>"fmt, __func__, __LINE__, ## __VA_ARGS__);fflush(stdout);}while(0)
#define ERROR(fmt, ...)		do{printf("<%s:%d>"fmt, __func__, __LINE__, ## __VA_ARGS__);fflush(stdout);}while(0)
#else
#define DEBUG(fmt, ...)		do{}while(0)
#define INFO(fmt, ...)		do{printf(fmt, ## __VA_ARGS__);fflush(stdout);}while(0)
#define ERROR(fmt, ...)		do{printf("\e[31m"fmt"\e[0m", ## __VA_ARGS__);fflush(stdout);}while(0)
#endif

#endif /* BUG */

#endif /* __LOG_H_ */
