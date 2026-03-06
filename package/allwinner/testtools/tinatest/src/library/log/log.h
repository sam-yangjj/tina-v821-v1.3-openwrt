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
#ifndef __MLOG_H
#define __MLOG_H

#include <stdio.h>

#define BASE 1
#define DATA 2

#if DEBUG_LEVEL
void init_log(void);
extern FILE *tty;
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

/* ERROR */
#if DEBUG_LEVEL && defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(tty, "\033[31m" "[ERROR]: (%s-%d): " fmt "\033[0m", \
            __func__, __LINE__, ##arg); \
}while(0)

#elif DEBUG_LEVEL && !defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(tty, "[ERROR]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)

#elif !DEBUG_LEVEL && defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "\033[31m" "[ERROR]: (%s-%d): " fmt "\033[0m", \
            __func__, __LINE__, ##arg); \
}while(0)

#elif !DEBUG_LEVEL && !defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "[ERROR]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)
#endif

/* DEBUG */
#if DEBUG_LEVEL && defined(COLOUR_LOG)
#define DEBUG(level, fmt, arg...) \
do { \
    if (level <= DEBUG_LEVEL) { \
        if (level == BASE) { \
            fprintf(tty, "\033[32m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
        else if (level == DATA) { \
            fprintf(tty, "\033[33m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
    } \
    fflush(tty); \
}while(0)

#elif DEBUG_LEVEL && !defined(COLOUR_LOG)
#define DEBUG(level, fmt, arg...) \
do { \
    if (DEBUG_LEVEL == level || DEBUG_LEVEL > DATA) \
        fprintf(tty, "[DEBUG]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
    fflush(tty); \
}while(0)

#else
#define DEBUG(level, fmt, arg...)
#endif

#endif
