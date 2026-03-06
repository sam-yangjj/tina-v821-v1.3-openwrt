
# This Makefile is free software, SPDX codes: GPL-2.0-or-later OR MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=usrsctp
PKG_VERSION:=0.9.5.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= https://codeload.github.com/sctplab/usrsctp/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=260107caf318650a57a8caa593550e39bca6943e93f970c80d6c17e59d62cd92

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/usrsctp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=sctp stack
  URL:=https://github.com/sctplab/usrsctp
endef

define Package/cJSON/description
  this is a userland SCTP stack
endef

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=on

#Provide compilation dependencies for other modules to call
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/usrsctp.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusrsctp.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/usrsctp.pc $(1)/usr/lib/pkgconfig
	$(SED) 's,/usr,$(STAGING_DIR)/usr,g' $(1)/usr/lib/pkgconfig/usrsctp.pc
endef

#Installed to the root file system for call at runtime
define Package/usrsctp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusrsctp.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,usrsctp))
