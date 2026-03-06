
ifeq ($(CONFIG_TUYA_SYS_BASE_TEST_LOG),y)

# LOCAL_PATH = path of base_hal

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := sys_log_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := test/sys_log_test.c

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

ifeq ($(CONFIG_TUYA_SYS_LOG_LEVEL_SAVE),y)
LOCAL_SHARED_LIBRARIES := libnvram
endif

ifeq ($(CONFIG_TUYA_SYS_LOG_FILTER),y)
LOCAL_SHARED_LIBRARIES := libnvram
endif

LOCAL_STATIC_LIBRARIES := libsys_log

LOCAL_LDFLAGS += -lpthread

include $(BUILD_EXECUTABLE)

endif
