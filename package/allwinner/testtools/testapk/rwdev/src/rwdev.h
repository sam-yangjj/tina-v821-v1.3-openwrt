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
#ifndef __RWCHECK_H
#define __RWCHECK_H

#include <stdio.h>
#include <libubox/list.h>
#include "crc16.h"
#include "log.h"
#include "speed.h"

#define min(val1, val2) (val1 > val2 ? val2 : val1)
#define is_power_of_2(val) !(val & (val - 1))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define KB ( 1024 )
#define MB ( KB * 1024 )
#define GB ( MB * 1024 )

#define ACTION_WRITE (1 << 0)
#define ACTION_READ (1 << 1)
#define ACTION_VERIFY (1 << 2)

typedef struct size_unit {
    unsigned long sumbytes;
    unsigned long size; //size match units, humanized view
    char units[3];      //KB,MB or GB
} unit;

struct data_list {
    char *data;
    struct list_head lnode;
};

struct size_list {
    unit size;
    struct list_head lnode;
};

struct task {
    char action;
    char *dev;
    char *buf;
    bool def_data;
    unit testsize;
    unsigned short wr_crc;
    unsigned short rd_crc;
    unsigned int times;
    float speed;
    struct list_head data_head;
    struct list_head size_head;
    struct size_list *size;
    struct data_list *data;
};

#define VERSION "v0.0.4"
#define COMPILE "Compiled in " __DATE__ " at " __TIME__
#endif
