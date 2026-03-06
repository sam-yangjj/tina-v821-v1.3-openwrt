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
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "filecount"

static struct {
    char *directory;
    bool include_hidden;
    bool include_subdir;
    struct {
        char *name;
        char *size;
        char *mtime;
    }match;
}module;

static int module_scan_json(const char *collectd_path)
{
    int len = 10 * strlen(collectd_path), ret = -1;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed\n", MODULE_NAME);
        goto out;
    }

    // directory
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_DIRECTORY);
    module.directory = mjson_fetch_string(buf);

    // include_hidden
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_INCLUDE_HIDDEN);
    module.include_hidden = mjson_fetch_boolean(buf);

    // include_subdir
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_INCLUDE_SUBDIR);
    module.include_subdir = mjson_fetch_boolean(buf);

    // match-name
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_NAME);
    module.match.name = mjson_fetch_string(buf);

    // match-size
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_SIZE);
    module.match.size = mjson_fetch_string(buf);

    // match-mtime
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_MTIME);
    module.match.mtime = mjson_fetch_string(buf);

    ret = 0;
out:
    free(buf);
    return ret;
}

int module_make_conf(const char *collectd_path, FILE *fp)
{
    DEBUG(BASE, "make config for collectd_%s\n", MODULE_NAME);
    if (module_scan_json(collectd_path) < 0) {
        ERROR("%s: scan json failed\n", MODULE_NAME);
        return -1;
    }

    fprintf(fp, "LoadPlugin %s\n", MODULE_NAME);
    fprintf(fp, "<Plugin %s>\n", MODULE_NAME);
    if (module.directory != NULL)
        fprintf(fp, "\t<Directory \"%s\">\n", module.directory);
    fprintf(fp, "\t\tIncludeHidden %s\n",
            module.include_hidden == true ? "true" : "false");
    fprintf(fp, "\t\tRecursive %s\n",
            module.include_subdir == true ? "true" : "false");
    if (module.match.name != NULL && strcmp(module.match.name,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tName \"%s\"\n", module.match.name);
    if (module.match.size != NULL && strcmp(module.match.size,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tSize %s\n", module.match.size);
    if (module.match.mtime != NULL && strcmp(module.match.mtime,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tMTime %s\n", module.match.mtime);
    if (module.directory != NULL)
        fprintf(fp, "\t</Directory>\n");
    fprintf(fp, "</Plugin>\n");

    return 0;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
