CC = gcc
CFLAGS = -Wall
LDFLAGS = -luapi -lawion
OBJS = yuview.o

all: yuview

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

yuview: $(OBJS)
	$(CC) -o $@ $(OBJS) -luapi -lawion

clean:
	rm -f rbcfg *.o
