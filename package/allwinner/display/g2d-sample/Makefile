#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=g2d-sample
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/display/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=g2d-sample
  CATEGORY:=Allwinner
  SUBMENU:=Display
  TITLE:=g2d sample for Allwinner
  DEPENDS:=@DISPLAY_SUPPORT +kmod-sunxi-g2d +libstdcpp +libawion
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

ifeq ($(LICHEE_USE_INDEPENDENT_BSP),true)
TARGET_CFLAGS+=-I$(LINUX_DIR)/user_headers/include/bsp/linux
TARGET_CFLAGS+=-DUSE_BSP_DRIVER
endif
TARGET_LDFLAGS+=-lawion

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/release/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
