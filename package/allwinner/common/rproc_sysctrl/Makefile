#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rproc_sysctrl
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_RPMSG_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/system/rpmsg
PKG_FILE_DEPENDS += $(PKG_RPMSG_CBB_SRC_DIR)

define Package/$(PKG_NAME)
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= rproc sysctrl
  DEPENDS:= +librpmsg +libpthread
endef

define Package/$(PKG_NAME)/description
  rproc sysctrl
endef

define Build/Prepare
	$(CP) -r $(LICHEE_PLATFORM_DIR)/allwinner/system/rpmsg/rproc_sysctrl $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/rproc_sysctrl \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
