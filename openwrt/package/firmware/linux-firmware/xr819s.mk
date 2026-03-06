Package/xr819s-firmware = $(call Package/firmware-default,Xradio xr819s firmware)

SRC_CODE_DIR:=$(XR819S_FW)

define Package/xr819s-firmware/install

ifeq ($(CONFIG_XR819S_USE_40M_SDD), y)
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/boot_xr819s.bin $(1)/$(FIRMWARE_PATH)/boot_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/etf_xr819s.bin $(1)/$(FIRMWARE_PATH)/etf_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_xr819s.bin $(1)/$(FIRMWARE_PATH)/fw_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/sdd_xr819s-40M.bin $(1)/$(FIRMWARE_PATH)/sdd_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_xr819s_bt_40M.bin $(1)/$(FIRMWARE_PATH)/fw_xr819s_bt.bin
else
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/boot_xr819s.bin $(1)/$(FIRMWARE_PATH)/boot_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/etf_xr819s.bin $(1)/$(FIRMWARE_PATH)/etf_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_xr819s.bin $(1)/$(FIRMWARE_PATH)/fw_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/sdd_xr819s-24M.bin $(1)/$(FIRMWARE_PATH)/sdd_xr819s.bin
	$(INSTALL_DATA) $(SRC_CODE_DIR)/fw_xr819s_bt.bin $(1)/$(FIRMWARE_PATH)/fw_xr819s_bt.bin
endif
endef

$(eval $(call BuildPackage,xr819s-firmware))
