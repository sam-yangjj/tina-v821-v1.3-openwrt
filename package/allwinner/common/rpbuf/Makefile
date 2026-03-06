#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rpbuf
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_RPBUF_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/system/rpbuf
PKG_FILE_DEPENDS += $(PKG_RPBUF_CBB_SRC_DIR)

define Package/librpbuf
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= RPBuf library
endef

define Package/librpbuf/description
  RPBuf (Remote Processor Buffer) framework library provides mechanisms to
  allocate buffers and transmit/receive their references to/from remote
  processor.
endef

define Package/rpbuf_demo
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= RPBuf demo
  DEPENDS:= +librpbuf
endef

define Package/rpbuf_demo/description
  RPBuf demo
endef

define Package/rpbuf_test
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= RPBuf test demo
  DEPENDS:= +librpbuf
endef

define Package/rpbuf_test/description
  RPBuf test demo
endef

define Build/Prepare
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/system/rpbuf/librpbuf $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/system/rpbuf/demo $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/system/rpbuf/test $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-Werror

define Build/Compile/librpbuf
	$(MAKE) -C $(PKG_BUILD_DIR)/librpbuf \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/rpbuf_demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/rpbuf_test
	$(MAKE) -C $(PKG_BUILD_DIR)/test \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/librpbuf)
ifeq ($(CONFIG_PACKAGE_rpbuf_demo),y)
	$(Build/Compile/rpbuf_demo)
endif
ifeq ($(CONFIG_PACKAGE_rpbuf_test),y)
	$(Build/Compile/rpbuf_test)
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/librpbuf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librpbuf.so $(1)/usr/lib/
endef

define Package/rpbuf_demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpbuf_demo $(1)/usr/bin/
endef

define Package/rpbuf_test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpbuf_test $(1)/usr/bin/
endef

$(eval $(call BuildPackage,librpbuf))
$(eval $(call BuildPackage,rpbuf_demo))
$(eval $(call BuildPackage,rpbuf_test))
