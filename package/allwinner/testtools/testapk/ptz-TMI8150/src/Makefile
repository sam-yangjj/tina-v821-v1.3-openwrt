lib_src := \
	ptz-TMI8150B.c \
	spi.c \

target := ptz

.PHONY: all

all:
	$(CC)  $(CFLAGS) $(LDFLAGS) $(lib_src) -o $(target)

clean:
	rm  $(target)
