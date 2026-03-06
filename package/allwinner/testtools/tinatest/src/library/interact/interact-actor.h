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
#ifndef __INTERACT_ACTOR_H
#define __INTERACT_ACTOR_H
#include "outlog.h"
#include <sys/select.h>

enum cmd {
    cmd_tips = 0,
    cmd_ask,
    cmd_istrue,
    cmd_scan,
    cmd_upfile,
    cmd_downfile,
    cmd_showimg,
    cmd_cnt, //作为cmd个数计数,命令务必写在此前
    cmd_err,
};

/*
 * struction for a actor.
 *
 * @func: all cmds function pointer.
 * @reply: the buf for reply from outlog to testcase.
 * @pth: the pthread key for actor thread.
 * @lnode: list node.
 */
typedef void (*cmd_func)(void *);   // a general pointer for function.
struct actor {
    cmd_func func[cmd_cnt];
    char *reply;
    pthread_t pth;
    struct list_head lnode;
};

/*
 * struction for interaction between core and outlog thread,
 * which stores the infomation of doing-interaction.
 *
 * @id: id for acting
 * @end_cnt: count for finishing actor.
 * @need_respond: which, set by outlog, means core should respond to testcase.
 * @cmd: id for cmd
 * @path: the testcase name which couse this acting.
 * @text: the text word from testcase to core/outlog.
 * @reply: the buf for relpy from outlog to testcase.
 * @rfd: a set of fd work for select.
 * @timeout: select timeout.
 * @pfd: fd between core and outlog.
 * @mutex: mutex lock while accessing this struction: acting.
 */
struct acting {
    /* init when ask */
    int id;
    int end_cnt;
    bool need_respond;
    enum cmd cmd;
    const char *path;
    const char *text;
    struct {
        char *buf;
        int len;
    } reply;

    /* init when init */
    fd_set rfd;
#define ACTING_TIMEOUT_SEC 180
    struct timeval timeout;
    int pfd[2];
    pthread_mutex_t mutex;
};
extern struct acting *acting;

extern int interact_actor_do(struct actor *act);
extern struct list_head list_actor;
extern int cnt_actor;

#endif
