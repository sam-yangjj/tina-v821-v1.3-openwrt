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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <getopt.h>

#include <libubox/list.h>
#include "mjson.h"
#include "fileops.h"
#include "task.h"

int main(int argc, char *argv[])
{
#if DEBUG_LEVEL
    init_log();
#endif
    int opt, flags = 0, cnt = 0;
    char *reboot_keypath = NULL;
    while ((opt = getopt(argc, argv, ":r:p")) != -1) {
        switch (opt) {
            case 'p':
                flags = 1;
                DEBUG(BASE, "run in print mode\n");
                break;
            case 'r':
                flags = 2;
                reboot_keypath = optarg;
                DEBUG(BASE, "run in reboot mode with task %s\n", optarg);
                break;
            case ':':
                ERROR("option -%c need a argument\n", optopt);
                exit(1);
            default:
                ERROR("Unknown option : %c\n", (char)optopt);
                exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    // clean the last '/'
    for (cnt = 0; cnt < argc; cnt++) {
        char *p = argv[cnt];
        int len = strlen(p);
        if (len != 1 && p[len - 1] == '/')
            p[len - 1] = '\0';
    }

    // init mjson
    if (mjson_load(DEFAULT_JSON_PATH) != 0) {
        ERROR("load %s failed\n", DEFAULT_JSON_PATH);
        goto err;
    }

    /* -p */
    if (flags & (1 << 0)) {
        if (argc == 0) {
            DEBUG(BASE, "draw all\n");
            mjson_draw_tree("/");
        }
        else {
            for (cnt = 0; cnt < argc; cnt++) {
                DEBUG(BASE, "draw %s\n", argv[cnt]);
                mjson_draw_tree(argv[cnt]);
            }
        }
        exit(0);
    }

    // init task
    if ((cnt = task_init(argc, argv)) < 0) {
        ERROR("init task failed\n");
        goto err;
    } else if (cnt == 0) {
        ERROR("NO ACTIVE TESTCASE\n");
        exit(0);
    }

    /* -r */
    if (flags & (1 << 1)) { //reboot mode
        DEBUG(BASE, "%s has rebooted, recovering ...\n", reboot_keypath);
        if(task_recover_after_reboot(reboot_keypath) < 0) {
            ERROR("recover after reboot failed\n");
            goto err;
        }
    }

    return task_begin();
err:
    return 1;
}
