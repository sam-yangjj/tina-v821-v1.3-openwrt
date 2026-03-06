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
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "upgrade_app.h"

#define _ALIGN(num,size) (((num)+(size)-1)&(~((size)-1)))

#define USE_ENV_LIB 1

static int update_partition(const char *from, const char *to)
{
	int fd_to, fd_from, ret = -1, rlen, wlen;

	char *buf = malloc(CHUNK);
	if (buf == NULL) {
		printf("malloc 64K buffer failed\n");
		return -1;
	}

	fd_to = open(to, O_RDWR);
	if (fd_to < 0) {
		printf("open %s failed - %d\n", to, fd_to);
		free(buf);
		return -1;
	}

	fd_from = open(from, O_RDONLY);
	if (fd_from < 0) {
		printf("open %s failed - %d\n", from, fd_from);
		goto close_to;
	}

	while ((rlen = read(fd_from, buf, CHUNK))) {
		if (rlen < 0) {
			printf("read %s failed - %d\n", from, rlen);
			goto close_from;
		}

		wlen = write(fd_to, buf, rlen);
		if (wlen != rlen) {
			printf("write %s failed - %d\n", to, wlen);
			goto close_from;
		}
	}

	ret = 0;
	fsync(fd_to);
close_from:
	close(fd_from);
close_to:
	close(fd_to);
	free(buf);
	return ret;
}

#ifndef USE_ENV_LIB
void executeCMD(const char *cmd, char *result)
{
	char buf_ps[CMD_RESULT_LEN];
	char ps[CMD_LEN] = {0};
	FILE *ptr;
	strcpy(ps, cmd);
	if ((ptr = popen(ps, "r")) != NULL) {
		if (result != NULL) {
			while (fgets(buf_ps, CMD_RESULT_LEN, ptr) != NULL) {
				strcat(result, buf_ps);
				if (strlen(result) > CMD_RESULT_LEN)
					break;
			}
		}
		pclose(ptr);
		ptr = NULL;
	} else {
		printf("popen %s error\n", ps);
	}
}

static void update_env(char next_system)
{
	char cmd[CMD_LEN];

	if (next_system != 0) {
		if (next_system != 'A' && next_system != 'B') {
			printf("set next_system fail, unknow system:%c\n", next_system);
			return ;
		}
	}

	snprintf(cmd, CMD_LEN, "echo boot_partition boot%c > /tmp/env_script", next_system);
	executeCMD(cmd, NULL);

	memset(cmd, '0', CMD_LEN);
	snprintf(cmd, CMD_LEN, "echo root_partition rootfs%c >> /tmp/env_script", next_system);
	executeCMD(cmd, NULL);

	memset(cmd, '0', CMD_LEN);
	snprintf(cmd, CMD_LEN, "echo riscv_partition rv_%c >> /tmp/env_script", next_system);
	executeCMD(cmd, NULL);
	printf("switch system to %c\n", next_system);

	/* update env */
	executeCMD("fw_setenv -s /tmp/env_script", NULL);
	executeCMD("fsync /dev/by-name/*", NULL);

	printf("switch done\n");

}

/* AB system OTA demo */
static int update_system(char next_system)
{
	int ret = -1;
	char kernel_part[PARTITION_PATH_MAX_LEN] = {'0'};
	char rootfs_part[PARTITION_PATH_MAX_LEN] = {'0'};
	char riscv_part[PARTITION_PATH_MAX_LEN] = {'0'};

	snprintf(kernel_part, PARTITION_PATH_MAX_LEN, "/dev/by-name/boot%c", next_system);
	snprintf(rootfs_part, PARTITION_PATH_MAX_LEN, "/dev/by-name/rootfs%c", next_system);
	snprintf(riscv_part, PARTITION_PATH_MAX_LEN, "/dev/by-name/rv_%c", next_system);

	printf("update part:%s\n", kernel_part);
	ret = update_partition(KERNEL_IMG, kernel_part);
	if (ret) {
		printf("update kernel failed\n");
		return -1;
	}

	printf("update part:%s\n", rootfs_part);
	ret = update_partition(ROOTFS_IMG, rootfs_part);
	if (ret) {
		printf("update rootfs failed\n");
		return -1;
	}

	printf("update part:%s\n", riscv_part);
	ret = update_partition(RISCV_IMG, riscv_part);
	if (ret) {
		printf("update riscv failed\n");
		return -1;
	}

	return 0;
}

