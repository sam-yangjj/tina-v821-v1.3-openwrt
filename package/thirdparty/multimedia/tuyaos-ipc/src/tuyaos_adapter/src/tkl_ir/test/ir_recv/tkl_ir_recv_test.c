#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "tkl_ir.h"

int main(int argc,char* argv[] )
{
    int i = 0;
    int size = 2048;
    OPERATE_RET ret = 0;
    TY_IR_BUF_T code = {0};
    unsigned int duration = 0;
    
    code.buf = malloc(sizeof(int)*size);

    if(code.buf == NULL) {
        printf("maill fail\n"); 
	return -1;
    }

    code.cnts = 0;
    code.size = size;
    memset(code.buf,0,sizeof(int)*size);
    
    printf("IR rx version 1.0 \n");

    if((ret = tkl_ir_init(IR_ID_0)) != 0) {
       printf("tkl_ir_init = %d\n",ret);
       return -1;
    }

    ret = tkl_ir_recv(IR_ID_0,&code,10000); // 10s

    printf("tkl_ir_receive ret = %d\n",ret);

    if(ret < 0)
    	return -1;
    
    for(i = 0; i < code.cnts; i++){
    	duration += code.buf[i];
    }
    
    printf("code count:%d time:%ld\n", code.cnts, duration);
    
    for(i = 0; i < code.cnts; i++){
    	printf("%u ",code.buf[i]);
    	if((i % 10 == 0) && i >= 10)
    		printf("\n");
    }
    printf("\n");
    
    free(code.buf);

    tkl_ir_deinit(IR_ID_0);

    return 0;
}
