CUR_PATH := .

TARGET_BIN := vetester

SRCS := vetester.c

OBJS := $(SRCS:%.c=%.o)

INCLUDE_DIR := \
	-I$(STAGING_DIR)/usr/include/libcedarc/include \
	-I$(STAGING_DIR)/usr/include/libcedarc/base/include

LIBS_DIR := \
	$(STAGING_DIR)/usr/lib

SHARED_LIBS :=

STATIC_LIBS := \
    -lvencoder \
    -lvenc_codec \
    -lvenc_base \
    -lMemAdapter \
    -lVE \
    -lcdc_base

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
