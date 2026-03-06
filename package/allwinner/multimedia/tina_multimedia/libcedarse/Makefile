#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libcedarse
PKG_VERSION:=2.9.4
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_LIBCEDARSE_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse
PKG_FILE_DEPENDS += $(PKG_LIBCEDARSE_CBB_SRC_DIR)

define Package/$(PKG_NAME)
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=Audio
  TITLE:= cedarse library
  DEPENDS:= +tinyalsa-lib
endef

define Build/Prepare
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-Werror

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
endef

define Package/libcedarse/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libcedarse.so $(1)/usr/lib/

	$(INSTALL_DIR)  $(1)/etc/cedarSE
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/config/cedarSE/* $(1)/etc/cedarSE/

	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cedarSE_demo $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cedarSE_parm_adj $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libcedarse))
