# LOCAL_PATH = path of tkl_wired

include $(CLEAR_VARS)

LOCAL_MODULE := tkl_wired_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
		test/test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

LOCAL_LDFLAGS := -ltkl_wired -lpthread

LOCAL_STATIC_LIBRARIES := libtkl_wired

include $(BUILD_EXECUTABLE)
