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
#ifndef __OUTLOG_H
#define __OUTLOG_H

#include "mjson.h"
#include "task.h"
#include "syskey.h"
#include "interact.h"

#define OUTLOG_PATH "/sys/global/info/outlog"

typedef int (*before_all) (struct list_head *TASK_LIST); //所有任务开始前
typedef int (*before_one) (struct task *task);          //单个任务开始前
typedef int (*after_one_begin) (struct task *task);     //单个任务开始后
typedef int (*after_one_once) (struct task *task);      //单个任务执行一次后
typedef int (*after_one_end) (struct task *task);       //单个任务完成后
typedef int (*after_all) (struct list_head *TASK_LIST);  //所有任务完成后

int outlog_register(
        before_all b_all,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all);

int outlog_register_ex(
        before_all b_all,
        before_one b_one,
        after_one_begin a_one_begin,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all);

// 下列函数由 tinatest 调用
int outlog_init(void);      //根据tinatest.json注册插件列表
int outlog_call_before_all(struct list_head *TASK_LIST);     //调用'所有任务开始前'注册的函数
int outlog_call_before_one(struct task *task);               //调用'单个任务开始前'注册的函数
int outlog_call_after_one_begin(struct task *task);          //调用'单个任务开始后'注册的函数
int outlog_call_after_one_once(struct task *task);           //调用'单个任务执行一次后'注册的函数
int outlog_call_after_one_end(struct task *task);            //调用'单个任务完成后'注册的函数
int outlog_call_after_all(struct list_head *TASK_LIST);      //调用'所有任务完成后'注册的函数

#endif
