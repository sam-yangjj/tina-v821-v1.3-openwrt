Package/v821_wlan-firmware = $(call Package/firmware-default,v821_wlan firmware)

SRC_CODE_DIR:=$(V821_WLAN_FW)

define Package/v821_wlan-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/boot_v821.bin $(1)/$(FIRMWARE_PATH)/boot_v821.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_mac_v821.bin $(1)/$(FIRMWARE_PATH)/fw_mac_v821.bin
ifeq ($(CONFIG_PACKAGE_v821-rftest), y)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/etf_v821.bin $(1)/$(FIRMWARE_PATH)/etf_v821.bin
endif

endef
$(eval $(call BuildPackage,v821_wlan-firmware))
