/**
 * @file tkl_system.c
 * @brief the default weak implements of tuya os system api, this implement only used when OS=linux
 * @version 0.1
 * @date 2019-08-15
 *
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_system.h"
#include "tkl_memory.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/reboot.h>

/**
* @brief Get system ticket count
*
* @param VOID
*
* @note This API is used to get system ticket count.
*
* @return system ticket count
*/
TUYA_WEAK_ATTRIBUTE SYS_TICK_T tkl_system_get_tick_count(VOID_T)
{
    struct timespec time1 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time1);
    return 1000*((ULONG_T)time1.tv_sec) + ((ULONG_T)time1.tv_nsec)/1000000;
}

/**
* @brief Get system millisecond
*
* @param none
*
* @return system millisecond
*/
TUYA_WEAK_ATTRIBUTE SYS_TIME_T tkl_system_get_millisecond(VOID_T)
{
    struct timespec time1 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time1);
    return 1000*((ULONG_T)time1.tv_sec) + ((ULONG_T)time1.tv_nsec)/1000000;
}

/**
* @brief System sleep
*
* @param[in] msTime: time in MS
*
* @note This API is used for system sleep.
*
* @return VOID
*/
TUYA_WEAK_ATTRIBUTE VOID_T tkl_system_sleep(CONST UINT_T num_ms)
{
    TIME_S sTmpTime;
    TIME_MS msTmpTime;

    sTmpTime = num_ms / 1000;
    msTmpTime = num_ms % 1000;

    if(sTmpTime)
        sleep(sTmpTime);

    if(msTmpTime)
        usleep(msTmpTime*1000);

    return;
}


/**
* @brief System reset
*
* @param VOID
*
* @note This API is used for system reset.
*
* @return VOID
*/
TUYA_WEAK_ATTRIBUTE VOID_T tkl_system_reset(VOID_T)
{
    return;
}

/**
* @brief System reboot
*
* @param VOID
*
* @note This API is used for system reboot.
*
* @return VOID
*/
TUYA_WEAK_ATTRIBUTE VOID_T tkl_system_reboot(VOID_T)
{

    printf("start reboot\n");
    sync();
    reboot(RB_AUTOBOOT);
    return;
}



/**
* @brief Get free heap size
*
* @param VOID
*
* @note This API is used for getting free heap size.
*
* @return size of free heap
*/
TUYA_WEAK_ATTRIBUTE INT_T tkl_system_get_free_heap_size(VOID_T)
{
    return -1;
}

/**
* @brief Get system reset reason
*
* @param VOID
*
* @note This API is used for getting system reset reason.
*
* @return reset reason of system
*/
TUYA_RESET_REASON_E tkl_system_get_reset_reason(CHAR_T** describe)
{
    return TUYA_RESET_REASON_UNKNOWN;
}

/**
* @brief Get a random number in the specified range
*
* @param[in] range: range
*
* @note This API is used for getting a random number in the specified range
*
* @return a random number in the specified range
*/
TUYA_WEAK_ATTRIBUTE INT_T tkl_system_get_random(CONST UINT_T range)
{
    return rand() % range;
}


/**
 * @brief system enter critical
 *
 * @param[in]   none
 * @return  irq mask
 */
UINT_T tkl_system_enter_critical(VOID_T)
{
    return OPRT_OK;
}

/**
 * @brief system exit critical
 *
 * @param[in]   irq_mask: irq mask 
 * @return  none
 */
VOID_T tkl_system_exit_critical(UINT_T irq_mask)
{
    return;
}
/**
* @brief system delay
*
* @param[in] msTime: time in MS
*
* @note This API is used for system sleep.
*
* @return VOID
*/
VOID_T tkl_system_delay(UINT_T num_ms)
{
    return; 
}

typedef struct cpu_occupy
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} CPU_OCCUPY_T;

