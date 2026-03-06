Package/v821b_wlan-firmware = $(call Package/firmware-default,v821b_wlan firmware)

SRC_CODE_DIR:=$(V821B_WLAN_FW)

define Package/v821b_wlan-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/boot_v821b.bin $(1)/$(FIRMWARE_PATH)/boot_v821b.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_mac_v821b.bin $(1)/$(FIRMWARE_PATH)/fw_mac_v821b.bin
ifeq ($(CONFIG_PACKAGE_v821-rftest), y)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/etf_v821b.bin $(1)/$(FIRMWARE_PATH)/etf_v821b.bin
endif

endef
$(eval $(call BuildPackage,v821b_wlan-firmware))
