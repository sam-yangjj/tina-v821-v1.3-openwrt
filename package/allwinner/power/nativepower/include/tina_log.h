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
#ifndef __TINA_LOG_H__
#define __TINA_LOG_H__

#include <syslog.h>


#define OPTION_TLOG_LEVEL_CLOSE      0
#define OPTION_TLOG_LEVEL_DEBUG      1
#define OPTION_TLOG_LEVEL_WARNING    2
#define OPTION_TLOG_LEVEL_DEFAULT    3
#define OPTION_TLOG_LEVEL_DETAIL     4

#define TLOG_LEVEL_ERROR     "E/"
#define TLOG_LEVEL_WARNING   "W/"
#define TLOG_LEVEL_INFO      "I/"
#define TLOG_LEVEL_VERBOSE   "V/"
#define TLOG_LEVEL_DEBUG     "D/"

#ifndef TAG
#define TAG "Tina-default"
#endif

#ifndef LOG_TAG
#define LOG_TAG "["TAG"]"
#endif

#ifndef CONFIG_TLOG_LEVEL
#define CONFIG_TLOG_LEVEL OPTION_TLOG_LEVEL_DEFAULT
#endif

#define TLOG(systag, level, fmt, arg...)  \
        syslog(systag, "%s:%s <%s:%u>: " fmt "\n", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#define TTAGLOG(systag, level, tag, fmt, arg...)  \
        syslog(systag, "%s:%s <%s:%u>: " fmt "\n", level, tag, __FUNCTION__, __LINE__, ##arg)


#define TLOGE(fmt, arg...) TLOG(LOG_ERR, TLOG_LEVEL_ERROR, fmt, ##arg)
#define TTAGLOGE(tag, fmt, arg...) TTAGLOG(LOG_ERR, TLOG_LEVEL_ERROR, tag, fmt, ##arg)

#if CONFIG_TLOG_LEVEL == OPTION_TLOG_LEVEL_CLOSE

#define TLOGD(fmt, arg...)
#define TLOGW(fmt, arg...)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)
#define TTAGLOGD(tag, fmt, arg...)
#define TTAGLOGW(tag, fmt, arg...)
#define TTAGLOGI(tag, fmt, arg...)
#define TTAGLOGV(tag, fmt, arg...)

#elif CONFIG_TLOG_LEVEL == OPTION_TLOG_LEVEL_DEBUG

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)
#define TTAGLOGD(tag, fmt, arg...) TTAGLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, tag, fmt, ##arg)
#define TTAGLOGW(tag, fmt, arg...)
#define TTAGLOGI(tag, fmt, arg...)
#define TTAGLOGV(tag, fmt, arg...)

#elif CONFIG_TLOG_LEVEL == OPTION_TLOG_LEVEL_WARNING

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING, TLOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...)
#define TLOGV(fmt, arg...)
#define TTAGLOGD(tag, fmt, arg...) TTAGLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, tag, fmt, ##arg)
#define TTAGLOGW(tag, fmt, arg...) TTAGLOG(LOG_WARNING, TLOG_LEVEL_WARNING, tag, fmt, ##arg)
#define TTAGLOGI(tag, fmt, arg...)
#define TTAGLOGV(tag, fmt, arg...)

#elif CONFIG_TLOG_LEVEL == OPTION_TLOG_LEVEL_DEFAULT

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING, TLOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...) TLOG(LOG_INFO, TLOG_LEVEL_INFO, fmt, ##arg)
#define TLOGV(fmt, arg...)
#define TTAGLOGD(tag, fmt, arg...) TTAGLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, tag, fmt, ##arg)
#define TTAGLOGW(tag, fmt, arg...) TTAGLOG(LOG_WARNING, TLOG_LEVEL_WARNING, tag, fmt, ##arg)
#define TTAGLOGI(tag, fmt, arg...) TTAGLOG(LOG_INFO, TLOG_LEVEL_INFO, tag, fmt, ##arg)
#define TTAGLOGV(tag, fmt, arg...)

#elif CONFIG_TLOG_LEVEL == OPTION_TLOG_LEVEL_DETAIL

#define TLOGD(fmt, arg...) TLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGW(fmt, arg...) TLOG(LOG_WARNING, TLOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...) TLOG(LOG_INFO, TLOG_LEVEL_INFO, fmt, ##arg)
#define TLOGV(fmt, arg...) TLOG(LOG_INFO, TLOG_LEVEL_VERBOSE, fmt, ##arg)
#define TTAGLOGD(tag, fmt, arg...) TTAGLOG(LOG_DEBUG, TLOG_LEVEL_DEBUG, tag, fmt, ##arg)
#define TTAGLOGW(tag, fmt, arg...) TTAGLOG(LOG_WARNING, TLOG_LEVEL_WARNING, tag, fmt, ##arg)
#define TTAGLOGI(tag, fmt, arg...) TTAGLOG(LOG_INFO, TLOG_LEVEL_INFO, tag, fmt, ##arg)
#define TTAGLOGV(tag, fmt, arg...) TTAGLOG(LOG_INFO, TLOG_LEVEL_VERBOSE, tag, fmt, ##arg)

#endif

#endif /*__TINA_LOG_H__*/
