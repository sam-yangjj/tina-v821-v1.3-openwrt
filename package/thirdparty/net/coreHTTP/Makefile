
# This Makefile is free software, SPDX codes: GPL-2.0-or-later OR MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=coreHTTP
PKG_VERSION:=b539e7
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= https://codeload.github.com/FreeRTOS/coreHTTP/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=955fd60843c01cb7cc7088c71ceee3de29d802292ab5609d6259100c283456ae

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/coreHTTP
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=coreHTTP Client Library
  URL:=https://github.com/FreeRTOS/coreHTTP
endef

define Package/coreHTTP/description
  coreHTTP Client Library
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/interface/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

define Package/coreHTTP/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcoreHTTP.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,coreHTTP))
