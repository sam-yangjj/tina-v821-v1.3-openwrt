CUR_PATH := .

TARGET_BIN := cameraisptester

SRCS := cameraisptester.c

OBJS := $(SRCS:%.c=%.o)

INCLUDE_DIR := \
	-I$(STAGING_DIR)/usr/include/libisp \
	-I$(STAGING_DIR)/usr/include/libisp/include \
	-I$(STAGING_DIR)/usr/include/libisp/isp_dev \
	-I$(STAGING_DIR)/usr/include/libisp/isp_events \
	-I$(STAGING_DIR)/usr/include/libisp/isp_manager \
	-I$(STAGING_DIR)/usr/include/libisp/isp_math \
	-I$(STAGING_DIR)/usr/include/libisp/isp_tunning \
	-I$(STAGING_DIR)/usr/include/libisp/include/device \
	-I$(LINUX_DIR)/usr/include/bsp

LIBS_DIR := \
	$(STAGING_DIR)/usr/lib

SHARED_LIBS := \
	-llog

STATIC_LIBS := \
	-lz \
	-liniparser \
	-lISP \
	-lisp_ae \
	-lisp_af \
	-lisp_afs \
	-lisp_awb \
	-lisp_base \
	-lisp_dev \
	-lisp_gtm \
	-lisp_ini \
	-lisp_iso \
	-lisp_math \
	-lisp_md \
	-lisp_pltm \
	-lisp_rolloff

LOCAL_CFLAGS := \


#-Wl,-rpath-link=$(LIBS_DIR)

LOCAL_CFLAGS := \
    $(CFLAGS) \
    $(LDFLAGS) \
    $(patsubst %,-L%,$(LIBS_DIR)) \
    -Wl,-rpath-link=$(subst $(space),:,$(strip $(LIBS_DIR))) \
    -Wl,-Bstatic -Wl,--whole-archive -Wl,--start-group $(STATIC_LIBS) -Wl,--end-group -Wl,--no-whole-archive \
    -Wl,-Bdynamic $(SHARED_LIBS)

.PHONY: all clean

all:$(TARGET_BIN)

%.o:%.c
	$(CC) $(INCLUDE_DIR) $^ -MD -MP -MF $(@:%=%.d) -c -o $@

$(TARGET_BIN):$(OBJS)
	$(CC) $^ $(LOCAL_CFLAGS) -o $@

clean:
	$(RM) $(TARGET_BIN) $(OBJS)
