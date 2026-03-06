include $(TOPDIR)/rules.mk

PKG_NAME:=hardwaretest
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=test
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE:=hardware signal test
  DEPENDS:= +cpu_monitor +mtop +stress-ng +stressapptest +uart_test +rwcheck
endef

define Package/$(PKG_NAME)/description
	Hardware signal test
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpu_dvfs_testing.sh $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpu_light_heavy_testing.sh $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scenario_testing.sh $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/s16le_44100_stereo.wav $(1)/etc
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
