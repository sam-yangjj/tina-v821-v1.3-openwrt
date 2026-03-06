TARGET := tcp8852_ptz_test
SRCS := tcp8852_ptz_test.c
OBJS := $(SRCS:%.c=%.o)
DEPEND := tcp8852_ptz_test.d

INCLUDE_DIRS := -I $(LINUX_USER_HEADERS)/include -I $(LINUX_USER_HEADERS)/include/bsp

.PHONY: all clean

all:$(TARGET)

-include $(DEPEND)

$(DEPEND):$(SRCS)
	$(CC) -MM $(SRCS) > $@.$$$$ ;\
	sed 's/$(OBJS)/$(OBJS) $@/g' < $@.$$$$ >$@ ;\
	$(RM) $@.$$$$

%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDE_DIRS) $<

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPEND)
