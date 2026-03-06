ifeq ($(CONFIG_TUYA_TKL_WATCHDOG_TEST),y)

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_watchdog_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
	test/tkl_watchdog_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \
	-I$(LOCAL_PATH)/include/tuyaos

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_watchdog
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_watchdog

include $(BUILD_EXECUTABLE)

endif
