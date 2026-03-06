#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=spidev-test
PKG_RELEASE:=$(LINUX_VERSION)
PKG_FLAGS:=nonshared
PKG_BUILD_DIR:=$(LINUX_DIR)/tools/spi-$(TARGET_DIR_NAME)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/spidev-test
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@!IN_SDK
  TITLE:=SPI testing utility
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
endef

define Package/spidev-test/description
  SPI testing utility.
endef

define Build/Prepare
	$(CP) $(LICHEE_KERN_DIR)/tools/spi/* $(PKG_BUILD_DIR)/

endef

define Build/Compile
    $(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/spidev_test \
         $(PKG_BUILD_DIR)/spidev_test.c
endef

define Package/spidev-test/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spidev_test $(1)/sbin/
endef

$(eval $(call BuildPackage,spidev-test))
