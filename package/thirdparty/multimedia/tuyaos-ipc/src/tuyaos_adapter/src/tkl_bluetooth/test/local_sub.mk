include $(CLEAR_VARS)
LOCAL_MODULE := tal_bluetooth_test
LOCAL_MODULE_PATH := /../tkl_test

LOCAL_SRC_FILES := \
	test/src/tal_bluetooth.c \
	test/src/tal_ble_peripheral_test.c\

LOCAL_CFLAGS := \
	-I$(LOCAL_PATH)/test/include

LOCAL_LDFLAGS := -ltkl_bluetooth  
LOCAL_STATIC_LIBRARIES := libtkl_bluetooth

include $(BUILD_EXECUTABLE)
