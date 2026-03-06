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
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <libubox/list.h>

#include "log.h"
#include "shmem.h"

#define PAGE_SIZE 4096
#define max(val1,val2) val1 > val2 ? val1 : val2

static int shmid = -1;                  // 共享内存id
static int max_num;                     // 最大使用多个个struct shmem_t
static int used_num = 0;                // 目前已经使用多少个struct shmem_t
static struct list_head probe_list;     // 链表头，记录测试用例的共享内存坐标
static struct shmem_testcase *shmem_testcase = NULL;    // 指向测试用例共享内存的地址
static struct shmem_tinatest *shmem_tinatest = NULL;    // 指向TinaTest共享内存的地址

struct probe {
    char *keypath;
    struct shmem_testcase *shmem;
    struct list_head lnode;
};

/**
 * 寻找或创建测试用例共享内存结构
 *
 * @keypath : 测试用例路径
 */
struct shmem_testcase *shmem_probe(char *keypath)
{
    if (shmid < 0) {
        ERROR("please call shmem_init firstly\n");
        return NULL;
    }
    if (!keypath) {
        ERROR("invail paramater\n");
        return NULL;
    }
    struct probe *p = NULL;
    list_for_each_entry(p, &probe_list, lnode) {
        if (!strcmp(keypath, p->keypath))
            return p->shmem;
    }
    // not existed, create
    p = malloc(sizeof(struct probe));
    if (p == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return NULL;
    }
    p->shmem = &(shmem_testcase[used_num++]);
    p->keypath = strdup(keypath);
    list_add_tail(&(p->lnode), &probe_list);
    return p->shmem;
}

/**
 * 申请和初始化共享内存
 *
 * @max_num : 共享内存最大存储多少个struct shmem_t
 */
struct shmem_tinatest *shmem_init(int _max_num)
{
    if (_max_num < 0) {
        ERROR("invail paramater\n");
        return NULL;
    }
    if (shmid >= 0) {
        ERROR("share memory had been initialized before\n");
        return NULL;
    }
    shmid = shmget(IPC_PRIVATE,
            max(PAGE_SIZE, _max_num * sizeof(struct shmem_testcase) +
                sizeof(struct shmem_tinatest)), IPC_CREAT | 0666);
    if (shmid < 0) {
        ERROR("allocate share memory failed - %s\n", strerror(errno));
        return NULL;
    }
    shmem_tinatest = (struct shmem_tinatest *)shmat(shmid, NULL, 0);
    shmem_testcase = (struct shmem_testcase *)(shmem_tinatest + sizeof(struct shmem_tinatest));
    if ((void *)-1 == shmem_tinatest) {
        ERROR("attach share memory failed - %s\n", strerror(errno));
        shmem_free();
        return NULL;
    }
    max_num = _max_num;
    INIT_LIST_HEAD(&probe_list);
    return shmem_tinatest;
}

/**
 * 释放共享内存空间
 */
int shmem_free()
{
    if (shmid < 0)
        return 0;
    struct probe *pre, *next;
    list_for_each_entry_safe(pre, next, &probe_list, lnode)
        free(pre);
    shmdt(shmem_tinatest);
    return shmctl(shmid, IPC_RMID, 0);
}
