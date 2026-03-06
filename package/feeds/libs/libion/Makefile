#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libion
PKG_VERSION:=27a8e282367e0a2fe98e99fbb7ad9783e61b00d1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://android.googlesource.com/platform/system/memory/$(PKG_NAME)/+archive/$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_HASH:=920397b23bf43ffd60d83473c29263987e36ab0fd76e4cb90762fa5cccd0d2e5

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=OWNERS
PKG_MAINTAINER:=sspatil <sspatil@google.com>

include $(INCLUDE_DIR)/package.mk

define Package/libion
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A ion memory allocation library
  URL:=https://android.googlesource.com/platform/system/memory/
  DEPENDS:=+libstdcpp
endef

define Package/libion/description
 Encapsulation of several functions of ion toctl.
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(TAR) -zxvf $(PKG_SOURCE) -C $(PKG_BUILD_DIR)
	$(call Build/Patch/Default)
	$(CP) -r Makefile_src $(PKG_BUILD_DIR)/Makefile
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/libion/ion
	$(INSTALL_DIR) $(1)/usr/include/libion/linux
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libion.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/libion/ion/*.h $(1)/usr/include/libion/ion/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/libion/linux/*.h $(1)/usr/include/libion/linux/
endef


define Package/libion/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libion.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libion))