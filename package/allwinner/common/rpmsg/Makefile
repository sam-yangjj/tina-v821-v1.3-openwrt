#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rpmsg
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_RPMSG_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/system/rpmsg
PKG_FILE_DEPENDS += $(PKG_RPMSG_CBB_SRC_DIR)

define Package/librpmsg
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= rpmsg library
endef

define Package/librpmsg/description
  rpmsg (Remote Processor Message) framework library provides mechanisms to
  allocate buffers and transmit/receive their references to/from remote
  processor.
endef

define Package/rpmsg_demo
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= rpmsg demo
  DEPENDS:= +librpmsg
endef

define Package/rpmsg_demo/description
  rpmsg demo
endef

define Package/rpmsg_test
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=System
  TITLE:= rpmsg test demo
  DEPENDS:= +librpmsg +libpthread
endef

define Package/rpmsg_test/description
  rpmsg test demo
endef

define Build/Prepare
	$(CP) -r $(LICHEE_PLATFORM_DIR)/allwinner/system/rpmsg/librpmsg $(PKG_BUILD_DIR)/
ifeq ($(CONFIG_PACKAGE_rpmsg_demo),y)
	$(CP) -r $(LICHEE_PLATFORM_DIR)/allwinner/system/rpmsg/demo $(PKG_BUILD_DIR)/
endif
ifeq ($(CONFIG_PACKAGE_rpmsg_test),y)
	$(CP) -r $(LICHEE_PLATFORM_DIR)/allwinner/system/rpmsg/test $(PKG_BUILD_DIR)/
endif
endef

TARGET_CFLAGS+=-Werror

define Build/Compile/librpmsg
	$(MAKE) -C $(PKG_BUILD_DIR)/librpmsg \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/rpmsg_demo
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

define Build/Compile/rpmsg_test
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
	$(Build/Compile/librpmsg)
ifeq ($(CONFIG_PACKAGE_rpmsg_demo),y)
	$(Build/Compile/rpmsg_demo)
endif
ifeq ($(CONFIG_PACKAGE_rpmsg_test),y)
	$(Build/Compile/rpmsg_test)
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/librpmsg/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librpmsg.so $(1)/usr/lib/
endef

define Package/rpmsg_demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpmsg_demo $(1)/usr/bin/
endef

define Package/rpmsg_test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpmsg_test $(1)/usr/bin/
endef

$(eval $(call BuildPackage,librpmsg))
$(eval $(call BuildPackage,rpmsg_demo))
$(eval $(call BuildPackage,rpmsg_test))
