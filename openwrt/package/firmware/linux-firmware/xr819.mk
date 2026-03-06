Package/xr819-firmware = $(call Package/firmware-default,Xradio xr819 firmware)

SRC_CODE_DIR:=$(XR819_FW)

define Package/xr819-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
ifeq ($(CONFIG_ARCH_SUN3IW1P1_C600), y)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/boot_B100.bin $(1)/$(FIRMWARE_PATH)/boot_xr819.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/etf_B100.bin $(1)/$(FIRMWARE_PATH)/etf_xr819.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_B100.bin $(1)/$(FIRMWARE_PATH)/fw_xr819.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/sdd_B100.bin $(1)/$(FIRMWARE_PATH)/sdd_xr819.bin
else
	$(INSTALL_DATA) $(SRC_CODE_DIR)/*_xr819.bin $(1)/$(FIRMWARE_PATH)/
endif

endef
$(eval $(call BuildPackage,xr819-firmware))
