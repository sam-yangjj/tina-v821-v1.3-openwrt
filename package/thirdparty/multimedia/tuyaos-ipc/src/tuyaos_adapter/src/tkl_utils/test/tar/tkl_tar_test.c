#include <stdio.h>
#include <stdlib.h>
#include "tkl_tar.h"

int main(int argc,char *argv[])
{
    if( argc >= 3 && strcmp("-x",argv[1]) == 0) {
         if(argc == 3) {
             tkl_tar_unpack(argv[2],NULL);
         } else {
             tkl_tar_unpack(argv[2],argv[3]);
         }
     }else if(argc >= 4 &&  strcmp("-c",argv[1]) == 0) {
         tkl_tar_pack(argv[2],(const char **)&argv[3],(const size_t)(argc-3));
     }else {
         printf("./%s -c/-x pack_file  pack files/unpack pach \n",argv[0]);
     }

     return 0;
}
