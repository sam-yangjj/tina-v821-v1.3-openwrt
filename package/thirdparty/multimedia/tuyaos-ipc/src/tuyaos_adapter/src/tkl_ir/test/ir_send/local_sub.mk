
ifeq ($(CONFIG_TY_TKL_IR_SEND_TEST),y)

# LOCAL_PATH = path of base_hal

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_ir_send_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
		test/ir_send/tkl_ir_send_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_ir -lpthread
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_ir

include $(BUILD_EXECUTABLE)

endif
