#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ramparser
PKG_VERSION:=2.1.4
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_RAMPARSER_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/utils/ramparser
PKG_FILE_DEPENDS += $(PKG_RAMPARSER_CBB_SRC_DIR)

define Package/ramparser
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= ramparser tools
endef

define Package/ramparser/description
  ramparser tools.
endef

define Build/Prepare
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/utils/ramparser $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-Werror

define Build/Compile/ramparser
	$(MAKE) -C $(PKG_BUILD_DIR)/ramparser \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/ramparser)
endef

define Package/ramparser/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ramparser $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ramparser))
