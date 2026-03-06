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
#include <string.h>
#include <unistd.h>
#include "interact-core.h"

/********************************************************
 * [below]: library for c api
 *******************************************************/
static char *testcase = NULL;
static int initenv(enum cmd cmd)
{
    if (msgattr.msgid_case_to_core == 0) {
        char *p = getenv(MSG_CASE_TO_CORE);
        if (NULL == p)
            return -1;
        msgattr.msgid_case_to_core = atoi(p);
    }
    if (msgattr.msgid_core_to_case == 0) {
        char *p = getenv(MSG_CORE_TO_CASE);
        if (NULL == p)
            return -1;
        msgattr.msgid_core_to_case = atoi(p);
    }
    if (testcase == NULL) {
        testcase = getenv(TESTCASE_PATH);
        if (NULL == testcase)
            return -1;
    }

    memset(pmsg->text, 0 , MAX_PATH);
    pmsg->addr = getpid();
    pmsg->cmd = cmd;
    strncpy(pmsg->path, testcase, min(strlen(testcase), MAX_PATH));

    return 0;
}

static int sendmsg()
{
    int ret = msgsnd(msgattr.msgid_case_to_core, pmsg, msgattr.len, 0);
    return ret;
}

static int recvmsg()
{
    int ret = 0;
    ret = msgrcv(msgattr.msgid_core_to_case, pmsg, msgattr.len, getpid(), MSG_NOERROR);
    if (ret < 0 || pmsg->cmd == cmd_err)
        return -1;
    else
        return 0;
}

/********************************************************
 * [above]: library for c api
 * [below]: c/c++ api for testcase
 *******************************************************/

#ifdef CONFIG_INTERACT_TTIPS
int ttips(const char *tips)
{
    if (initenv(cmd_tips) < 0)
        return -1;

    strncpy(pmsg->text, tips, min(strlen(tips), MAX_TEXT));
    return sendmsg();
}
#else
int ttips(const char *tips)
{
	return 0;
}
#endif

#ifdef CONFIG_INTERACT_API
int task(const char *ask, char *reply, int len)
{
    if (initenv(cmd_ask) < 0)
        return -1;

    strncpy(pmsg->text, ask, min(strlen(ask), MAX_TEXT));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;
    strncpy(reply, pmsg->text, min(strlen(pmsg->text), len));

    return 0;
};

int ttrue(const char *tips)
{
    if (initenv(cmd_istrue) < 0)
        return -1;

    strncpy(pmsg->text, tips, min(strlen(tips), MAX_TEXT));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;

    if (!strncmp(pmsg->text, STR_TRUE, sizeof(STR_TRUE)))
        return true;
    else
        return false;
};
#else

int task(const char *ask, char *reply, int len)
{
	return 0;
}

int ttrue(const char *tips)
{
	return 0;
}
#endif

int tscan(const char *key, const char *tips)
{
    if (initenv(cmd_scan) < 0)
        return -1;

    //buf[512]: key
    //buf[MAX_TEXT_KEY]: tips
    strncpy(pmsg->text, key, min(strlen(key), MAX_KEY));
    if (*(pmsg->text + MAX_KEY - 2) != '\0')
        *(pmsg->text + MAX_KEY - 1) = '\0';
    strncpy(pmsg->text + MAX_KEY, tips, min(strlen(tips), MAX_TEXT_KEY));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;
    if (!strncmp(pmsg->text, STR_TRUE, sizeof(STR_TRUE)))
        return true;
    else
        return false;
};

int tupfile(const char *filepath, const char *tips)
{
    if (initenv(cmd_upfile) < 0)
        return -1;

    //buf[512]: filepath
    //buf[MAX_TEXT_DFPATH]: tips
    strncpy(pmsg->text, filepath, min(strlen(filepath), MAX_FPATH));
    if (*(pmsg->text + MAX_FPATH - 2) != '\0')
        *(pmsg->text + MAX_FPATH - 1) = '\0';
    strncpy(pmsg->text + MAX_FPATH, tips, min(strlen(tips), MAX_TEXT_DFPATH));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;

    return 0;
};

int tdownfile(const char *filename, const char *tips)
 {
    if (initenv(cmd_downfile) < 0)
        return -1;

    //buf[512]: filepath
    //buf[MAX_TEXT_DFPATH]: tips
    strncpy(pmsg->text, filename, min(strlen(filename), MAX_FPATH));
    if (*(pmsg->text + MAX_FPATH - 2) != '\0')
        *(pmsg->text + MAX_FPATH - 1) = '\0';
    strncpy(pmsg->text + MAX_FPATH, tips, min(strlen(tips), MAX_TEXT_DFPATH));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;

    return 0;
};

int tshowimg(const char *filepath, const char *tips)
{
    if (initenv(cmd_showimg) < 0)
        return -1;

    //buf[512]: filepath
    //buf[MAX_TEXT_DFPATH]: tips
    strncpy(pmsg->text, filepath, min(strlen(filepath), MAX_FPATH));
    if (*(pmsg->text + MAX_FPATH - 2) != '\0')
        *(pmsg->text + MAX_FPATH - 1) = '\0';
    strncpy(pmsg->text + MAX_FPATH, tips, min(strlen(tips), MAX_TEXT_DFPATH));
    if (sendmsg() < 0)
        return -1;

    if (recvmsg() < 0)
        return -1;

    if (!strncmp(pmsg->text, STR_TRUE, sizeof(STR_TRUE)))
        return 1;
    else
        return 0;
};
