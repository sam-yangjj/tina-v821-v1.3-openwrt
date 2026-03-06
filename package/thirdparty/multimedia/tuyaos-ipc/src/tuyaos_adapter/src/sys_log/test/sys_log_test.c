#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "sys_log.h"

#define LOG_TAG1 "log_tag1"
#define LOG_TAG2 "log_tag2"
#define LOG_TAG3 "log_tag3"
#define LOG_TAG4 "log_tag4"

#define ASSERT_EXPECT(x) do {  \
    if(x) {                    \
        printf("%s:%d %s fail\n",__func__,__LINE__,#x); \
        return -1;                                      \
    }}while(0)

static int sys_log_test(TUYA_LOG_LEVEL_E type,const char *msg)
{
    printf("(cb)type:%d,msg:%s\n",type,msg);
    return 0;
}

static int log_single_thread_test(void) 
{
    int cnts = 0;
    int iret;

    printf("single thread log test\n");

    ASSERT_EXPECT(sys_log_init(TUYA_LOG_MODE_CONSOLE,TUYA_LOG_LEVEL_ALL,NULL));

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,LOG_TAG1,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,LOG_TAG1,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,LOG_TAG1,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,LOG_TAG1,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,LOG_TAG1,"hello word:%d\n",cnts ++);
    sys_log_deinit();

    ASSERT_EXPECT(sys_log_init(TUYA_LOG_MODE_CONSOLE | TUYA_LOG_MODE_FILE |TUYA_LOG_MODE_CALLBACK,TUYA_LOG_LEVEL_ALL,"/tmp/test.txt"));

    sys_log_set_callback(sys_log_test);

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,LOG_TAG2,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,LOG_TAG2,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,LOG_TAG2,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,LOG_TAG2,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,LOG_TAG2,"hello word:%d\n",cnts ++);
    sys_log_deinit();

    sys_log_set_callback(NULL);

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,LOG_TAG3,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,LOG_TAG3,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,LOG_TAG3,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,LOG_TAG3,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,LOG_TAG3,"hello word:%d\n",cnts ++);


    printf("LOG Test\n");
    sys_log_deinit();

    ASSERT_EXPECT(sys_log_init(TUYA_LOG_MODE_CONSOLE | TUYA_LOG_MODE_FILE |TUYA_LOG_MODE_CALLBACK,TUYA_LOG_LEVEL_ALL,"/tmp/test.txt"));

    LOGD(LOG_TAG4,"hello word:%d\n",cnts ++);
    LOGI(LOG_TAG4,"hello word:%d\n",cnts ++);
    LOGW(LOG_TAG4,"hello word:%d\n",cnts ++);
    LOGE(LOG_TAG4,"hello word:%d\n",cnts ++);
    LOG(TUYA_LOG_LEVEL_ALL,LOG_TAG4,"hello word:%d\n",cnts ++);
    sys_log_deinit();
	
    return 0;
}

static  int __log_test(int index)
{
    int iret;
    char tag[32] = {0};
    int cnts = 0;

    snprintf(tag,32,"tag%d",index);
    ASSERT_EXPECT(sys_log_init(TUYA_LOG_MODE_CONSOLE,TUYA_LOG_LEVEL_ALL,NULL));

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,tag,"hello word:%d\n",cnts ++);
    sys_log_deinit();

    ASSERT_EXPECT(sys_log_init(TUYA_LOG_MODE_CONSOLE | TUYA_LOG_MODE_FILE |TUYA_LOG_MODE_CALLBACK,TUYA_LOG_LEVEL_ALL,"/tmp/test.txt"));

    sys_log_set_callback(sys_log_test);

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,tag,"hello word:%d\n",cnts ++);
    sys_log_deinit();

    sys_log_set_callback(NULL);

    sys_log_msg(TUYA_LOG_LEVEL_ERROR,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_WARNING,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_INFO,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_DEBUG,tag,"hello word:%d\n",cnts ++);
    sys_log_msg(TUYA_LOG_LEVEL_ALL,tag,"hello word:%d\n",cnts ++);

    return 0;
}


static int log_multi_process_test(int counts) 
{
   int i;

   for(i=0;i<counts;i++) {
      int fd  = fork();

      if(fd == 0) {
          __log_test(i);
          exit(0);  
      }
   }
}

static void* __log_thread (void *arg) 
{
    int index = (int)arg;

    printf("thread %d run \n",index);

    pthread_detach(pthread_self());

    while(1) {

       __log_test(index);
    
       sleep(1);
    } 

    return NULL;
}

static int log_multi_thread_test(int counts)
{
   int i;
   pthread_t thread;

   for(i=0;i<counts;i++) {
       pthread_create(&thread,NULL,__log_thread,(void*)i);
   }

   while(1) {
       printf("wait ....\n");
       sleep(5);
   }
}

int main(int argc,char *argv[]) 
{
  log_single_thread_test(); 
  log_multi_process_test(32); 
  log_multi_thread_test(32);

  return 0;
}
