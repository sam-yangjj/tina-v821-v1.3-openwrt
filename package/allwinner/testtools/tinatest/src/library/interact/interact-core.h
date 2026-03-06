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
#ifndef __INTERACT_CORE_H
#define __INTERACT_CORE_H

#include <sys/msg.h>
#include "outlog.h"
#include "interact-actor.h"

/* env variable */
#define MSG_CORE_TO_CASE "MSG_CORE_TO_CASE"
#define MSG_CASE_TO_CORE "MSG_CASE_TO_CORE"

/*
 * msg struction between testcase and core
 *
 * @addr: the address(pid) of testcase
 * @cmd: interact command.
 * @path: path of testcase
 * @text: message buf.
 */
struct msg {
    long addr;
    enum cmd cmd;
    char path[MAX_PATH];
    char text[MAX_TEXT];
};
extern struct msg msg, *pmsg;

/*
 * all attributes for message queue
 *
 * @key: message queue key;
 * @msgid: message queue id;
 * @msgno: the count for interactive request(seens as acting id)
 * @msg_core_addr: the core address for message queue.
 * @len: the valid length of message between core and testcase for message queue.
 */
struct msgattr {
    key_t key;
#define IPC_KEY "/etc/tinatest.json"
    /* core send to testcases */
    int msgid_core_to_case;
#define CORE_TO_CASE_OFFSET 0x00
    /* core receive from testcases */
    int msgid_case_to_core;
#define CASE_TO_CORE_OFFSET 0x01
    unsigned long msgno;
    long msg_core_addr;
    int len;
};
extern struct msgattr msgattr;

#endif
