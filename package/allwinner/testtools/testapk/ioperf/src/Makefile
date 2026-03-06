TARGET := ioperf

SRCS := main.c
OBJS := $(SRCS:%.c=%.o)
DEPEND := main.d

.PHONY: all clean

all:$(TARGET)

-include $(DEPEND)

$(DEPEND):$(SRCS)
	$(CC) -MM $(SRCS) > $@.$$$$ ;\
	sed 's/$(OBJS)/$(OBJS) $@/g' < $@.$$$$ >$@ ;\
	$(RM) $@.$$$$

%.o:%.c
	$(CC) -c $(CFLAGS) $<

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPEND)
