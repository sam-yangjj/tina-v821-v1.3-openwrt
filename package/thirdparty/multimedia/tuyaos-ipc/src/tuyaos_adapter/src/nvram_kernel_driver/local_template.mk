ifeq ($(CONFIG_TUYA_BSP_NVRAM_KDRIVER), y)

# 当前文件所在目录
LOCAL_PATH := $(call my-dir)


#---------------------------------------

# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := nvram

# 模块源代码
LOCAL_SRC_FILES := src/lib/nvram.c

# 模块的 CFLAGS
LOCAL_CFLAGS := -fPIC -I$(LOCAL_PATH)/include
LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/common

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_EXPORT_SHARED_LIBRARY := YES

# 生成动态库
include $(BUILD_SHARED_LIBRARY)


#---------------------------------------
# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := nvram

# 模块源代码
LOCAL_SRC_FILES := src/cmd/nvram_cmd.c

# 模块的 CFLAGS
LOCAL_CFLAGS := -I$(LOCAL_PATH)/include
LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/common


CI_NVRAM_FLASH_SIZE=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_SIZE))
CI_NVRAM_FLASH_OFFSET=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_OFFSET))

LOCAL_CFLAGS += -DNVRAM_FLASH_SIZE=$(CI_NVRAM_FLASH_SIZE)
LOCAL_CFLAGS += -DNVRAM_FLASH_OFFSET=$(CI_NVRAM_FLASH_OFFSET)

LOCAL_SHARED_LIBRARIES := libnvram

# 生成动态库
include $(BUILD_EXECUTABLE)

#---------------------------------------
# PC端工具
include $(CLEAR_VARS)

LOCAL_MODULE := gen_nvram_zone
LOCAL_SRC_FILES := \
	src/common/aes.c \
	src/tools/file_flash.c \
	src/tools/file_nvram.c \
	src/tools/gen_nvram_zone.cpp \
	src/common/uni_base64.c
LOCAL_CFLAGS := -I$(LOCAL_PATH)/include -I$(LOCAL_PATH)/src/tools -D__TOOL__
LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/common
LOCAL_CXXFLAGS := -I$(LOCAL_PATH)/include -I$(LOCAL_PATH)/src/tools -D__TOOL__ -fpermissive

DFLAGS := $(foreach n,$(CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY),$(n))

DVAR0 := $(word 1,$(DFLAGS))
DVAR1 := $(patsubst "%,%,$(DVAR0))
DDX += $(subst $(DVAR0),$(DVAR1),$(DFLAGS))
DVAR1 := $(word 1,$(DDX))

DVAR2 := $(word 2,$(DFLAGS))
DVAR3 := $(word 3,$(DFLAGS))
DVAR4 := $(word 4,$(DFLAGS))
DVAR5 := $(word 5,$(DFLAGS))
DVAR6 := $(word 6,$(DFLAGS))
DVAR7 := $(word 7,$(DFLAGS))
DVAR8 := $(word 8,$(DFLAGS))
DVAR9 := $(word 9,$(DFLAGS))
DVAR10 := $(word 10,$(DFLAGS))
DVAR11 := $(word 11,$(DFLAGS))
DVAR12 := $(word 12,$(DFLAGS))
DVAR13 := $(word 13,$(DFLAGS))
DVAR14 := $(word 14,$(DFLAGS))
DVAR15 := $(word 15,$(DFLAGS))

DVAR20 := $(word 16,$(DFLAGS))
DVAR21 := $(patsubst %",%,$(DVAR20))
DDX2 += $(subst $(DVAR20),$(DVAR21),$(DDX))
DVAR16 := $(word 16,$(DDX2))

ifneq ($(wildcard $(shell pwd)/sdk),)
    PWD := $(shell pwd)/sdk/nvram_kernel_driver
endif

ifneq ($(wildcard $(shell pwd)/components/nvram_kernel_driver),)
    PWD := $(shell pwd)/components/nvram_kernel_driver
endif

ifneq ($(wildcard $(shell pwd)/tuyaos_adapter/src/nvram_kernel_driver),)
    PWD := $(shell pwd)/tuyaos_adapter/src/nvram_kernel_driver
endif


VALUE = $(shell $(PWD)/make.sh "$(DVAR1)")
LOCAL_CFLAGS += -DKEY1=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR2)")
LOCAL_CFLAGS += -DKEY2=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR3)")
LOCAL_CFLAGS += -DKEY3=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR4)")
LOCAL_CFLAGS += -DKEY4=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR5)")
LOCAL_CFLAGS += -DKEY5=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR6)")
LOCAL_CFLAGS += -DKEY6=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR7)")
LOCAL_CFLAGS += -DKEY7=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR8)")
LOCAL_CFLAGS += -DKEY8=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR9)")
LOCAL_CFLAGS += -DKEY9=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR10)")
LOCAL_CFLAGS += -DKEY10=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR11)")
LOCAL_CFLAGS += -DKEY11=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR12)")
LOCAL_CFLAGS += -DKEY12=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR13)")
LOCAL_CFLAGS += -DKEY13=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR14)")
LOCAL_CFLAGS += -DKEY14=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR15)")
LOCAL_CFLAGS += -DKEY15=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR16)")
LOCAL_CFLAGS += -DKEY16=$(VALUE)

