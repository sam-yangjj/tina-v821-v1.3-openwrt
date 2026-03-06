/*
 * Copyright (C) 2021 Tuya Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include "tkl_watchdog.h"

typedef struct {
    int timeout_ms;     /* watchdog timeout, in milliseconds */
} TKL_WDT_TEST_PARA_T;

static TKL_WDT_TEST_PARA_T sg_test_para;

static void __print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n", prog);
    printf(
"  -t <timeout>     Set watchdog timeout(in ms) and Enable watchdog\n"
    );
}

static int __parse_args(int argc, char **argv, TKL_WDT_TEST_PARA_T *para)
{
    int option_index;
    int c;
    const char short_options[] = "t:h";
    const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* set the default parameters */
    para->timeout_ms = 5000;

    while ((c = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'h':
            return -1;
        case 't':
            para->timeout_ms = atoi(optarg);
            if (para->timeout_ms < 1000) {
                printf("error: timeout (%d ms) too small\n", para->timeout_ms);
                return -1;
            }
            break;
        default:
            return -1;
        }
    }

    return 0;
}

static int sg_flag = 0;
void signal_handle(int sig)
{
    printf("get signal: %d\n", sig);

    switch(sig) {
    case SIGINT:
    case SIGKILL:
    case SIGTERM:
        sg_flag = 1;
        break;

    default:
        break;
    }

    return;
}

int main(int argc, char **argv)
{
    TUYA_WDOG_BASE_CFG_T cfg;
    OPERATE_RET ret;
    UINT_T timeout_ms;
    int func_num = 0;

    printf("%s %s %s\n", argv[0], __DATE__, __TIME__);

    if (argc == 1) {
        __print_usage(argv[0]);
        return -1;
    }

    if (__parse_args(argc, argv, &sg_test_para)) {
        __print_usage(argv[0]);
        return -1;
    }

    signal(SIGINT, signal_handle);

    cfg.interval_ms = sg_test_para.timeout_ms;
    timeout_ms = tkl_watchdog_init(&cfg);
    if (timeout_ms == 0) {
        printf("error: tkl_watchdog_init failed\n");
        return -1;
    }
    printf("timeout: %d ms\n", timeout_ms);

    while (!sg_flag) {
        printf("0   Refresh watchdog\n");
        printf("----\n");
        printf("9   Exit\n");
        printf("Please enter the function number: \n");

        ret = scanf("%d", &func_num);
        if (ret != 1) {
            printf("scanf error %d\n", ret);
        }
        getchar();  // enter key

        switch(func_num) {
        case 0:
            ret = tkl_watchdog_refresh();
            if (ret != OPRT_OK) {
                printf("error: tkl_watchdog_refresh failed\n");
                //return -1;
            }
            break;
        default:
            sg_flag = 1;
            break;
        }
    }

    tkl_watchdog_deinit();
    return 0;
}
