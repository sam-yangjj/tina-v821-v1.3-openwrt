TARGET = gpu_keystone_correction

LOCAL_CFLAGS =
LOCAL_LDFLAGS = -lEGL -lGLESv2 -lmali -lkeystone

SRCS += ${wildcard *.c}
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

all:install

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -I./include -I./ $(LDFLAGS) -L./lib $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) -I./include -I./ $(LDFLAGS) -L./lib $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

install:$(TARGET)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(TARGET) $(CONFIG_PREFIX)/bin

clean:
	-rm -rf $(OBJS) $(TARGET)
.PHONY:all clean $(TARGET) install
