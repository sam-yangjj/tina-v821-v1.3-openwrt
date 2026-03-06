Package/xr819a-firmware = $(call Package/firmware-default,Xradio xr819a firmware)

SRC_CODE_DIR:=$(XR819A_FW)

define Package/xr819a-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/*_xr819.bin $(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,xr819a-firmware))
