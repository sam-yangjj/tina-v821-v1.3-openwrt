/*
 * Copyright (C) 2021 Ty Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "tkl_gpio.h"

typedef struct {
    int id;
    int direction;
    int value;
} tkl_gpio_test_para_t;

static tkl_gpio_test_para_t test_para;

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n", prog);
    printf(
"  -i <ID>\n"
"  -d <direction>,  direction=[in, out]  out: output, in: input\n"
"  -v <value>,      value=[high, low]    Only required when the direction is OUT\n"
    );

    printf("e.g. Set GPIO to OUTPUT with HIGH level\n");
    printf("        %s -i 0 -d out -v high\n", prog);
}

static int parse_args(int argc, char **argv, tkl_gpio_test_para_t *para)
{
    int option_index;
    int c;
    const char short_options[] = "i:d:v:h";
    const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* set the default parameters */
    para->id = -1;
    para->direction = -1;
    para->value = -1;

    while ((c = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'h':
            return -1;
        case 'i':   /* id */
            para->id = atoi(optarg);
            break;
        case 'd':   /* direction */
            if (strcasecmp(optarg, "in") == 0) {
                para->direction = TUYA_GPIO_INPUT;
            } else if (strcasecmp(optarg, "out") == 0) {
                para->direction = TUYA_GPIO_OUTPUT;
            } else {
                printf("error: Invalid direction %s\n", optarg);
                return -1;
            }
            break;
        case 'v':   /* value */
            if (strcasecmp(optarg, "high") == 0) {
                para->value = TUYA_GPIO_LEVEL_HIGH;
            } else if (strcasecmp(optarg, "low") == 0) {
                para->value = TUYA_GPIO_LEVEL_LOW;
            } else {
                printf("error: Invalid value %s\n", optarg);
                return -1;
            }
            break;
        default:
            return -1;
        }
    }

    if ((para->id == -1) || (para->direction == -1)) {
        return -1;
    }
    if ((para->direction == TUYA_GPIO_OUTPUT) && (para->value == -1)) {
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    UINT32_T port;
    TUYA_GPIO_BASE_CFG_T cfg;
    TUYA_GPIO_LEVEL_E level;
    OPERATE_RET ret;

    printf("%s %s %s\n", argv[0], __DATE__, __TIME__);

    if (argc == 1) {
        print_usage(argv[0]);
        return -1;
    }

    if (parse_args(argc, argv, &test_para)) {
        print_usage(argv[0]);
        return -1;
    }

    printf("para: num=%d, direction=%d, value=%d\n",
           test_para.id, test_para.direction, test_para.value);

    port = (UINT32_T)test_para.id;
    cfg.direct = (TUYA_GPIO_DRCT_E)test_para.direction;
    cfg.level = (TUYA_GPIO_LEVEL_E)test_para.value;

    ret = tkl_gpio_init(port, &cfg);
    if (ret != OPRT_OK) {
        printf("error: tkl_gpio_init failed\n");
        return -1;
    }

    if (test_para.direction == TUYA_GPIO_INPUT) {
        ret = tkl_gpio_read(port, &level);
        if (ret != OPRT_OK) {
            printf("error: tkl_gpio_read failed\n");
            //return -1;
        } else {
            printf("gpio read: %s\n", (level == TUYA_GPIO_LEVEL_HIGH) ? "High" : "Low" );
        }
    } else {
        level = cfg.level;
        ret = tkl_gpio_write(port, level);
        if (ret != OPRT_OK) {
            printf("error: tkl_gpio_write failed\n");
            //return -1;
        } else {
            printf("gpio write: %s\n", (level == TUYA_GPIO_LEVEL_HIGH) ? "High" : "Low" );
        }
    }

    tkl_gpio_deinit(port);
    return 0;
}
