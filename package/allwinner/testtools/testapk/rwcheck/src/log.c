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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "rwcheck.h"

#if DUMP_TRACE
#include <execinfo.h>
#endif

#define MAX_ERRLOG_LEN 1024
#define DUMP_TRACE_BUF_SIZE 1024
static struct errlog {
	int _errno;
	unsigned int line;
	const char *file;
	const char *func;
	char log[MAX_ERRLOG_LEN];
	struct errlog *next;
#if DUMP_TRACE
	void *trace[DUMP_TRACE_BUF_SIZE];
	int trace_cnt;
#endif
} *errlog;

void delay_errlog(int err_no, const char *file, unsigned int line,
		const char *func_name, const char *fmt, ...)
{
	va_list ap;
	struct errlog *err;

	err = malloc(sizeof(*err));
	if (!err) {
		printf("malloc for errlog failed\n");
		return;
	}
	memset(err, 0, sizeof(*err));

	err->_errno = err_no;
	err->file = file;
	err->line = line;
	err->func = func_name;

	va_start(ap, fmt);
	vsnprintf(err->log, MAX_ERRLOG_LEN, fmt, ap);
	va_end(ap);

	if (errlog) {
		struct errlog *next = errlog, *pre;
		while (next) {
			pre = next;
			next = errlog->next;
		}
		pre->next = err;
	} else {
		errlog = err;
	}

#if DUMP_TRACE
	err->trace_cnt = backtrace(err->trace, DUMP_TRACE_BUF_SIZE);
#endif
}

void show_errlog(void)
{
	struct errlog *err;
#if DUMP_TRACE
	char **str;
	int i;
#endif

	while ((err = errlog)) {
		fprintf(stderr, "\n\t--- ERROR INFO ---\n");
		fprintf(stderr, "\tfile: %s (%d)\n", err->file, err->line);
		fprintf(stderr, "\terrno: %s (%d)\n", strerror(err->_errno),
				err->_errno);
		fprintf(stderr, "\tinfo: %s", err->log);
		if (err->log[strlen(err->log) - 1] != '\n')
			fprintf(stderr, "\n");

#if DUMP_TRACE
		fprintf(stderr, "\tstack:\n");
		str = backtrace_symbols(err->trace, err->trace_cnt);
		for (i = 0; i < err->trace_cnt; i++)
			fprintf(stderr, "\t  | %s\n", str[i]);
		free(str);
#endif
		errlog = errlog->next;
		free(err);
	}
}
