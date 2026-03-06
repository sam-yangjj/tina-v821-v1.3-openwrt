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
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <dirent.h>
#include "uboot.h"

#define CONFIG_UBOOT_FWENV	"/etc/fw_env.config"

struct env_opts *fw_env_opts = &(struct env_opts) {
	.config_file = (char *)CONFIG_UBOOT_FWENV
};

/*
 * The lockfile is the same as defined in U-Boot for
 * the fw_printenv utilities
 */
static const char *lockname = "/var/lock/fw_printenv.lock";
static int lock_uboot_env(void)
{
	int lockfd = -1;
	lockfd = open(lockname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (lockfd < 0) {
		printf("Error opening U-Boot lock file %s, %s", lockname, strerror(errno));
		return -1;
	}
	if (flock(lockfd, LOCK_EX) < 0) {
		printf("Error locking file %s, %s", lockname, strerror(errno));
		close(lockfd);
		return -1;
	}

	return lockfd;
}

static void unlock_uboot_env(int lock)
{
	flock(lock, LOCK_UN);
	close(lock);
}

int app_env_set(const char *name, const char *value)
{
	int lock = lock_uboot_env();
	int ret;

	if (lock < 0)
		return -1;

	if (fw_env_open (fw_env_opts)) {
		printf("Error: environment not initialized, %s", strerror(errno));
		unlock_uboot_env(lock);
		return -1;
	}
	fw_env_write ((char *)name, (char *)value);
	ret = fw_env_flush(fw_env_opts);
	fw_env_close (fw_env_opts);

	unlock_uboot_env(lock);

	return ret;
}

int app_env_unset(const char *name)
{
	return app_env_set(name, "");
}

char *app_env_get(const char *name)
{
	int lock;
	char *value = NULL;
	char *var;

	lock = lock_uboot_env();
	if (lock < 0)
		return NULL;

	if (fw_env_open (fw_env_opts)) {
		printf("Error: environment not initialized, %s", strerror(errno));
		unlock_uboot_env(lock);
		return NULL;
	}

	var = fw_getenv((char *)name);
	if (var)
		value = strdup(var);

	fw_env_close (fw_env_opts);

	unlock_uboot_env(lock);

	return value;
}

int app_env_apply_list(const char *filename)
{
	int lockfd;
	int ret;

	lockfd = lock_uboot_env();
	if (lockfd < 0) {
		printf("Error opening U-Boot lock file %s, %s", lockname, strerror(errno));
		return -ENODEV;
	}

	ret = fw_parse_script((char *)filename, fw_env_opts);
	fw_env_close (fw_env_opts);
	unlock_uboot_env(lockfd);

	return ret;
}
