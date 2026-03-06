#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lz4
PKG_VERSION:=1.9.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/lz4/lz4/tar.gz/v$(PKG_VERSION)?
PKG_MD5SUM:=3a1ab1684e14fc1afc66228ce61b2db3

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

HOST_PREBUILT:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
		--prefix=$(STAGING_DIR_HOST) \
		--enable-shared \
		--enable-static \

ifeq ($(HOST_OS),Linux)
MAKE_STATIC := STATIC=1
endif

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) $(MAKE_STATIC)
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/lib/lz4.h $(STAGING_DIR_HOST)/include/
	$(CP) $(HOST_BUILD_DIR)/lib/lz4hc.h $(STAGING_DIR_HOST)/include/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/lib/liblz4.a $(STAGING_DIR_HOST)/lib
endef

define Host/Clean
endef

$(eval $(call HostBuild))
