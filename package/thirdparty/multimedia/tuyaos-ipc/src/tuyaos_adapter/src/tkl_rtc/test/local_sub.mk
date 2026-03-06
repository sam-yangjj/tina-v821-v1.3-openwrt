
ifeq ($(CONFIG_TY_TKL_RTC_TEST),y)

include $(CLEAR_VARS)

LOCAL_MODULE := tkl_rtc_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := test/tkl_rtc_test.c

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_rtc
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_rtc

include $(BUILD_EXECUTABLE)

endif
