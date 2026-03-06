TARGET = aicrf_wifi_test

all: $(TARGET) aicrf_bt_test
$(TARGET): aicrf_wifi_test.o
	@mkdir -p $(CONFIG_PREFIX)
	$(CC) $(CFLAGS) -lpthread $(LOCAL_CFLAGS) -I$(INCLUDES) $(LDFLAGS) $^ -o $@
	@cp $@ $(CONFIG_PREFIX)/aicrf_wifi_test

aicrf_bt_test: aicrf_bt_test.o
	@mkdir -p $(CONFIG_PREFIX)
	$(CC) $(CFLAGS) -lpthread $(LOCAL_CFLAGS) -I$(INCLUDES) $(LDFLAGS) $^ -o $@
	@cp $@ $(CONFIG_PREFIX)/aicrf_bt_test

clean:
	$(RM) $(TARGET)
