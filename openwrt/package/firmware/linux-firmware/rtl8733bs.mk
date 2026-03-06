Package/rtl8733bs-firmware = $(call Package/firmware-default,RealTek RTL8733BS firmware)

SRC_CODE_DIR:=$(RTL8733BS_FW)

define Package/rtl8733bs-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8723fs_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723fs_fw
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8723fs_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723fs_config
endef

$(eval $(call BuildPackage,rtl8733bs-firmware))
