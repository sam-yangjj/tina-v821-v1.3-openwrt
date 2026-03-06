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
#include <sysexits.h>
#include <sys/time.h>
#include "audiotest.h"

static void print_help_msg()
{
    printf("USAGE:\n");
    printf("\taudiotest_wav_header <wav_file>\n");
    printf("DESCRIPTION:\n");
    printf("\tparse and print WAVE file header\n");
    printf("EXIT CODE:\n");
    printf("\t0         : success\n");
    printf("\t66        : cannot open input file\n");
    printf("\totherwise : error\n");
}

int main(int argc, char *argv[])
{
    int ret;
    if (argc <= 1) {
        print_help_msg();
        ret = EXIT_FAILURE;
        goto out;
    }

    char *wav_file = argv[1];
    FILE *fp = fopen(wav_file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open \"%s\"\n", wav_file);
        ret = EX_NOINPUT;
        goto out;
    }

    struct audiotest_wav_pcm_header header;
    if (0 != audiotest_wav_pcm_header_read(&header, fp)) {
        ret = EXIT_FAILURE;
        goto close_file;
    }
    audiotest_wav_pcm_header_print(&header);

    ret = EXIT_SUCCESS;

close_file:
    fclose(fp);
out:
    return ret;
}
