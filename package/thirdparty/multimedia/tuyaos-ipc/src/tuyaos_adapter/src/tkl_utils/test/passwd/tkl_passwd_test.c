#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tkl_passwd.h"

static void usage(char *name)
{
    printf("%s user passwd algo(0(des),1(md5),2(sha256),3(sha512)\n",name);
}

int main(int argc,char*argv[])
{
   int algo;

   OPERATE_RET iret;

   if(argc < 4) {
       usage(argv[0]);
       return 0;
   }
   
   algo = atoi(argv[3]);

   if(algo < 0 || algo >= 4) {
       usage(argv[0]); 
       return 0;
   }

   iret = tkl_passwd_update((TKL_PWD_ALGO_E)algo,argv[1],argv[2]);

   printf("iret = %d\n",iret);

   return 0;
}