#else
#define ENV_PARAM_FILE "/tmp/env_param_file"
static int update_env(char next_system)
{
	int ret = -1;
	int fd;
	char rv_buf[256];
	char sys_buf[256];

	if (next_system != 0) {
		if (next_system != 'A' && next_system != 'B') {
			printf("set next_system fail, unknow system:%c\n", next_system);
			return ret;
		}
	}

	if (next_system == 'A') {
		/*next system is A*/
		snprintf(rv_buf, sizeof(rv_buf), "%s %s\n", "riscv_partition", "riscv0");
		snprintf(sys_buf, sizeof(sys_buf), "%s %s\n", "systemAB_now", "A");
	} else if (next_system == 'B') {
		/*next system is B*/
		snprintf(rv_buf, sizeof(rv_buf), "%s %s\n", "riscv_partition", "riscv0-r");
		snprintf(sys_buf, sizeof(sys_buf), "%s %s\n", "systemAB_now", "B");
	}

	fd = open(ENV_PARAM_FILE, O_CREAT | O_WRONLY);
	if (fd < 0) {
		printf("open %s failed - %d\n", ENV_PARAM_FILE, fd);
		return -1;
	}

	if (write(fd, rv_buf, strlen(rv_buf)) != (ssize_t)strlen(rv_buf)) {
		printf("write rv_buf:%s failed\n", rv_buf);
		close(fd);
		return -1;
	}

	if (write(fd, sys_buf, strlen(sys_buf)) != (ssize_t)strlen(sys_buf)) {
		printf("write sys_buf:%s failed\n", sys_buf);
		close(fd);
		return -1;
	}

	close(fd);

	ret = app_env_apply_list(ENV_PARAM_FILE);
	if (ret != 0) {
		printf("set env failed\n");
	}

	return ret;
}

/* riscv OTA demo */
static int update_system(char next_system)
{
	int ret = -1;
	char riscv_part[PARTITION_PATH_MAX_LEN] = {'0'};
	char *riscv_a = "/dev/by-name/riscv0";
	char *riscv_b = "/dev/by-name/riscv0-r";

	if (next_system == 'A') {
		/*next system is A*/
		snprintf(riscv_part, PARTITION_PATH_MAX_LEN, "%s", riscv_a);
	} else if (next_system == 'B') {
		/*next system is B*/
		snprintf(riscv_part, PARTITION_PATH_MAX_LEN, "%s", riscv_b);
	}

	printf("update part:%s\n", riscv_part);
	ret = update_partition(RISCV_IMG, riscv_part);
	if (ret) {
		printf("update riscv failed\n");
		return -1;
	}

	//app_env_set("test", "123456");

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	int ret = -1;
	char next_system;
#ifndef USE_ENV_LIB
	char systemAB_now[CMD_RESULT_LEN] = {0};
	executeCMD("fw_printenv -n boot_partition", systemAB_now);
	if (strncmp(systemAB_now, SYSTEMA, strlen(SYSTEMA)) == 0) {
		next_system = 'B';
	} else if (strncmp(systemAB_now, SYSTEMB, strlen(SYSTEMB)) == 0) {
		next_system = 'A';
	} else {
		printf("error: unknow boot_partition in env:%s\n", systemAB_now);
		return -1;
	}

	ret = update_system(next_system);
	if (ret) {
		return -1;
	}

	update_env(next_system);
#else
	char *system_now = NULL;

	system_now = app_env_get("systemAB_now");
	printf("system_now:%s\n", system_now);

	if (!strcmp(system_now, "A")) {
		/*current system is A, next system should be B*/
		next_system = 'B';
	} else if (!strcmp(system_now, "B")) {
		/*current system is B, next system should be A*/
		next_system = 'A';
	} else {
		printf("error: systemAB_now is:%s, please input A or B in env\n", system_now);
		return -1;
	}
	ret = update_system(next_system);
	if (ret) {
		return -1;
	}

	ret = update_env(next_system);
	if (ret) {
		return -1;
	}
#endif

	return 0;
}
