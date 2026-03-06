TARGET = thermal_pwm

LIBS :=  -lpthread -lm

SRCS = main.c \
	pid.c \
	platform.c \
	utils.c

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
