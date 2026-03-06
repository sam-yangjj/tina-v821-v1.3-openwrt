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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "task.h"

// These functions is based on code in book
// <<Advanced Programming in the UNIX Environment>>
// To realize synchronous communication between father process and child process

static struct list_head SYNC_LIST;

struct msync{
    int id;
    int father_fd[2]; //father read, child write
    int child_fd[2]; //child read, fahter write

    struct list_head lnode;
};

int TELL_WAIT(int id)
{
    if (SYNC_LIST.prev == NULL || SYNC_LIST.next == NULL)
        INIT_LIST_HEAD(&SYNC_LIST);

    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            ERROR("id %d is allready existed\n", id);
            return -1;
        }
    }

    msync = NULL;
    msync = malloc(sizeof(struct msync));
    if (msync == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto out1;
    }

    if (pipe(msync->father_fd) < 0 || pipe(msync->child_fd) < 0) {
        ERROR("pipe create error - %s\n", strerror(errno));
        goto out1;
    }

    list_add_tail(&msync->lnode, &SYNC_LIST);
    msync->id = id;
    goto out;

out1:
    close(msync->father_fd[0]);
    close(msync->father_fd[1]);
    close(msync->child_fd[0]);
    close(msync->child_fd[1]);
    free(msync);
    return -1;
out:
    return 0;
}

int TELL_PARENT(int id)
{
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (write(msync->father_fd[1], "c", 1) != 1)
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int WAIT_PARENT(int id)
{
    char c;
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (read(msync->child_fd[0], &c, 1) != 1)
                goto out1;

            if (c != 'p')
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int TELL_CHILD(int id)
{
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (write(msync->child_fd[1], "p", 1) != 1)
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int WAIT_CHILD(int id)
{
    char c;
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (read(msync->father_fd[0], &c, 1) != 1)
                goto out1;

            if (c != 'c')
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}
