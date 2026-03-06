#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dramstress
PKG_VERSION:=1.0.0

PKG_LICENSE:=MIT
PKG_INSTALL:=1


include $(INCLUDE_DIR)/package.mk

define Package/dramstress
  SECTION:=test
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE:=dramstress is a memory interface test
  DEPENDS:=+stressapptest
endef

define Package/dramstress/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) src/*.sh $(1)/usr/bin/
	$(INSTALL_BIN) dramstress.init $(1)/etc/init.d/dramstress
endef

$(eval $(call BuildPackage,dramstress))
