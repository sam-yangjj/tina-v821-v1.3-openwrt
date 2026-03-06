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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "interact-actor.h"

struct list_head list_actor; // list head for all actors.
int cnt_actor = 0;           // count for all actors.

#define INIT_LNODE() \
    struct actor *act = calloc(sizeof(struct actor), 1); \
    if (NULL == act) { \
        ERROR("malloc failed - %s\n", strerror(errno)); \
        return -1; \
    } \
    act->reply = calloc(MAX_TEXT, 1); \
    if (NULL == act->reply) { \
        free(act); \
        ERROR("malloc failed - %s\n", strerror(errno)); \
        return -1; \
    } \
    if (NULL == list_actor.prev) \
        INIT_LIST_HEAD(&list_actor); \
    list_add_tail(&act->lnode, &list_actor); \
    cnt_actor++; \
    DEBUG(BASE, "register interact: %d\n", cnt_actor);

#define ADD_CMD(CMD) \
    act->func[cmd_ ## CMD] = (cmd_func)CMD;

int interact_register(
        f_ask ask,
        f_tips tips,
        f_istrue istrue,
        f_scan scan,
        f_upfile upfile,
        f_downfile downfile,
        f_showimg showimg)
{
    INIT_LNODE();

    ADD_CMD(ask);
    ADD_CMD(tips);
    ADD_CMD(istrue);
    ADD_CMD(scan);
    ADD_CMD(upfile);
    ADD_CMD(downfile);
    ADD_CMD(showimg);

    return 0;
}

/***********************************************************
 * [below] Function to do command.
 **********************************************************/
static int interact_do_ask(struct actor *act)
{
    return ((f_ask)(act->func[acting->cmd]))(acting->path,
            acting->text, act->reply, MAX_TEXT);
}

static int interact_do_istrue(struct actor *act)
{
    int ret = ((f_istrue)(act->func[acting->cmd]))(acting->path, acting->text);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == (int)true ? STR_TRUE : STR_FALSE);
    return 0;
}

static int interact_do_tips(struct actor *act)
{
    return ((f_tips)(act->func[acting->cmd]))(acting->path, acting->text);
}

static int interact_do_scan(struct actor *act)
{
    const char *key = acting->text;
    const char *tips = acting->text + 512;
    return ((f_upfile)(act->func[acting->cmd]))(acting->path, tips, key);
}

static int interact_do_upfile(struct actor *act)
{
    const char *filepath = acting->text;
    const char *tips = acting->text + 512;
    int ret = ((f_upfile)(act->func[acting->cmd]))(acting->path, filepath, tips);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == (int)true ? STR_TRUE : STR_FALSE);
    return 0;
}

static int interact_do_downfile(struct actor *act)
{
    const char *filename = acting->text;
    const char *tips = acting->text + 512;
    return ((f_upfile)(act->func[acting->cmd]))(acting->path, filename, tips);
}

static int interact_do_showimg(struct actor *act)
{
    const char *filepath = acting->text;
    const char *tips = acting->text + 512;
    int ret = ((f_showimg)(act->func[acting->cmd]))(acting->path, filepath, tips);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == 1 ? STR_TRUE : STR_FALSE);
    return 0;
}
/***********************************************************
 * [above] Function to do command.
 **********************************************************/

#define interact_do(CMD, need_resp) \
    if (need_resp != false) { \
        pthread_mutex_lock(&acting->mutex); \
        acting->need_respond = true; \
        pthread_mutex_unlock(&acting->mutex); \
    } \
    if (!act->func[acting->cmd]) \
        return -1; \
    return interact_do_ ## CMD(act);

int interact_actor_do(struct actor *act)
{
    switch (acting->cmd) {
    case cmd_ask:
        interact_do(ask, true);
    case cmd_istrue:
        interact_do(istrue, true);
    case cmd_scan:
        interact_do(scan, true);
    case cmd_tips:
        interact_do(tips, false);
    case cmd_upfile:
        interact_do(upfile, true);
    case cmd_downfile:
        interact_do(downfile, true);
    case cmd_showimg:
        interact_do(showimg, true);
    default:
        return -1;
    }
}
