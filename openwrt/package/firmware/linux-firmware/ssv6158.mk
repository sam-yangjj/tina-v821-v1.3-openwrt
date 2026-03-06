Package/ssv6158-firmware = $(call Package/firmware-default,ssv6158 firmware)
SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/ssv6158
define Package/ssv6158-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/ssv6x5x-wifi.cfg $(1)/$(FIRMWARE_PATH)/ssv6x5x-wifi.cfg
endef
$(eval $(call BuildPackage,ssv6158-firmware))
