
# This Makefile is free software, SPDX codes: GPL-2.0-or-later OR MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=librws
PKG_VERSION:=1.2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:= https://github.com/OlehKulykov/librws/archive/refs/tags/1.2.4.zip
PKG_HASH:=cfbb875720e9f5ec0c26d4f64c59971716224eb4d3f847cf11a2e4c9823d76aa

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Tiny, cross platform websocket client C library.
  URL:=https://github.com/OlehKulykov/librws/archive/refs/tags/1.2.4.zip
  DEPENDS:= +libopenssl
endef

define Package/$(PKG_NAME)/description
	Tiny, cross platform websocket client C library.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
	$(info $(CMAKE_INSTALL_PREFIX))
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/librws.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
