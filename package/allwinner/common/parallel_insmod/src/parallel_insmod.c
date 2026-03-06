/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.

 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.

 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.


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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/lirc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>

#define NUM_THREAD				(4)

static FILE *fp;
static int cur_height, insmoding;
static const char *module_path;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void print_usage(char **argv)
{
	printf("usage: %s depends_file module_path\n", argv[0]);
}

static int get_module(char *name_buf, int len)
{
	static char *line = NULL;
	size_t line_len;

	pthread_mutex_lock(&lock);
again:
	if (getline(&line, &line_len, fp) <= 0) {
		if (line) {
			free(line);
			line = NULL;
		}
		pthread_mutex_unlock(&lock);
		return 0;
	}

	if (!strncmp("Height", line, sizeof("Height") - 1)) {
		pthread_mutex_lock(&mutex);
		while (insmoding)
			pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
		cur_height++;
		goto again;
	}

	if (strlen(line) > 1 && line[0] != '\t') {
		printf("invalid module name: %s\r\n", line);
	}

	pthread_mutex_lock(&mutex);
	insmoding++;
	pthread_mutex_unlock(&mutex);

	snprintf(name_buf, len, "%s/%s", module_path, &line[1]);

	pthread_mutex_unlock(&lock);

	return 1;
}

static void put_module(void)
{
	pthread_mutex_lock(&mutex);
	if (insmoding)
		insmoding--;
	if (insmoding == 0)
		pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

void *insmod_thread(void *arg)
{
	pid_t pid;
	char name[1024];
	int ret;

	while (1) {
		ret = get_module(name, sizeof(name));
		if (ret != 1)
			break;

		pid = vfork();
		if (pid == -1) {
			printf("vfork error");
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			/* child process */
			char *kmod[] = { "/sbin/insmod", name, NULL };

			execvp(kmod[0], kmod);
			exit(EXIT_FAILURE);
		} else {
			wait(NULL);
		}
		put_module();
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int i = 0;
	pthread_t *tid;
	struct timeval start_time, end_time;

	if (argc != 3) {
		print_usage(argv);
		return 0;
	}

	gettimeofday(&start_time, NULL);

	module_path = argv[2];

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("open %s failed: %s\n", argv[1], strerror(errno));
		return 0;
	}

	rewind(fp);

	cur_height = 0;
	insmoding = 0;

	tid = malloc(sizeof(*tid) * NUM_THREAD);
	for (i = 0; i < NUM_THREAD; i++) {
		if (pthread_create(&tid[i], NULL, insmod_thread, NULL) != 0) {
			printf("pthread_create error\r\n");
			exit(0);
		}
	}

	for (i = 0; i < NUM_THREAD; i++)
		pthread_join(tid[i], NULL);

	fclose(fp);
	fp = NULL;
	free(tid);

	gettimeofday(&end_time, NULL);
	fp = fopen("/dev/console", "w");
	if (fp != NULL) {
		fprintf(fp, "async insmod cost: %ld us\r\n",
						1000000 * (end_time.tv_sec - start_time.tv_sec) +
						end_time.tv_usec - start_time.tv_usec);
		fclose(fp);
	}

	return 0;
}
