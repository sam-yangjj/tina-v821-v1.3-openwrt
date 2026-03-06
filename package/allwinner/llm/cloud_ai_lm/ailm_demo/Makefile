include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/package.mk
-include $(INCLUDE_DIR)/version.mk
-include $(BUILD_DIR)/package.mk

PKG_NAME:=ailm
PKG_VERSION:=0.1
PKG_RELEASE:=1
#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/llm/cloud_ai_lm

define Package/$(PKG_NAME)
  SUBMENU:=AiLargeModel
  CATEGORY:=Allwinner
  TITLE:=Ai Large Model
  DEPENDS:= +libailm
endef

define Package/$(PKG_NAME)/description
	Tina Ai Chat Demo
endef

define Build/Prepare
	rm $(PKG_BUILD_DIR) -rf
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/demo $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		all
endef

define Build/InstallDev/$(PKG_NAME)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/ailm $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/ailm
	$(INSTALL_DATA) $(SRC_CODE_DIR)/demo/ailm.config $(1)/etc/ailm
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
