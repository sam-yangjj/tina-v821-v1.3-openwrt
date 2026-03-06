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
#include "plugin_interface.h"
#include <signal.h>
#include <stdio.h>
#include "../utils/log.h"
size_t get_image_size(PixelFormat fmt, int width, int height) {
    switch (fmt) {
        case PIXEL_FORMAT_NV12:
        case PIXEL_FORMAT_NV21:
            // Semi-planar YUV 4:2:0: Y plane (W×H) + UV plane (W×H/2)
            return width * height * 3 / 2;

        case PIXEL_FORMAT_YUV420P:
        case PIXEL_FORMAT_YV12:
            // Planar YUV 4:2:0: Y (W×H) + U (W×H/4) + V (W×H/4)
            return width * height * 3 / 2;

        case PIXEL_FORMAT_RGB565:
            // 2 bytes per pixel
            return width * height * 2;

        case PIXEL_FORMAT_RGB888:
        case PIXEL_FORMAT_BGR888:
            // 3 bytes per pixel
            return width * height * 3;

        case PIXEL_FORMAT_ARGB8888:
        case PIXEL_FORMAT_ABGR8888:
        case PIXEL_FORMAT_RGBA8888:
            // 4 bytes per pixel
            return width * height * 4;

        case PIXEL_FORMAT_GRAY8:
            // 1 byte per pixel
            return width * height;

        default:
            return 0; // Unknown format
    }
}

int plugin_link(Plugin* upstream, Plugin* downstream) {
    if (!upstream || !downstream) return -1;
    upstream->downstream = downstream;
    downstream->upstream = upstream;
    return 0;
}

int plugin_link_many(Plugin* first, ...) {
    va_list args;
    va_start(args, first);
    Plugin* prev = first;
    Plugin* curr;

    while ((curr = va_arg(args, Plugin*)) != NULL) {
        plugin_link(prev, curr);
        prev = curr;
    }

    va_end(args);
    return 0;
}

int plugin_chain_start(Plugin* first) {
    Plugin* current = first;
    while (current) {
        if (current->start)
            current->start(current);
        current = current->downstream;
    }
    return 0;
}

void pipeline_main_loop()
{
    sigset_t sigset;
    int sig;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

    // 阻塞信号处理，使其他线程不会抢占处理
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    LOG_INFO("[plugin] Running... press Ctrl+C to exit.\n");

    if (sigwait(&sigset, &sig) == 0) {
        LOG_INFO("[plugin] Caught signal %d, exiting...\n", sig);
    } else {
        perror("sigwait failed");
        exit(1);
    }

}

int plugin_chain_stop(Plugin* first) {
    // 首先找到链的末端（最后一个插件）
    Plugin* last = first;
    while (last && last->downstream) {
        last = last->downstream;
    }

    // 从下游开始停止，逐步向上游停止
    Plugin* current = last;
    while (current) {
        if (current->stop) {
            LOG_INFO("Stopping plugin: %s\n", current->name ? current->name : "unnamed");
            current->stop(current);
        }
        current = current->upstream;
    }
    return 0;
}