CI_NVRAM_FLASH_SIZE=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_SIZE))
CI_NVRAM_FLASH_OFFSET=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_OFFSET))

LOCAL_CFLAGS += -DNVRAM_FLASH_SIZE=$(CI_NVRAM_FLASH_SIZE)
LOCAL_CFLAGS += -DNVRAM_FLASH_OFFSET=$(CI_NVRAM_FLASH_OFFSET)
LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/common

LOCAL_LDFLAGS := -lstdc++
LOCAL_IS_HOST_MODULE := true
include $(BUILD_EXECUTABLE)


#---------------------------------------

# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
#LOCAL_MODULE := $(notdir $(LOCAL_PATH))
LOCAL_MODULE := nvram_kernel_driver

# 模块源代码
#LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH)/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES := src/common/aes.c src/linux/linux_flash.c src/linux/linux_nvram.c src/common/uni_base64.c

# 模块的 CFLAGS
LOCAL_CFLAGS := -I$(LOCAL_PATH)/include


#AES128_KEY = $(call qstrip, $(CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY))

#$(call MSG_INFO, "TUYA_NVRAM_PLATFORM_AES_KEY is $(CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY)")
$(call warning, "CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY is $(CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY)")


DFLAGS := $(foreach n,$(CONFIG_TUYA_NVRAM_KERNEL_DRIVER_KEY),$(n))
$(call warning, "DFLAGS is $(DFLAGS)")

