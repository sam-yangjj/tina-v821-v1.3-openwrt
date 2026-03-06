#include <stdio.h>
#include "tkl_reboot.h"


int main(int argc,char *argv[]) 
{
    printf("reboot system ....\n");
    tkl_reboot(1);	
    return 0;	
}
