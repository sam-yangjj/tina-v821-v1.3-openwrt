TARGET = hgpriv

all: $(TARGET)
$(TARGET): hgpriv.o
	@mkdir -p $(CONFIG_PREFIX)
	$(CC) $(CFLAGS) -lpthread $(LOCAL_CFLAGS) -I$(INCLUDES) $(LDFLAGS) $^ -o $@
	@cp $@ $(CONFIG_PREFIX)/hgpriv

clean:
	$(RM) $(TARGET)
