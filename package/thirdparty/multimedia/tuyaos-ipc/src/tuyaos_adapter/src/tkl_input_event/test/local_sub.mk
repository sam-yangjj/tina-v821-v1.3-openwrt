
ifeq ($(CONFIG_TUYA_TKL_INPUT_EVENT_TEST),y)

# LOCAL_PATH = path of base_hal

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_input_event_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
		test/tkl_input_event_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

LOCAL_LDFLAGS := -lnvram -lsys_log  -ltkl_input_event
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_input_event

include $(BUILD_EXECUTABLE)

endif
