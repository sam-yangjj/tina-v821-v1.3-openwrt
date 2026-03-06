#
#Copyright (c) 2019 Allwinner Technology Co. Ltd.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=upgrade_app
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DEPENDS:= +uboot-envtools

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_CFLAGS += -I$(STAGING_DIR)/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib/
TARGET_LDFLAGS += -L$(STAGING_DIR)/lib/

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +uboot-envtools
  TITLE:=upgrede_app for nor ota.
endef

define Package/$(PKG_NAME)/description
  This package is used for nor ota

endef

define Build/Prepare
	@$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	@$(CP) ./src $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/upgrade_app $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
