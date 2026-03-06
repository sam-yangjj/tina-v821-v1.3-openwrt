TARGET = anti_copy_board
LIB = libacb.so
INCLUDES += -I.
LIBS += -L. -lacb
LIB_SRCS = anti_copy_board.c
MAIN_SRCS = main.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)
%.o: %.c
	$(CC)  -fPIC $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(TARGET): $(MAIN_OBJS)
	$(CC) -o $@ $(MAIN_OBJS) $(LIBS) $(LDFLAGS)

$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $(LIB_OBJS) $(LDFLAGS)

all:$(LIB) $(TARGET)
clean:
	rm -rf $(TARGET) *.o *.a
