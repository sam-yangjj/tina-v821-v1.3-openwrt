#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lark
PKG_VERSION:=1.1.5
PKG_RELEASE:=2

PYPI_NAME:=lark
PKG_HASH:=4b534eae1f9af5b4ea000bea95776350befe1981658eea3820a01c37e504bb4d

PKG_MAINTAINER:=erezshin@gmail.com
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-$(PKG_NAME)
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=a modern parsing library
  URL:=https://pypi.org/project/lark
  DEPENDS:=+python3
endef

define Package/python3-$(PKG_NAME)/description
  Lark is a modern general-purpose parsing library for Python.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call Py3Package,python3-$(PKG_NAME)))
$(eval $(call BuildPackage,python3-$(PKG_NAME)))
