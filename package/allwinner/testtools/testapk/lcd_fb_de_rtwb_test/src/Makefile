TARGET := lcd_fb_de_rtwb_test
SRCS := lcd_fb_de_rtwb_test.c
OBJS := $(SRCS:%.c=%.o)
INCLUDE_DIRS := $(LINUX_USER_HEADERS)/include $(LINUX_USER_HEADERS)/include/bsp
DEPEND := lcd_fb_de_rtwb_test.d

.PHONY: all clean

all:$(TARGET)

-include $(DEPEND)

$(DEPEND):$(SRCS)
	$(CC) -MM $(SRCS) > $@.$$$$ ;\
	sed 's/$(OBJS)/$(OBJS) $@/g' < $@.$$$$ >$@ ;\
	$(RM) $@.$$$$

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(foreach inc,$(INCLUDE_DIRS),$(addprefix -I, $(inc)))

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPEND)
