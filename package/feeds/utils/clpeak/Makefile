#
# Copyright (C) 2007-2015 OpenWrt.org
#
# It is licensed under the GPLv3 (see COPYING).
#

include $(TOPDIR)/rules.mk

PKG_NAME:=clpeak
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/krrishnarraj/clpeak/releases
PKG_MD5SUM:=a627b36fd07676b4c18f809f6374be08

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A tool which profiles OpenCL devices to find their peak capacities
  URL:=https://github.com/krrishnarraj/clpeak
  DEPENDS:= @libgpu +PACKAGE_libgpu:libgpu
endef

TARGET_CXXFLAGS += -DSUNXI_CL_DEVICE_MAX_MEM_ALLOC_SIZE_MB=256 -DSUNXI_CL_DEVICE_GLOBAL_MEM_SIZE_MB=256
TARGET_CXXFLAGS += -lmali

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
