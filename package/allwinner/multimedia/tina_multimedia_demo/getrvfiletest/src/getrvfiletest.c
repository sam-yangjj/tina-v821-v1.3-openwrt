/*************************************************************************
    > File Name: geth264test.c
    > Author:
    > Mail:
    > Created Time: 2017/10/15 14:20:17
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
//#include <linux/io.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#define OUTPUT_FILE_MEMRESERVE 0x83600000 //out buf begin
//#define OUTPUT_FILE_SIZE 1896962
#define OUTPUT_FILE_SIZE 45213 //320x240 jpg

int main(int argc, char **argv)
{
	void *rv_h264_vaddr;
        FILE* fpStream   = NULL;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd == -1)
	{
		printf("fd error\n");
		return -1;
	}

	rv_h264_vaddr = mmap(NULL, OUTPUT_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OUTPUT_FILE_MEMRESERVE);

	if(rv_h264_vaddr == MAP_FAILED)
	{
		printf("map error, check OUTPUT_FILE_MEMRESERVE\n");
		return -1;
	}


        //fpStream = fopen("/mnt/UDISK/test.h264", "wb");
        fpStream = fopen("/tmp/testout.jpg", "wb");

	if(fpStream != NULL)
        {
            fwrite(rv_h264_vaddr,1,OUTPUT_FILE_SIZE, fpStream);
            fclose(fpStream);
	    printf("get h264 stream, outpath: /tmp/testout.jpg\n");
        }
        else
        {
            printf("++the fpStream is null\n");
        }
}
