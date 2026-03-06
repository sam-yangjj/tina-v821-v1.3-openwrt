Package/txw901-firmware = $(call Package/firmware-default,txw901 firmware)

SRC_CODE_DIR:=$(TXW901_FW)

define Package/txw901-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/hgics.bin $(1)/$(FIRMWARE_PATH)
endef
$(eval $(call BuildPackage,txw901-firmware))


