TARGET = pincfg

LIBS := -lncurses

SRCS = main.c parse.c

OBJS := $(SRCS:.c=.o)

install:$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ -c $<

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

########################################################
all: install

clean:
	rm -rf $(target) $(OBJS)
