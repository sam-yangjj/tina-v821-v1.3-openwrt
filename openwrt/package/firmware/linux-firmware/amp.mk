Package/amp-firmware = $(call Package/firmware-default,allwinner amp firmware)

SRC_FW:=$(AMP_FW)

define Package/amp-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(SRC_FW) $(1)/$(FIRMWARE_PATH)/
endef

$(eval $(call BuildPackage,amp-firmware))
