#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libyaml
PKG_VERSION:=0.2.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:pyyaml_project:pyyaml

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libyaml
  SUBMENU:=Languages
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Fast YAML 1.1 parser and emitter library
  URL:=https://pyyaml.org/wiki/LibYAML
endef

CMAKE_OPTIONS += \
        -DBUILD_SHARED_LIBS=ON \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/cmake
	$(CP) $(PKG_INSTALL_DIR)/usr/cmake/* $(1)/usr/cmake/
endef

define Package/libyaml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/cmake
	$(CP) $(PKG_INSTALL_DIR)/usr/cmake/* $(1)/usr/cmake/
endef

$(eval $(call BuildPackage,libyaml))
