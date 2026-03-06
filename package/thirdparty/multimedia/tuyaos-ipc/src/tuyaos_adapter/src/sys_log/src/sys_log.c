/*
 * Copyright (C) 2021 Tuya Inc.
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include "sys_log.h"
#ifdef CONFIG_TUYA_SYS_LOG_JOURNALD
    #include <systemd/sd-journal.h>
#endif

#if defined (CONFIG_TUYA_SYS_LOG_LEVEL_SAVE)  || defined (CONFIG_TUYA_SYS_LOG_FILTER)
#include "bcmnvram.h"
#endif

#define LOG_IDENT_MAX (32)
#define LOG_DEFAULT_IDENT "sys_log"
#define LOG_LEVEL_KEY "persist.log.level"
#define LOG_FILTER_KEY "persist.log.filter"

static int sg_log_init_flag = 0;
static FILE *sg_log_file_fd = NULL;
static TUYA_LOG_MODE_E sg_log_mode = TUYA_LOG_MODE_NONE;
static TUYA_LOG_LEVEL_E sg_log_level = TUYA_LOG_LEVEL_NONE;
static TUYA_LOG_CALLBACK sg_log_callback = NULL;
static char sg_log_filter[LOG_IDENT_MAX] = {0};
static pthread_mutex_t sg_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static int __sys_get_log_level(int* level) 
{
    int iret = -1;
#ifdef CONFIG_TUYA_SYS_LOG_LEVEL_SAVE
    char buf[NVRAM_MAX_VALUE_LEN] = {0};

    if(level == NULL) {
        fprintf(stderr,"invalid param\n");
	return -1;
    } 

    memset(buf,0,sizeof(NVRAM_MAX_VALUE_LEN));

    if(nvram_get(LOG_LEVEL_KEY,buf) == 0 && strlen(buf) ) {
        *level = atoi(buf);
	iret = 0;
    }
#endif
    return iret;
}

static int __sys_get_log_filter(char *filter,int len)
{
    int iret = -1;

#ifdef CONFIG_TUYA_SYS_LOG_FILTER
    char buf[NVRAM_MAX_VALUE_LEN] = {0};

    if(filter == NULL || len == 0) {
        fprintf(stderr,"invalid param\n");
	return -1;
    }

    memset(buf,0,sizeof(NVRAM_MAX_VALUE_LEN));

    if(nvram_get(LOG_FILTER_KEY,buf) == 0 && strlen(buf)) {
	strncpy(filter,buf,len);
	iret = 0;
    }
#endif
    return iret;

}

int sys_log_init(TUYA_LOG_MODE_E mode,TUYA_LOG_LEVEL_E level,const char *log_file)
{
    int _level;
    FILE * log_fd = NULL;
    TUYA_LOG_LEVEL_E log_level = level;
    char log_filter[LOG_IDENT_MAX] = {0};

    pthread_mutex_lock(&sg_log_mutex);
    if(sg_log_init_flag == 1) {
//      fprintf(stderr,"log is inited\n");
        pthread_mutex_unlock(&sg_log_mutex);
        return 0;
    }

    pthread_mutex_unlock(&sg_log_mutex);

    if(__sys_get_log_level((int*)&_level) == 0) {
        log_level = (TUYA_LOG_LEVEL_E) _level;
    }

    __sys_get_log_filter(log_filter,sizeof(log_filter));

    if((mode&TUYA_LOG_MODE_FILE) && log_file == NULL) {
        fprintf(stderr,"log file is invalid\n");
	return -1;
    }

    if(log_file != NULL) {
        if ((log_fd = fopen(log_file, "a+")) == NULL) {
            fprintf(stderr, "%s open fail\n",log_file);
	    return -1;
        }
    }

    pthread_mutex_lock(&sg_log_mutex);
    sg_log_mode = mode;
    sg_log_level = log_level;

    if(log_fd != NULL) {
	if(sg_log_file_fd) {
            fclose(sg_log_file_fd);	
	}

        sg_log_file_fd = log_fd;
    }

    if(strlen(log_filter)) {
        strncpy(sg_log_filter,log_filter,sizeof(sg_log_filter));
    }
   
    sg_log_init_flag = 1;

    pthread_mutex_unlock(&sg_log_mutex);

//  printf("level:%d,mode:%x\n",level,mode);
//  printf("log_filter:%s\n",log_filter);

    return 0;
}

int sys_log_set_callback(TUYA_LOG_CALLBACK cb)
{

    pthread_mutex_lock(&sg_log_mutex);
    if(cb) {
        sg_log_callback  = cb;
    }else {
        sg_log_callback = NULL;
    }

    pthread_mutex_unlock(&sg_log_mutex);

    return 0;
}

static int __sys_log_msg(FILE *fd,const char *ident,int level, char * msg)
{
    char time_str[64];
    time_t raw_time;
    struct tm tm_time;
    char *level_str;

    time(&raw_time);
    gmtime_r(&raw_time, &tm_time);
    strftime (time_str, sizeof(time_str), "%F %T", &tm_time);

    switch (level) {
    case TUYA_LOG_LEVEL_ERROR:
        level_str = "ERROR";
         break;
    case TUYA_LOG_LEVEL_WARNING:
        level_str = "WARNING";
        break;
    case TUYA_LOG_LEVEL_INFO:
        level_str = "INFO";
        break;
    case TUYA_LOG_LEVEL_DEBUG:
        level_str = "DEBUG";
        break;
    default:
        level_str = "DEBUG";
        break;
    }

    fprintf(fd, "%s %s %s: %s\n",time_str,ident,level_str, msg);
    fflush(fd);
    return 0;
}

static int __sys_log_msg_console(const char *ident,int level, char * msg) 
{
    FILE *out;

    if ((level&TUYA_LOG_LEVEL_WARNING) || (level&TUYA_LOG_LEVEL_ERROR)) {
        out = stderr;
    } else {
        out = stdout;
    }

    return __sys_log_msg(out,ident,level,msg);
}

static int __sys_log_msg_file(const char *ident,int level,char *msg)
{
    int iret = 0;
    FILE *out;

    pthread_mutex_lock(&sg_log_mutex);
    out = sg_log_file_fd;

    if(out) {
       iret =  __sys_log_msg(out,ident,level,msg);
     }

    pthread_mutex_unlock(&sg_log_mutex);
    return iret;
}

static int __sys_log_msg_syslog(const char *ident,int level, char *msg) 
{
    openlog(ident, LOG_PID|LOG_CONS, LOG_USER);

    switch (level) {
    case TUYA_LOG_LEVEL_ERROR:
      syslog( LOG_ERR, "%s",msg);
      break;
    case TUYA_LOG_LEVEL_WARNING:
      syslog( LOG_WARNING, "%s",msg);
      break;
    case TUYA_LOG_LEVEL_INFO:
      syslog( LOG_INFO, "%s",msg);
      break;
    case TUYA_LOG_LEVEL_DEBUG:
      syslog( LOG_DEBUG, "%s",msg);
      break;
  }
  closelog();
  return 0;
}

#ifdef CONFIG_TUYA_SYS_LOG_JOURNALD

static int __sys_log_msg_journald(const char* ident,int level,char *msg) 
{
    switch (level) {
    case TUYA_LOG_LEVEL_ERROR:
        sd_journal_print(LOG_ERR,"%s %s",ident,msg);
        break;
    case TUYA_LOG_LEVEL_WARNING:
        sd_journal_print(LOG_WARNING,"%s %s",ident,msg);
        break;
    case TUYA_LOG_LEVEL_INFO:
        sd_journal_print(LOG_INFO,"%s %s",ident,msg);
        break;
    case TUYA_LOG_LEVEL_DEBUG:
    default:
        sd_journal_print(LOG_DEBUG,"%s %s",ident,msg);
        break;
    }

    return 0; 
}
#endif

int sys_log_msg(TUYA_LOG_LEVEL_E level,const char *tag,const char *msg, ...)
{
    int mode;
    int iret = 0;
    int init_flag = 0;
    va_list args, args_cpy;
    size_t out_len = 0;
    char * out = NULL;
    const char *ident = "";
    TUYA_LOG_LEVEL_E log_level;

    pthread_mutex_lock(&sg_log_mutex);
    init_flag = sg_log_init_flag;
    pthread_mutex_unlock(&sg_log_mutex);

    if(!init_flag) {
//	printf("%s:init log\n",__func__);
        if(sys_log_init(TUYA_LOG_MODE_CONSOLE,TUYA_LOG_LEVEL_ERROR,NULL)) {
            fprintf(stderr,"log init fail\n");
	    return -1;
        }
    }

    if(tag != NULL  && strlen(tag)) {
        ident = tag;
    }

    pthread_mutex_lock(&sg_log_mutex);
    mode = sg_log_mode;
    log_level = sg_log_level;

    if(strlen(sg_log_filter) && strcmp(sg_log_filter,"xxx") != 0 && 
        strcmp(sg_log_filter,ident) != 0) {
        pthread_mutex_unlock(&sg_log_mutex);
        return 0;
    }
    pthread_mutex_unlock(&sg_log_mutex);

    if(log_level < level) {
        return 0;
    }

    va_start(args,msg);
    va_copy(args_cpy,args);

    out_len = vsnprintf(NULL,0,(const char*)msg, args);
    out = malloc((out_len + 1)*sizeof(char));

    if (out != NULL) {
        vsnprintf(out, (out_len + 1),(const char *)msg, args_cpy);
    }else {
        fprintf(stderr,"malloc fail\n");
        va_end(args);
        va_end(args_cpy);
        return -1;
    }

    if(mode & TUYA_LOG_MODE_CONSOLE) {
        iret |=   __sys_log_msg_console(ident,level,out);
    }
 
    if(mode & TUYA_LOG_MODE_SYSLOG) {
        iret |=  __sys_log_msg_syslog(ident,level,out);
    }

    if(mode & TUYA_LOG_MODE_FILE) {
        iret |= __sys_log_msg_file(ident,level,out);
    }

#ifdef CONFIG_TUYA_SYS_LOG_JOURNALD
    if(mode & TUYA_LOG_MODE_JOURNALD) {
        iret |= __sys_log_msg_journald(ident,level,out) ;
    }
#endif

    if((mode & TUYA_LOG_MODE_CALLBACK) && sg_log_callback) {
        sg_log_callback(level,out);
    }

    va_end(args);
    va_end(args_cpy);
    free(out);

    return 0;
}

int sys_log_deinit(void)
{
    pthread_mutex_lock(&sg_log_mutex);

    if(sg_log_mode & TUYA_LOG_MODE_FILE && sg_log_file_fd) {
	fclose(sg_log_file_fd);
        sg_log_file_fd = NULL;
    }

    sg_log_mode = TUYA_LOG_MODE_NONE;
    sg_log_level = TUYA_LOG_LEVEL_NONE;
    sg_log_callback = NULL;
    sg_log_init_flag = 0;

    pthread_mutex_unlock(&sg_log_mutex);

    return 0;
}
