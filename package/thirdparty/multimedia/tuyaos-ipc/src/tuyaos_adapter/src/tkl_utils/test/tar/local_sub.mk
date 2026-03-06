
ifeq ($(CONFIG_TY_TKL_UTILS_TEST_TAR),y)

# LOCAL_PATH = path of base_hal

# clear LOCAL_xxx variables except LOCAL_PATH
include $(CLEAR_VARS)

LOCAL_MODULE := tkl_tar_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
		test/tar/tkl_tar_test.c \

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/include \

LOCAL_LDFLAGS := -lnvram -lsys_log -ltkl_utils
LOCAL_SHARED_LIBRARIES := libnvram libsys_log libtkl_utils


include $(BUILD_EXECUTABLE)

endif
