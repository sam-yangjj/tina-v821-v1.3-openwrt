nvram_uboot组件

注意：
1、由于uboot无法简单的拼接成果物，故本组件无法直接在每个平台中使用，只做代码记录。具体移植工作需要各平台负责人完成。

2、一般存放于uboot/common/bsp_nvram/路径下

3、由于平台差异，uboot_flash.c中的读写函数需要适配；uboot_nvram.c中的NVRAM_HEADER_LOAD_ADDR宏需要适配。

4、头文件include/nvram/bcmnvram.h中的宏需要和CI可视化配置一致“Tuya bsp nvram kernle driver”，包括NVRAM_FLASH_SIZE，NVRAM_FLASH_OFFSET，AES_128_KEY, NVRAM_UBOOT_FLASH_OFFSET。需要把四个宏移植到uboot的local.mk中.
移植前：
sed -i '/CFLAGS += -DCONFIG_TUYA_PRODUCT_/c CFLAGS += -DCONFIG_TUYA_PRODUCT_$(product_type)' $(bootloader_src)/uboot-2013.10/config.mk
移植后：
CI_NVRAM_FLASH_SIZE=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_SIZE))
sed -i '/CFLAGS += -DCONFIG_TUYA_PRODUCT_/c CFLAGS += -DCONFIG_TUYA_PRODUCT_$(product_type) CFLAGS += -DNVRAM_FLASH_SIZE=$(CI_NVRAM_FLASH_SIZE)' $(bootloader_src)/uboot-2013.10/config.mk

5、CI可视化里配置CONFIG_TUYA_BSP_NVRAM_UBOOT_FLASH_OFFSET宏，通过和第4点一样的方法传入-DNVRAM_UBOOT_FLASH_OFFSET=CI_TUYA_BSP_NVRAM_UBOOT_FLASH_OFFSET

6、至于main_loop()函数中的调用nvram功能需要各平台负责人移植。


uboot下移植代码架构：
include/nvram/bcmnvran.h
src/uboot/uboot_flash.c
src/uboot/uboot_nvram.c
src/aes.c
src/uni_base64.c
src/nvram_core.c
src/aes.h
src/uni_base64.h

