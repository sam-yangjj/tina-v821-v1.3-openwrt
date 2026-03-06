#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tkl_cmd.h"

int main(int argc,char*argv[]) 
{
    OPERATE_RET iret ;
    int i,n=0;
    char cmd[1024];
    int len = 1024;
    char buffer[1024];

    if(argc <= 1) {
        printf("%s cmd\n",argv[0]);
	return 0;
    }

    memset(cmd,0,1024);
    memset(buffer,0,1024);

    for(i=1;i<argc;i++) {
       n += snprintf(cmd+n,1024-n,"%s ",argv[i]); 
    }
    
    printf("%s\n",cmd);

    iret = tkl_cmd_run(cmd,buffer,&len);

    if(strlen(buffer)) {
        printf("len:%d,%s\n",len,buffer);
    }
    printf("iret = %d\n",iret);

    return 0;
} 