DVAR0 := $(word 1,$(DFLAGS))
DVAR1 := $(patsubst "%,%,$(DVAR0))
DDX += $(subst $(DVAR0),$(DVAR1),$(DFLAGS))
DVAR1 := $(word 1,$(DDX))

#DVAR1 := $(word 1,$(DFLAGS))
DVAR2 := $(word 2,$(DFLAGS))
DVAR3 := $(word 3,$(DFLAGS))
DVAR4 := $(word 4,$(DFLAGS))
DVAR5 := $(word 5,$(DFLAGS))
DVAR6 := $(word 6,$(DFLAGS))
DVAR7 := $(word 7,$(DFLAGS))
DVAR8 := $(word 8,$(DFLAGS))
DVAR9 := $(word 9,$(DFLAGS))
DVAR10 := $(word 10,$(DFLAGS))
DVAR11 := $(word 11,$(DFLAGS))
DVAR12 := $(word 12,$(DFLAGS))
DVAR13 := $(word 13,$(DFLAGS))
DVAR14 := $(word 14,$(DFLAGS))
DVAR15 := $(word 15,$(DFLAGS))
#DVAR16 := $(word 16,$(DFLAGS))

DVAR20 := $(word 16,$(DFLAGS))
DVAR21 := $(patsubst %",%,$(DVAR20))
DDX2 += $(subst $(DVAR20),$(DVAR21),$(DDX))
DVAR16 := $(word 16,$(DDX2))


$(call warning, "DVAR1 is $(DVAR1)")
$(call warning, "DVAR16 is $(DVAR16)")

#SEED1 := $(shell perl -e "print int(rand(100))")
#LOCAL_CFLAGS += -DKEY1=\"$(DVAR1)\"


ifneq ($(wildcard $(shell pwd)/sdk),)
    PWD := $(shell pwd)/sdk/nvram_kernel_driver
endif

ifneq ($(wildcard $(shell pwd)/components/nvram_kernel_driver),)
    PWD := $(shell pwd)/components/nvram_kernel_driver
endif

ifneq ($(wildcard $(shell pwd)/tuyaos_adapter/src/nvram_kernel_driver),)
    PWD := $(shell pwd)/tuyaos_adapter/src/nvram_kernel_driver
endif

VALUE = $(shell $(PWD)/make.sh "$(DVAR1)")
LOCAL_CFLAGS += -DKEY1=$(VALUE)
$(call warning, "DVAR1 is $(DVAR1)")
$(call warning, "KEY1 is $(VALUE)")

VALUE = $(shell $(PWD)/make.sh "$(DVAR2)")
LOCAL_CFLAGS += -DKEY2=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR3)")
LOCAL_CFLAGS += -DKEY3=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR4)")
LOCAL_CFLAGS += -DKEY4=$(VALUE)
$(call warning, "DVAR4 is $(DVAR4)")
$(call warning, "KEY4 is $(VALUE)")

VALUE = $(shell $(PWD)/make.sh "$(DVAR5)")
LOCAL_CFLAGS += -DKEY5=$(VALUE)
$(call warning, "KEY5 is $(VALUE)")

VALUE = $(shell $(PWD)/make.sh "$(DVAR6)")
LOCAL_CFLAGS += -DKEY6=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR7)")
LOCAL_CFLAGS += -DKEY7=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR8)")
LOCAL_CFLAGS += -DKEY8=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR9)")
LOCAL_CFLAGS += -DKEY9=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR10)")
LOCAL_CFLAGS += -DKEY10=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR11)")
LOCAL_CFLAGS += -DKEY11=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR12)")
LOCAL_CFLAGS += -DKEY12=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR13)")
LOCAL_CFLAGS += -DKEY13=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR14)")
LOCAL_CFLAGS += -DKEY14=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR15)")
LOCAL_CFLAGS += -DKEY15=$(VALUE)

VALUE = $(shell $(PWD)/make.sh "$(DVAR16)")
LOCAL_CFLAGS += -DKEY16=$(VALUE)



CI_NVRAM_FLASH_SIZE=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_SIZE))
CI_NVRAM_FLASH_OFFSET=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_FLASH_OFFSET))
CI_NVRAM_BLKDEV_MAJOR=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_BLKDEV_MAJOR))
CI_NVRAM_BLKDEV_MINOR=$(call qstrip,$(CONFIG_TUYA_BSP_NVRAM_BLKDEV_MINOR))

LOCAL_CFLAGS += -DNVRAM_FLASH_SIZE=$(CI_NVRAM_FLASH_SIZE)
LOCAL_CFLAGS += -DNVRAM_FLASH_OFFSET=$(CI_NVRAM_FLASH_OFFSET)
LOCAL_CFLAGS += -DNVRAM_FLASH_MAJOR=$(CI_NVRAM_BLKDEV_MAJOR)
LOCAL_CFLAGS += -DNVRAM_FLASH_MINOR=$(CI_NVRAM_BLKDEV_MINOR)

ifeq ($(CONFIG_TUYA_TARGET_BOARD_MODEL_TPP01Z),y)
LOCAL_CFLAGS += -DRK3308
endif

include $(BUILD_KMOD)

#---------------------------------------
endif
