/*
 * Copyright (C) 2021 Ty Inc.
 */
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "sys_log.h"
#include "tkl_file.h"
#include "tkl_gpio.h"

#ifndef TAG 
#define TAG "tkl_gpio"
#endif

#ifndef MAX_PATH
#define MAX_PATH    256
#endif

#define GPIO_EXPORT_PATH        "/sys/class/gpio/export"

#ifdef GPIO_UPPERCASE
#define GPIO_PATH               "/sys/class/gpio/GPIO%d"
#define GPIO_DIRECTION_PATH     "/sys/class/gpio/GPIO%d/direction"
#define GPIO_VALUE_PATH         "/sys/class/gpio/GPIO%d/value"
#define GPIO_EDGE_PATH          "/sys/class/gpio/GPIO%d/edge"
#else
#define GPIO_PATH               "/sys/class/gpio/gpio%d"
#define GPIO_DIRECTION_PATH     "/sys/class/gpio/gpio%d/direction"
#define GPIO_VALUE_PATH         "/sys/class/gpio/gpio%d/value"
#define GPIO_EDGE_PATH          "/sys/class/gpio/gpio%d/edge"
#endif

#define GPIO_DIRECTION_IN       "in"
/* Writing as "out" defaults to initializing the value as low */
#define GPIO_DIRECTION_OUT      "out"
/* Writing as "high" to configure the GPIO as an output with high value */
#define GPIO_DIRECTION_OUT_HIGH "high"

#define GPIO_VALUE_STR_HIGH     "1"
#define GPIO_VALUE_STR_LOW      "0"
/*
#define GPIO_EDGE_STR_RISING    "rising"
#define GPIO_EDGE_STR_FALLING   "falling"
#define GPIO_EDGE_STR_BOTH      "both"
*/

/* gpio value */
#define GPIO_HIGH   1
#define GPIO_LOW    0

/* gpio direction */
#define GPIO_OUT    0
#define GPIO_IN     1

/* gpio attribute */
typedef struct {
    int direction;              /* GPIO_OUT or GPIO_IN */
    int value;                  /* initial level, only for OUTPUT */
} GPIO_ATTR_T;

static int __tkl_gpio_config(int num, GPIO_ATTR_T *attr)
{
    char gpio_path[MAX_PATH] = {0};
    char direction_path[MAX_PATH] = {0};
    char value_path[MAX_PATH] = {0};
    //char edge_path[MAX_PATH] = {0};
    char gpio_num[MAX_PATH] = {0};
    int ret = 0;

    if (NULL == attr) {
        LOGE(TAG, "invalid gpio attr\n");
        return -1;
    }

    snprintf(gpio_num,MAX_PATH, "%d", num);
    snprintf(gpio_path,MAX_PATH,GPIO_PATH, num);
    snprintf(direction_path,MAX_PATH, GPIO_DIRECTION_PATH, num);
    snprintf(value_path,MAX_PATH,GPIO_VALUE_PATH,num);

    /* export gpio */
    if (! tkl_file_exists(gpio_path)) {
        ret = tkl_file_write(GPIO_EXPORT_PATH, gpio_num, strlen(gpio_num));
        if (ret < 0) {
            LOGE(TAG,"Failed to export gpio(%d)\n", num);
            return -1;
        }
    }

    /* set gpio direction */
    if (tkl_file_exists(direction_path)) {
        if (attr->direction == GPIO_OUT) {
            if (attr->value == GPIO_HIGH) {
                ret = tkl_file_write(direction_path, GPIO_DIRECTION_OUT_HIGH,
                                    strlen(GPIO_DIRECTION_OUT_HIGH));
            } else {
                ret = tkl_file_write(direction_path, GPIO_DIRECTION_OUT,
                                    strlen(GPIO_DIRECTION_OUT));
            }
        } else {
            ret = tkl_file_write(direction_path, GPIO_DIRECTION_IN,
                                strlen(GPIO_DIRECTION_IN));
        }

        if (ret < 0) {
            LOGE(TAG, "Failed to set gpio(%d) direction\n", num);
            return -1;
        }
    } else {
        LOGE(TAG,"kernel doesn't support changing the direction of gpio(%d)\n", num);
    }

    return 0;
}

OPERATE_RET tkl_gpio_init(TUYA_GPIO_NUM_E port_id, CONST TUYA_GPIO_BASE_CFG_T *cfg)
{
    int num = (int)port_id;
    GPIO_ATTR_T attr = {0};

    if (NULL == cfg) {
        return OPRT_INVALID_PARM;
    }

    if (cfg->direct == TUYA_GPIO_INPUT) {
        attr.direction = GPIO_IN;
    } else {
        attr.direction = GPIO_OUT;
        if (cfg->level == TUYA_GPIO_LEVEL_LOW) {
            attr.value = GPIO_LOW;
        } else {
            attr.value = GPIO_HIGH;
        }
    }

    if (__tkl_gpio_config(num, &attr) != 0) {
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_gpio_deinit(TUYA_GPIO_NUM_E port_id)
{
    return OPRT_OK;
}

OPERATE_RET tkl_gpio_write(TUYA_GPIO_NUM_E port_id, TUYA_GPIO_LEVEL_E level)
{
    int value;
    int ret;
    int num = (int)port_id;
    char value_path[MAX_PATH] = {0};

    if (level == TUYA_GPIO_LEVEL_LOW) {
        value = GPIO_LOW;
    } else {
        value = GPIO_HIGH;
    }

    snprintf(value_path,MAX_PATH,GPIO_VALUE_PATH, num);

    if (value == GPIO_HIGH) {
        ret = tkl_file_write(value_path,
                             GPIO_VALUE_STR_HIGH, strlen(GPIO_VALUE_STR_HIGH));
    } else {
        ret = tkl_file_write(value_path,
                             GPIO_VALUE_STR_LOW, strlen(GPIO_VALUE_STR_LOW));
    }

    if (ret < 0) {
        LOGE(TAG, "Failed to set gpio(%d) level\n", num);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_gpio_read(TUYA_GPIO_NUM_E port_id,TUYA_GPIO_LEVEL_E *level)
{
    //int value = 0;
    int num = (int)port_id;
    char buf[32] = {0};
    char value_path[MAX_PATH] = {0};

    snprintf(value_path,MAX_PATH,GPIO_VALUE_PATH, num);

    if (tkl_file_read(value_path, buf, sizeof(buf)) <= 0) {
        LOGE(TAG, "Failed to get gpio(%d) level\n", num);
        return -1;
    }

    if (strncmp(buf, GPIO_VALUE_STR_LOW, strlen(GPIO_VALUE_STR_LOW)) == 0) {
        *level = TUYA_GPIO_LEVEL_LOW;
    } else {
        *level = TUYA_GPIO_LEVEL_HIGH;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_gpio_irq_init(TUYA_GPIO_NUM_E port_id, CONST TUYA_GPIO_IRQ_T *cfg)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_gpio_irq_enable(TUYA_GPIO_NUM_E port_id)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_gpio_irq_disable(TUYA_GPIO_NUM_E  port_id)
{
    return OPRT_NOT_SUPPORTED;
}
