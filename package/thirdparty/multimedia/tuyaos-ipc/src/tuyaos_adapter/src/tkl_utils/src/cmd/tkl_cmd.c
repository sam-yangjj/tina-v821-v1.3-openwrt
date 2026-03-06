/**
 * @file tkl_cmd.c
 * @brief the implements of ty os execute and script run
 * @version 0.1
 * @date 2019-08-15
 *
 * @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
 *
 */

#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include "sys_log.h"
#include "tkl_cmd.h"

#ifndef TAG
#define TAG "tkl_cmd"
#endif

static pid_t    *childpid = NULL;
static int      maxfd;
#define SHELL   "/bin/sh"
#ifdef  OPEN_MAX
static long openmax = OPEN_MAX;
#else
static long openmax = 0;
#endif
#define OPEN_MAX_GUESS 256

#define TKL_CMD_MAX 1024

OPERATE_RET tkl_cmd_run(const char *cmd, char *buffer,int *len)
{
    int pfd[2];
    FILE    *fp;
    pid_t   pid;
    int stat;
    int flag = 0;
    char *node = NULL;
    int i = 0;
    char *argv[32];
    OPERATE_RET iret = OPRT_COM_ERROR;
    char cmd_buf[TKL_CMD_MAX] = {0};
    char *save = NULL;

    if(cmd == NULL) {
        LOGE(TAG,"invalid param\n"); 
        return OPRT_INVALID_PARM;
    }

    if(strlen(cmd) <= 0 || strlen(cmd) >= TKL_CMD_MAX) {
        LOGE(TAG,"cmd is not valid\n");
        return OPRT_INVALID_PARM;
    }

    strcpy(cmd_buf,cmd);

    node = strtok_r((char*)cmd_buf," ",&save);

    if(node == NULL ||  strrchr(node,'/') == NULL) {
        LOGE(TAG,"run cmd need path\n");
        return OPRT_COM_ERROR;
    }

    if(access(node,F_OK|X_OK)) {
        LOGE(TAG,"%s is not valid cmd\n",node);
        return OPRT_COM_ERROR;
    }

    while(node != NULL && i< 31) {
//      LOGD(TAG,"param:%s\n",node);
	argv[i] = node;
	i++;
        node = strtok_r(NULL," ",&save);
    }

    argv[i] = NULL;


    if((buffer != NULL) && (len != NULL) && (*len  > 0)) {
        flag = 1;
    }else {
        flag = 0; 
    }

    if(flag) {
        if (pipe(pfd) < 0) {
            LOGE(TAG,"pipe open fail\n");
            return OPRT_COM_ERROR;
        }
    }

    if ((pid = vfork()) < 0) {
        LOGE(TAG,"fork fail\n");

	if(flag) {
            close(pfd[0]);
            close(pfd[1]);
	}
        return  OPRT_COM_ERROR;   /* errno set by fork() */
    }
    
    if (pid == 0) {/* child */

        if(flag) {
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        }

        //execvp(argv[0],argv);
        execv(argv[0],argv);

        _exit(127);
    }

    if(flag) {

        close(pfd[1]);
        if ( (fp = fdopen(pfd[0],"r")) == NULL) {
            close(pfd[0]);
	    return OPRT_COM_ERROR;
        }

        memset(buffer, 0x0, *len );
        read( pfd[0], buffer, *len);


        buffer[*len - 1] = '\0';
        *len = strlen(buffer) + 1;
        close(pfd[0]);
        fclose(fp);
    }

    while (waitpid(pid, &stat, 0) < 0) {
        if (errno != EINTR) {
            return OPRT_COM_ERROR;
	 }
    }

    if (WIFEXITED(stat)) {
        if(WEXITSTATUS(stat) == 0) {
            iret = OPRT_OK;
        }
    } 

    return iret;
}

static long __open_max(void)
{
    if (openmax == 0) {      /* first time through */
        errno = 0;

        if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
            if (errno == 0) {
                openmax = OPEN_MAX_GUESS;    /* it's indeterminate */
            } else {
                LOGE(TAG,"sysconf error for _SC_OPEN_MAX");
            }
        }
    }
    return openmax;
}

static int __close_all_fd(void)
{
    int i = 0;
    struct rlimit rl;

    if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        LOGE(TAG,"getrlimit(RLIMIT_NOFILE, &rl)");
        return -1;
    }

    if(rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    for(i = 3; i < rl.rlim_max; i++) {
        close(i);
    }

    return 0;
}

static FILE* __popen(const char *cmdstring, const char *type)
{
    int     i, pfd[2];
    pid_t   pid;
    FILE    *fp;

    if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {
        LOGE(TAG,"popen is errno:%d cmdstring:%s\n",errno,cmdstring);
        errno = EINVAL;
        return(NULL);
    }

    if (childpid == NULL) {
        maxfd = __open_max();
        if((childpid = calloc(maxfd, sizeof(pid_t))) == NULL) {
            LOGE(TAG,"open is calloc err:%d cmdstring:%s\n",errno,cmdstring);
            return NULL;
        }
    }

    if (pipe(pfd) < 0) {
        LOGE(TAG,"__popen is pipe err:%d cmdstring:%s\n",errno,cmdstring);
        return(NULL);
    }

    if ((pid = vfork()) < 0) {
        LOGE(TAG,"popen is fork err:%d\n",errno);
        return(NULL);
    } else if (pid == 0) {               
        if (*type == 'r') {
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        } else {
            close(pfd[1]);
            if (pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }
        }

        for (i = 0; i < maxfd; i++)
            if (childpid[ i ] > 0)
                close(i);

        __close_all_fd();

        execl(SHELL, "sh", "-c", cmdstring, (char *) 0);
        _exit(127);
    }

    if (*type == 'r') {
        close(pfd[1]);
        if ( (fp = fdopen(pfd[0], type)) == NULL)
            return(NULL);
    } else {
        close(pfd[0]);
        if ( (fp = fdopen(pfd[1], type)) == NULL)
            return(NULL);
    }

    childpid[fileno(fp)] = pid;

    return(fp);
}

static int __pclose(FILE *fp)
{
    int     fd, stat;
    pid_t   pid;

    if (childpid == NULL)
        return(-1);

    fd = fileno(fp);
    if ( (pid = childpid[fd]) == 0)
        return(-1);

    childpid[fd] = 0;

    while (waitpid(pid, &stat, 0) < 0)
        if (errno != EINTR)
            return(-1); 

    if (fclose(fp) == EOF)
        return(-1);

    return(stat);
}

OPERATE_RET tkl_cmd_run_shell(const char* cmd, char* buffer, int* len)
{
    FILE *fp = NULL;

    if(cmd == NULL) {
        LOGE(TAG,"invalid param\n");
        return OPRT_INVALID_PARM;	
    }

    fp = __popen(cmd, "r");
    if(fp == NULL) {
        LOGE(TAG,"popen is null %s\n",cmd);
        return OPRT_COM_ERROR;
    }

    if (NULL != buffer && len != 0 && *len > 0) {
        fread(buffer,*len,1,fp);
        buffer[*len - 1] = '\0';

        *len = strlen(buffer);
    }

    __pclose(fp);
    fp = NULL;

    return OPRT_OK;
}
