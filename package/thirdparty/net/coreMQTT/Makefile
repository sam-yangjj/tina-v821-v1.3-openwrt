
# This Makefile is free software, SPDX codes: GPL-2.0-or-later OR MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=coreMQTT
PKG_VERSION:=2.3.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= https://codeload.github.com/FreeRTOS/coreMQTT/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=b8e95044e6ef8381610949b7fe546c5ddf4e52989ce9938209d5dd5f3371b5d7

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/coreMQTT
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=coreMQTT Client Library
  URL:=https://github.com/FreeRTOS/coreMQTT
endef

define Package/coreMQTT/description
  coreMQTT Client Library
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

define Package/coreMQTT/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcoreMQTT.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,coreMQTT))
