Package/aic8800-firmware = $(call Package/firmware-default,AIC aic8800 firmware)

SRC_CODE_DIR:=$(AIC8800_FW)

define Package/aic8800-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
ifeq ($(CONFIG_AIC8800_USE_USB_SDD), y)
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)aic8800d80/

	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/aic8800d80/fmacfw_8800d80_u02.bin $(1)/$(FIRMWARE_PATH)/aic8800d80
	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/aic8800d80/fw_adid_8800d80_u02.bin $(1)/$(FIRMWARE_PATH)/aic8800d80
	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/aic8800d80/fw_patch_8800d80_u02.bin $(1)/$(FIRMWARE_PATH)/aic8800d80
	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/aic8800d80/fw_patch_table_8800d80_u02.bin $(1)/$(FIRMWARE_PATH)/aic8800d80
ifeq ($(CONFIG_PACKAGE_aic8800-rftest), y)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/usb/aic8800d80/lmacfw_rf_8800d80_u02.bin $(1)/$(FIRMWARE_PATH)/aic8800d80
endif
else
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)aic8800d80/

	$(INSTALL_DATA) $(SRC_CODE_DIR)/*.* $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/sdio/aic8800d80/*.* $(1)/$(FIRMWARE_PATH)aic8800d80
endif
endef
$(eval $(call BuildPackage,aic8800-firmware))


