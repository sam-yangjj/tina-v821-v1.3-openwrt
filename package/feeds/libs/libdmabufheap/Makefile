#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdmabufheap
PKG_VERSION:=64ea795d815487b5ae92d9e3aebdfc6c9e8c020c
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://android.googlesource.com/platform/system/memory/$(PKG_NAME)/+archive/$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_HASH:=41c59f84c17ce1cfdd117817e046290e2767619b79fe77a4cfd9369d1441ae81

PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/libdmabufheap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A ion/dmabufheap memory allocation library
  URL:=https://android.googlesource.com/platform/system/memory/
  DEPENDS:=+libstdcpp +libion
endef

define Package/libdmabufheap/description
 Implemented a userspace library that interacts with the kernel DMA-BUF Heap,
 shielding the details of the dma-buf heap operation for the application layer.
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(TAR) -zxvf $(PKG_SOURCE) -C $(PKG_BUILD_DIR)
	$(call Build/Patch/Default)
	$(CP) -r Makefile_src $(PKG_BUILD_DIR)/Makefile
	$(CP) -r Makefile_test $(PKG_BUILD_DIR)/tests/Makefile
	$(CP) -r file/android-base $(PKG_BUILD_DIR)/include/
	$(CP) -r file/logging.cpp $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libion/
TARGET_LDFLAGS+=-lion

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	$(MAKE) -C $(PKG_BUILD_DIR)/tests \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/BufferAllocator
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libdmabufheap.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/BufferAllocator/*.h $(1)/usr/include/BufferAllocator/
endef


define Package/libdmabufheap/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dmabuf_heap_test $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libdmabufheap.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdmabufheap))