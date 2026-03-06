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
#ifndef __MJSON_H
#define __MJSON_H

#define DEFAULT_JSON_PATH "/etc/tinatest.json"

#include <stdio.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/arraylist.h>
#include <json-c/linkhash.h>

#define ARRAY_MAX 30
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum mjson_type {
    mjson_type_null = 0,
    mjson_type_boolean,
    mjson_type_double,
    mjson_type_int,
    mjson_type_object, //user can ignore it
    mjson_type_array,
    mjson_type_string,
    mjson_type_error //no this key or array not string
};

struct mjson_value {
    union mdata {
        int m_boolean;
        double m_double;
        int m_int;
        char *m_string;
        struct {
            int m_array_cnt;
            char *m_array[ARRAY_MAX];
        };
    } val;
    enum mjson_type type;

    char *keyname;
    const char *keypath;
    struct json_object *obj;
};

extern int mjson_load(const char *fpath);
extern void mjson_draw_tree(const char *keypath);
extern enum mjson_type mjson_get_type(const char *keypath);
//fetch family
extern   struct mjson_value mjson_fetch(const char *keypath);
extern                  int mjson_fetch_int(const char *keypath);
extern                  int mjson_fetch_boolean(const char *keypath);
extern               double mjson_fetch_double(const char *keypath);
extern               char * mjson_fetch_string(const char *keypath);
/**
 * array: 子符串指针数组
 * max: 最多保存多少组数组（二维指针长度）
 */
extern                  int mjson_fetch_array(const char *keypath, char *array[], int max);

//this funcion is used for macros mjson_foreach, not user
extern void mjson_get_value_from_obj(json_object *obj, struct mjson_value *val);
//mjson_foreach(char *keypath, char *key, union mdata val, enum mjson_type mtype);
#define mjson_foreach(kpath, key, _val, mtype) \
    char *key __attribute__((__unused__)); \
    union mdata _val __attribute__((__unused__)); \
    enum mjson_type mtype __attribute__((__unused__)); \
    struct mjson_value entry ## _val; \
    struct lh_entry *entry_next ## key, *entry ## key; \
    struct json_object *obj ## key; \
    for (\
            ({ \
                entry_next ## key = NULL; \
                obj ## key = mjson_fetch(kpath).obj; \
                if (obj ## key) \
                    entry ## key = json_object_get_object(obj ## key)->head; \
                else \
                    entry ## key = NULL; \
            }); \
            ({ \
                if (entry ## key) { \
                    key = (char *)entry ## key->k; \
                    mjson_get_value_from_obj((struct json_object *)entry ## key->v, \
                            &entry ## _val); \
                    _val = entry ## _val.val; \
                    mtype = entry ## _val.type; \
                    entry_next ## key = entry ## key->next; \
                }; \
                entry ## key; \
             }); \
            entry ## key = entry_next ## key \
    ) \

#endif
