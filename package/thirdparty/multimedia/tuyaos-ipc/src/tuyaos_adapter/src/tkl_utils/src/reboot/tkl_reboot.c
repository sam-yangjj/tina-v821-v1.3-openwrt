/**
 * @file tkl_reboot.c
 * @brief the implements of ty reboot 
 * @version 0.1
 * @date 2019-08-15
 *
 * @copyright Copyright 2020-2021 Ty Inc. All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/reboot.h>
#include "tkl_reboot.h"

VOID_T tkl_reboot(int en_sync)
{
    if(en_sync)
        sync();

    reboot(RB_AUTOBOOT);
}
