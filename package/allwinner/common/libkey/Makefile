#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libkey
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/common/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk
PKG_CONFIG_DEPENDS := \
	CONFIG_SUNXI_SUPPORT_PRIVATE_KEY \
	CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K \
	CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_TAIL_8K \
	CONFIG_SUNXI_SUPPORT_SECURE_STORAGE_KEY \

define Package/$(PKG_NAME)
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Allwinner secure storage library
  DEPENDS:=
endef

define Package/$(PKG_NAME)/description
  Allwinner secure storage library

    Contains:
   - lisst.so
   - sst_test
endef

define Package/$(PKG_NAME)/config
config SUNXI_SUPPORT_PRIVATE_KEY
	bool "support private key"
	depends on PACKAGE_libkey
	default n

if SUNXI_SUPPORT_PRIVATE_KEY
choice
prompt "private bare key mode"
	default SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K
	help
	select the store mode of private bare

config SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K
	bool "Enable private bare key first 8K support"

config SUNXI_SUPPORT_PRIVATE_BARE_TAIL_8K
	bool "Enable private bare key tail 8K support"

endchoice
endif

config SUNXI_SUPPORT_SECURE_STORAGE_KEY
	bool "Enable secure storage key support"
	default n
	depends on PACKAGE_libkey

endef

define Package/$(PKG_NAME)-demo
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=libkey-demo
  DEPENDS:= libkey
endef

define Package/$(PKG_NAME)-demo/description
  sst_test tool
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(LICHEE_BSP_DIR)/include -I$(LICHEE_KERN_DIR)/include

TARGET_LDFLAGS +=
#config the key type
ifeq ($(CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_PRIVATE_BARE_KEY
endif

ifeq ($(CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_TAIL_8K),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_PRIVATE_BARE_KEY -DCONF_SUNXI_PRIVATE_BARE_TAIL_8K
endif

ifeq ($(CONFIG_SUNXI_SUPPORT_SECURE_STORAGE_KEY),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_SECURE_STORAGE_KEY
endif

define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/$(PKG_NAME))

ifeq ($(CONFIG_PACKAGE_libkey-demo),y)
	$(Build/Compile/$(PKG_NAME)-demo)
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/

	$(CP) $(PKG_BUILD_DIR)/src/libsst.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/

	$(CP) $(PKG_BUILD_DIR)/src/libsst.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin/

	$(CP) $(PKG_BUILD_DIR)/demo/sst_test $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
