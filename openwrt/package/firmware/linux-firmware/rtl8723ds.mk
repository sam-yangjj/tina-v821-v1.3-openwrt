Package/rtl8723ds-firmware = $(call Package/firmware-default,RealTek RTL8723DS firmware)

SRC_CODE_DIR:=$(RTL8823DS_FW)

define Package/rtl8723ds-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8723dsh4_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723dsh4_fw
	$(INSTALL_DATA) \
		$(SRC_CODE_DIR)/rtl8723dsh4_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723dsh4_config
	cp \
		$(SRC_CODE_DIR)/rtl8723d_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723d_fw
	cp \
		$(SRC_CODE_DIR)/rtl8723d_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723d_config
endef
$(eval $(call BuildPackage,rtl8723ds-firmware))
