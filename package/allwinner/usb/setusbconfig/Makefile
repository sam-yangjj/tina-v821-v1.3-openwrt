#
# Copyright (C) 2007-2016 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=setusbconfig
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/usb/setusbconfig

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=USB
  SECTION:=allwinner
  CATEGORY:=Allwinner
  TITLE:= USB gadget function setting script
endef

PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

define Package/$(PKG_NAME)/description
 USB gadget function setting, support adb,mtp,uac...
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/setusbconfig $(PKG_BUILD_DIR)/
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setusbconfig $(1)/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
