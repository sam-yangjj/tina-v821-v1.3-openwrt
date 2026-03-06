#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=powerkey
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk

PKG_POWERKEY_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/power/powerkey
PKG_FILE_DEPENDS += $(PKG_POWERKEY_CBB_SRC_DIR)/files
PKG_FILE_DEPENDS += $(PKG_POWERKEY_CBB_SRC_DIR)/daemon

define Package/$(PKG_NAME)
  SUBMENU:=Power
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=powerkey for allwinner
  DEPENDS:=+libpthread
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  it comes from platform/allwinner/power/powerkey!!
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(PKG_POWERKEY_CBB_SRC_DIR)/daemon/ $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_POWERKEY_CBB_SRC_DIR)/files/ $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/daemon \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/daemon/powerkey_daemon $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/powerkey.init $(1)/etc/init.d/

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
