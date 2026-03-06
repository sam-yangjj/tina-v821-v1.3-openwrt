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
// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "rwcheck.h"

struct progress_bar_t{
	unsigned short skip_cnt;
	unsigned char last_percent;
};

void progressBar(long long cur_size, long long total_size, unsigned long long *_data, unsigned int skip)
{
	struct progress_bar_t *data = (struct progress_bar_t*)_data;
	float percent = (float) cur_size / total_size;
	unsigned char diff = (unsigned char)(percent*100) - data->last_percent;

	if( 0!=data->skip_cnt && data->skip_cnt<skip && diff<10 && percent!=1.0f ){
		data->skip_cnt++;
		return;
	}

	data->skip_cnt = 1;
	data->last_percent = (unsigned char)(percent*100);

	const int numTotal = 50;
	int numShow = (int)(numTotal * percent);

	if (numShow == 0)
		numShow = 1;

	if (numShow > numTotal)
		numShow = numTotal;

	char sign[51] = {0};
	memset(sign, '=', numTotal);

	printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB",
			percent * 100,
			numTotal,
			numShow,
			sign,
			cur_size / 1024.0 / 1024.0,
			total_size / 1024.0 / 1024.0);
	fflush(stdout);

	if (numShow == numTotal)
		printf("\n100.00%%\n");
}

bool is_existed(const char *path)
{
	return !access(path, F_OK | R_OK);
}

bool is_dir(const char *path)
{
	int ret;
	struct stat s;

	ret = stat(path, &s);
	if (ret)
		return false;
	return S_ISDIR(s.st_mode);
}

bool is_file(const char *path)
{
	int ret;
	struct stat s;

	ret = stat(path, &s);
	if (ret)
		return false;
	return S_ISREG(s.st_mode);
}

#define PROG_BAR_SIZE (100 * MB)
int copy_file(const char *in, const char *out, unsigned long long size,
		unsigned long long buf_size)
{
	char *buf;
	int fd_in, fd_out;
	int ret = -1;
	unsigned long long total_size = size;
	unsigned long long tmp_data = 0;

	buf = malloc(buf_size);
	if (!buf) {
		ERROR("malloc failed\n");
		return -1;
	}

	fd_in = open(in, O_RDONLY);
	if (fd_in < 0) {
		ERROR("open %s failed\n", in);
		goto free_buf;
	}
	fd_out = open(out, O_LARGEFILE | O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd_out < 0) {
		ERROR("open %s failed\n", out);
		goto close_in;
	}
	/* pre-allocate flash space, in case of no enough space while writing */
	if (ftruncate(fd_out, size)) {
		ERROR("ftruncate %s to %llu failed\n", out, size);
		goto close_out;
	}

	while (size > 0) {
		unsigned long long sz = min(size, buf_size);
		unsigned long long done = 0;

		while (sz > done) {
			int rret;

			rret = read(fd_in, buf + done, sz - done);
			if (rret < 0) {
				ERROR("read %s failed\n", in);
				goto close_out;
			}
			/* lseek to start of file to ensure read full size */
			if (rret != sz - done){
				long long off64 = lseek(fd_in, 0, SEEK_SET);
				if(off64<0){
					ERROR("lseek failed, return %lld\n", off64);
					ret = (int)off64;
					goto close_out;
				}
			}
			done += rret;
		}
		done = write(fd_out, buf, sz);
		if (done != sz) {
			ERROR("write %s failed\n", out);
			goto close_out;
		}
		size -= sz;
		if(total_size>PROG_BAR_SIZE)
			progressBar(total_size-size, total_size, &tmp_data, 5);
	}

	fsync(fd_out);
	ret = 0;
close_out:
	close(fd_out);
close_in:
	close(fd_in);
free_buf:
	free(buf);
	return ret;
}

int append_crc(const char *path)
{
	int fd, ret = -1;
	long long int fsize;
	unsigned short crc = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		ERROR("open %s failed\n", path);
		goto err;
	}

	fsize = file_size(fd);
	if (fsize < 0)
		goto err;

	ret = get_crc16_by_fd(&crc, fd, fsize - 2);
	if (ret) {
		ERROR("get crc16 failed\n");
		goto err;
	}

	{
		long long off64 = lseek(fd, -2, SEEK_END);
		if(off64<0){
			ERROR("lseek %s failed, return %lld\n", path, off64);
			ret = (int)off64;
			goto err;
		}
	}

	ret = write(fd, &crc, 2);
	if (ret != 2) {
		ERROR("write %s failed\n", path);
		goto err;
	}

	fsync(fd);
	ret = 0;
err:
	close(fd);
	return ret;
}

char *abs_path(const char *path)
{
	char *ret = NULL, *pwd;

	pwd = getcwd(NULL, 0);
	if (!pwd)
		return NULL;

	if (chdir(path) < 0) {
		ERROR("chdir failed\n");
		goto out;
	}
	ret = getcwd(NULL, 0);

	if (chdir(pwd) < 0) {
		free(ret);
		ret = NULL;
	}

out:
	free(pwd);
	return ret;
}

long long int file_size(int fd)
{
	struct stat s;

	if (fstat(fd, &s) < 0) {
		ERROR("fstat failed\n");
		return -1;
	}
	return s.st_size;
}
