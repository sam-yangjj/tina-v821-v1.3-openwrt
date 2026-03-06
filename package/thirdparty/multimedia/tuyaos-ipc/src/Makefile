############################################################
# 默认编译目标
############################################################
target = tuyaos_demo_ipc

############################################################
# 同步tuyaos Makefile_sdk里写的编译参数
############################################################
TUYA_BUILD_PARAM_CFLAGS := -pipe -D_REENTRANT -Wall -Wno-unused-result -ffunction-sections -fdata-sections -O2
TUYA_PLATFORM_CFLAGS := -std=gnu11 -Os -fno-aggressive-loop-optimizations -Wno-format-truncation
TUYA_CCFLAGS := -g -fPIC -ffunction-sections -fdata-sections

LOCAL_CFLAGS += $(TUYA_CCFLAGS) $(TUYA_PLATFORM_CFLAGS) $(TUYA_BUILD_PARAM_CFLAGS)

############################################################
# 同步tuyaos Makefile_sdk里写的依赖库
############################################################
LIBS += -L./sdk/libs
LIBS += -ltuyaos -pthread -lm -lrt -ldl -Wl,--gc-sections

############################################################
# 添加allwinner的wifi、media依赖库
############################################################
# aw wifi lib
LIBS += -lnl-tiny -lwifimg-v2.0 -lwirelesscom -lrpbuf -lrpmsg -ljson-c

# aw bt lib
ifeq ($(CONFIG_PACKAGE_v821b_wlan-firmware),y)
LIBS += -lbtmg -lbluetooth -lbluetooth-internal -ljson-c -ldl -lm -lresolv -lshared-mainloop
endif

# aw media lib
MPP_LIB_DIR := $(STAGING_DIR)/usr/lib/eyesee-mpp
PDET_LIB_DIR := $(STAGING_DIR)/usr/lib/pdet

MPP_LIBS := $(addprefix -l, $(patsubst lib%,%,$(patsubst %.a,%,$(notdir $(foreach dir,$(MPP_LIB_DIR),$(wildcard $(dir)/*.a))))))
PDET_LIBS := $(addprefix -l, $(patsubst lib%,%,$(patsubst %.a,%,$(notdir $(foreach dir,$(PDET_LIB_DIR),$(wildcard $(dir)/*.a))))))
CEDA_LIBS := -lcdc_base -lMemAdapter -lVE -lvenc_base -lvenc_common -lvenc_h264 -lvenc_jpeg  -lvencoder
AWISPAPI_LIBS := -liniparser -lisp_ae -lisp_afs -lisp_base -lisp_gtm -lisp_iso -lisp_md -lisp_rolloff \
				-lISP -lisp_af -lisp_awb -lisp_dev -lisp_ini -lisp_math -lisp_pltm

MPP_LIB := \
  -Wl,-Bstatic -Wl,--start-group \
  $(MPP_LIBS) \
  $(PDET_LIBS) \
  $(CEDA_LIBS) \
  $(AWISPAPI_LIBS) \
  -lawion \
  -lMotionDetect \
  -liconv \
  -lzbar \
  -Wl,--end-group \
  -Wl,--no-whole-archive \
  -Wl,-Bdynamic \
  -lasound \
  -lexpat \
  -lglog \
  -llog \
  -lunwind \
  -lz \
  -lstdc++

LIBS += $(MPP_LIB)

# aw lib dir
LIBS += -L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib/libiconv-stub/lib \
           -L$(MPP_LIB_DIR) -L$(PDET_LIB_DIR)

ifeq ($(CONFIG_TUYAOS_IPC_LOWPOWER),y)
LIBS += -L$(STAGING_DIR)/usr/lib/rt_media -lrt_media
endif

############################################################
# 添加头文件路径
############################################################
INCLUDES += $(addprefix -I, $(shell find ./sdk/include -type d))
INCLUDES += $(addprefix -I, $(shell find ./application_components -type d))
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/include -type d))
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_wifi -type d))
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_bluetooth -type d))
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_wired -type d))
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_motor -type d))
ifeq ($(CONFIG_TUYAOS_IPC_LOWPOWER),y)
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_media_fastboot -type d))
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/rt_media/ -type d))
else
INCLUDES += $(addprefix -I, $(shell find ./tuyaos_adapter/src/tkl_media_template -type d))
endif
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/middleware/ -type d))
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/system/ -type d))
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/libisp/ -type d))
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/libcedarc/ -type d))
INCLUDES += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/pdet/ -type d))
INCLUDES += -I$(KERNEL_DIR)/user_headers/include/bsp
INCLUDES += -I$(KERNEL_DIR)/user_headers/include/bsp/media
INCLUDES += -I$(STAGING_DIR)/usr/include

############################################################
# 添加源文件
############################################################
SRC += $(wildcard ./application_components/*/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/include/utilities/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/include/init/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_system/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_wifi/src/*.c)
ifeq ($(CONFIG_PACKAGE_v821b_wlan-firmware),y)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_bluetooth/src/tkl_bluetooth.c)
else
SRC += $(wildcard ./tuyaos_adapter/src/tkl_bluetooth/src/tkl_bluetooth_empty.c)
endif
SRC += $(wildcard ./tuyaos_adapter/src/tkl_wired/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_motor/src/*.c)
ifeq ($(CONFIG_TUYAOS_IPC_LOWPOWER),y)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_media_fastboot/src/*.c)
else
SRC += $(wildcard ./tuyaos_adapter/src/tkl_media_template/src/*.c)
SRC += $(wildcard ./tuyaos_adapter/src/tkl_media_template/common/*.c)
endif

############################################################
# 添加LOCAL CFLAGS
############################################################
ifeq ($(CONFIG_TUYAOS_IPC_LOWPOWER),y)
LOCAL_CFLAGS += -DLOW_BAT_IPC
endif

ifeq ($(CONFIG_PACKAGE_v821b_wlan-firmware),y)
LOCAL_CFLAGS += -DSUPPORT_V821B_BLE
endif

############################################################
# 编译目标
############################################################
$(target): $(SRC)
	$(CC) -o $@ $^ $(LOCAL_CFLAGS) $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS)

all: $(target)

clean:
	rm -rf $(target)

.PHONY: all clean
