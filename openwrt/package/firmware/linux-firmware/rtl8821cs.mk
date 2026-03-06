Package/rtl8821cs-firmware = $(call Package/firmware-default,RealTek RTL8821CS firmware)

SRC_CODE_DIR:=$(RTL8821CS_FW)

define Package/rtl8821cs-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8821c_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8821c_fw
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8821c_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8821c_config
endef

$(eval $(call BuildPackage,rtl8821cs-firmware))
