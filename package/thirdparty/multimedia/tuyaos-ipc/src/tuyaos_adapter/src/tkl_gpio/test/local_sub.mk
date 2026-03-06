
ifeq ($(CONFIG_TY_TKL_GPIO_TEST),y)

# LOCAL_PATH = path of tkl_base

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_gpio_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
	test/tkl_gpio_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include

LOCAL_LDFLAGS := -lnvram -ltkl_base  
LOCAL_SHARED_LIBRARIES := libnvram libtkl_base

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_utils -ltkl_gpio
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_utils libtkl_gpio

include $(BUILD_EXECUTABLE)

endif
