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
#include <mp3player.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
/*
 * argv[1]: Mp3 file path
 * argv[2]: Speaker Function only for sunxicodec
 * argv[3]: headphone volume control volume only for sunxicodec
 */
static void get_file_type(struct stat *st, char *type, int len)
{
    switch (st->st_mode & S_IFMT) {
    case S_IFBLK:
        strncpy(type, "block device", len);
        break;
    case S_IFCHR:
        strncpy(type, "character device", len);
        break;
    case S_IFDIR:
        strncpy(type, "directory", len);
        break;
    case S_IFIFO:
        strncpy(type, "FIFO/pipe", len);
        break;
    case S_IFLNK:
        strncpy(type, "symlink", len);
        break;
    default:
        strncpy(type, "unknown file", len);
    }
}

static int filldata(char *buf, int size, void *user)
{
    return read((int)user, buf, size);
}
static off_t seekdata(off_t offset, int whence, void *user)
{
    return lseek((int)user, offset, whence);
}

tiny_mp3_ops_t ops = {
    .read = filldata,
    .lseek = seekdata,
};
struct thread_args
{
    tinymp3_ctx_t *ctx;
    const char *file;
};

void *thread1(void *junk)
{
    struct thread_args *args = (struct thread_args *)junk;
    tinymp3_ctx_play(args->ctx);
}

pthread_t create_thread(tinymp3_ctx_t *ctx, const char *file)
{
    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->ctx = ctx;
    args->file = file;
    pthread_t tid;
    pthread_create(&tid, NULL, thread1, (void *)args);
    return tid;
}

void test_set_volume(tinymp3_ctx_t *ctx, const char *file)
{
    pthread_t tid = create_thread(ctx, file);
    int volume = 0;
    while(volume <= 40){
        tinymp3_ctx_setvolume(ctx, volume++);
        usleep(50*1000);
    }
    pthread_join(tid, NULL); //join thread
}

void test_stop(tinymp3_ctx_t *ctx, const char *file)
{
    tinymp3_ctx_setvolume(ctx, 10);
    pthread_t tid = create_thread(ctx, file);

    usleep(1500*1000);

    tinymp3_ctx_play(ctx); // fail, return -1
    tinymp3_ctx_destroy(&ctx); // fail, return -1
    tinymp3_ctx_stop(ctx); //block for wait play finish
    tinymp3_ctx_play(ctx); //play new

    pthread_join(tid, NULL); //join thread
}

int main(int argc, char *argv[])
{
    char *mp3_file = argv[1];
    struct stat stat;

    printf("create\n");
    tinymp3_ctx_t *ctx = tinymp3_ctx_create(NULL, NULL);
    printf("play\n");

    tinymp3_ctx_prepare_file(ctx, mp3_file);
    tinymp3_ctx_play_file(ctx);

    tinymp3_ctx_stop(ctx);
    tinymp3_ctx_destroy(&ctx);
    /*
    tinymp3_ctx_t *ctx = tinymp3_ctx_create();

    test_set_volume(ctx, argv[1]);
    test_stop(ctx, argv[1]);

    tinymp3_ctx_destroy(&ctx);
    */
	return 0;
}
