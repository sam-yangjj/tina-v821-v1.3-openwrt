include $(TOPDIR)/rules.mk

PKG_NAME:=libsrtp
PKG_VERSION:=2.6.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/cisco/libsrtp/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=bf641aa654861be10570bfc137d1441283822418e9757dc71ebb69a6cf84ea6b

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libsrtp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libsrtp Client Library
  URL:=https://github.com/cisco/libsrtp
endef

define Package/libsrtp/description
  srtp Library
endef

CMAKE_BINARY_SUBDIR=build

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=on -DTEST_APPS=off

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/srtp2
	$(CP) $(PKG_INSTALL_DIR)/usr/include/srtp2/*.h $(1)/usr/include/srtp2
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

define Package/libsrtp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsrtp))
