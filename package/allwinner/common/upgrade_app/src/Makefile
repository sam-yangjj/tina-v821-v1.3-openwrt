TARGET = upgrade_app

SRCS = upgrade_app.c \
	env.c

LIBS := -lubootenv

OBJS := $(SRCS:.c=.o)

install:$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ -c $<

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

########################################################
all: install

clean:
	rm -rf $(target) $(OBJS)
