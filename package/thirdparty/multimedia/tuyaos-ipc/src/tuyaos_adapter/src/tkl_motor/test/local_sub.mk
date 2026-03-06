
ifeq ($(CONFIG_TY_TKL_MOTOR_TEST),y)

# LOCAL_PATH = path of tkl_base

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_motor_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
	test/tkl_motor_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_motor
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_motor


include $(BUILD_EXECUTABLE)

endif
