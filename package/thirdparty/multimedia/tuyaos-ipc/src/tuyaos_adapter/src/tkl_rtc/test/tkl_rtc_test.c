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
#include <time.h>
#include "tkl_rtc.h"

typedef enum {
    TKL_RTC_TEST_OPS_GET_TIME,
    TKL_RTC_TEST_OPS_SET_TIME
} TKL_RTC_TEST_E;

typedef struct {
    TKL_RTC_TEST_E ops;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} TKL_RTC_TEST_PARA_T;

static TKL_RTC_TEST_PARA_T sg_test_para;

static void __print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n", prog);
    printf(
"  -g           Get rtc time(Local time)\n"
"  -s <string>  Set rtc time(Local time). <string> format: 'YYYY-MM-DD hh:mm:ss'\n"
    );

    printf("e.g. Get rtc time\n");
    printf("        %s\n", prog);
    printf("e.g. Set the rtc time to 2021-01-01 00:00:00\n");
    printf("        %s -s '2021-01-01 00:00:00'\n", prog);
}

static int __parse_args(int argc, char **argv, TKL_RTC_TEST_PARA_T *para)
{
    int option_index;
    int c;
    const char short_options[] = "gs:h";
    const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* set the default parameters */
    para->ops = TKL_RTC_TEST_OPS_GET_TIME;
    // default: 2021-01-01 00:00:00
    para->year = 2021;
    para->month = 1;
    para->day = 1;
    para->hour = 0;
    para->minute = 0;
    para->second = 0;

    while ((c = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'g':
            para->ops = TKL_RTC_TEST_OPS_GET_TIME;
            break;
        case 's':
            para->ops = TKL_RTC_TEST_OPS_SET_TIME;
            sscanf(optarg, "%d-%d-%d %d:%d:%d",
                &para->year, &para->month, &para->day,
                &para->hour, &para->minute, &para->second);
            break;
        default:
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    TIME_T tkl_timestamp;
    time_t unix_time;
    struct tm tm_local = {0};
    OPERATE_RET ret;

    // printf("%s ver: %s %s\n", argv[0], __DATE__, __TIME__);

    if (__parse_args(argc, argv, &sg_test_para)) {
        __print_usage(argv[0]);
        return -1;
    }

    // init
    if (tkl_rtc_init() != OPRT_OK) {
        printf("error: tkl_rtc_init failed\n");
        return -1;
    }

    // get/set rtc time
    if (sg_test_para.ops == TKL_RTC_TEST_OPS_GET_TIME) {
        ret = tkl_rtc_time_get(&tkl_timestamp);
        if (ret != OPRT_OK) {
            printf("error: tkl_rtc_time_get failed\n");
            tkl_rtc_deinit();
            return -1;
        }

        unix_time = (time_t)tkl_timestamp;
        localtime_r(&unix_time, &tm_local);

        printf("timestamp %d\n", tkl_timestamp);
        printf("local time: %04d-%02d-%02d %02d:%02d:%02d\n",
               tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday,
               tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
    } else {
        printf("para: %d-%d-%d %d:%d:%d\n",
               sg_test_para.year, sg_test_para.month, sg_test_para.day,
               sg_test_para.hour, sg_test_para.minute, sg_test_para.second);
        //
        tm_local.tm_year = sg_test_para.year - 1900;
        tm_local.tm_mon = sg_test_para.month - 1;
        tm_local.tm_mday = sg_test_para.day;
        tm_local.tm_hour = sg_test_para.hour;
        tm_local.tm_min = sg_test_para.minute;
        tm_local.tm_sec = sg_test_para.second;
        //
        unix_time = timelocal(&tm_local);
        tkl_timestamp = (TIME_T)unix_time;
        ret = tkl_rtc_time_set(tkl_timestamp);
        if (ret != OPRT_OK) {
            printf("error: tkl_rtc_time_set failed\n");
            tkl_rtc_deinit();
            return -1;
        }
    }

    tkl_rtc_deinit();
    return 0;
}
