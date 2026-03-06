include $(TOPDIR)/rules.mk

PKG_NAME:=libpeer
PKG_VERSION:=2.6.0
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libpeer
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libpeer Library
  DEPENDS:=+libmbedtls +cJSON +coreHTTP +coreMQTT +libsrtp +usrsctp +@MBEDTLS_SSL_DTLS_SRTP
endef

define Package/libpeer/description
  peer Library
endef

#CMAKE_BINARY_SUBDIR=build

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=on -DTEST_APPS=off

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

define Package/libpeer/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpeer))