static int get_cpuoccupy (CPU_OCCUPY_T *cpust, int nums)
{
    int i;
    int ret = -1;
    FILE *fd;
    char buff[256];
    CPU_OCCUPY_T *cpu_all_occupy = &cpust[nums - 1];

    fd = fopen ("/proc/stat", "r");
    if(fd == NULL) {
        fprintf(stderr, "err:open /proc/stat\n");
        goto func_quit;
    }

    if(fgets(buff, sizeof(buff), fd) == NULL) {
        fprintf(stderr, "err:fgets\n");
        goto func_quit; 
    }

    ret = sscanf(buff, "%s %u %u %u %u %u %u %u",
                        cpu_all_occupy->name, &cpu_all_occupy->user,
                        &cpu_all_occupy->nice, &cpu_all_occupy->system,
                        &cpu_all_occupy->idle, &cpu_all_occupy->iowait,
                        &cpu_all_occupy->irq, &cpu_all_occupy->softirq);
    if (ret <= 0) {
        fprintf(stderr, "err:sscanf\n"); 
        goto func_quit;
    }

    for(i = 0; i < (nums - 1); i++) {
        CPU_OCCUPY_T *cpu_per_occupy = &cpust[i];
        memset(buff, 0, sizeof(buff));
        if(fgets(buff, sizeof(buff), fd) == NULL) {
            fprintf(stderr, "err: fgets\n");
            goto func_quit;
        }

        ret = sscanf (buff, "%s %u %u %u %u %u %u %u",
                            cpu_per_occupy->name, &cpu_per_occupy->user,
                            &cpu_per_occupy->nice,&cpu_per_occupy->system,
                            &cpu_per_occupy->idle ,&cpu_per_occupy->iowait,
                            &cpu_per_occupy->irq,&cpu_per_occupy->softirq);
        if (ret <= 0) {
            fprintf(stderr, "err:sscanf\n"); 
            goto func_quit;
        }
    }

    ret = 0;

func_quit:
    if (fd != NULL) fclose(fd);
    return ret;
}

static int cal_cpuoccupy(CPU_OCCUPY_T *o, CPU_OCCUPY_T *n, TUYA_CPU_INFO_T *cpu_info, int cpu_nums)
{
    double od, nd;
    double id, sd;
    double cpu_use ;
    int i;
 
    for (i = 0; i < cpu_nums; i++) {
        od = (double) (o[i].user + o[i].nice + o[i].system + o[i].idle + o[i].softirq + o[i].iowait + o[i].irq);
        nd = (double) (n[i].user + n[i].nice + n[i].system + n[i].idle + n[i].softirq + n[i].iowait+ n[i].irq);

        id = (double) (n[i].idle);
        sd = (double) (o[i].idle);

        if((nd - od) != 0)
            cpu_use =100.0 - ((id-sd))/(nd-od)*100.00;
        else 
            cpu_use = 0;

	cpu_info[i].use_ratio = (UINT32_T)cpu_use;

    }

    return 0;
}

static int get_sysCpuUsage(TUYA_CPU_INFO_T *cpu_info, int arr_num)
{
    int ret = 0;
    SIZE_T len = sizeof(CPU_OCCUPY_T) * arr_num;

    CPU_OCCUPY_T *cpu_stat1 = (CPU_OCCUPY_T *)tkl_system_malloc(len);
    CPU_OCCUPY_T *cpu_stat2 = (CPU_OCCUPY_T *)tkl_system_malloc(len);
    if (cpu_stat1 == NULL || cpu_stat2 == NULL) {
        ret = OPRT_MALLOC_FAILED;
        goto func_quit;
    }

    if(get_cpuoccupy(cpu_stat1, arr_num)) {
        ret = OPRT_COM_ERROR;
        goto func_quit;
    }

    sleep(1);

    if(get_cpuoccupy(cpu_stat2, arr_num)) {
        ret = OPRT_COM_ERROR;
        goto func_quit;
    }
 
    if (cal_cpuoccupy(cpu_stat1, cpu_stat2, cpu_info, arr_num)) {
        ret = OPRT_COM_ERROR;
        goto func_quit;
    }

func_quit:
    if (cpu_stat1 != NULL) tkl_system_free(cpu_stat1);
    if (cpu_stat2 != NULL) tkl_system_free(cpu_stat2);

    return ret;
}

/**
* @brief get system cpu info
*
* @param[in] cpu_ary: info of cpus
* @param[in] cpu_cnt: num of cpu
* @note This API is used for system cpu info get.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_system_get_cpu_info(TUYA_CPU_INFO_T **cpu_ary, INT_T *cpu_cnt)
{
    int cpu_nums;
    int cpu_nums_total;
    TUYA_CPU_INFO_T *cpu_info;

    return OPRT_NOT_SUPPORTED;
    if (cpu_ary == NULL || cpu_cnt == NULL) {
        return OPRT_INVALID_PARM;
    }

    cpu_nums = (int)sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_nums <= 0) {
        return OPRT_COM_ERROR; 
    }
    cpu_nums_total = cpu_nums + 1; // it cover cpu total, so plus 1.

    size_t mem_size = sizeof(TUYA_CPU_INFO_T) * cpu_nums_total;
    if ((cpu_info = (TUYA_CPU_INFO_T *)tkl_system_malloc(mem_size)) == NULL) {
        return OPRT_COM_ERROR;
    }

    if (get_sysCpuUsage(cpu_info, cpu_nums_total)) {
        return OPRT_COM_ERROR;
    }

    *cpu_ary = cpu_info;
    *cpu_cnt = cpu_nums;

    return OPRT_OK;
}
