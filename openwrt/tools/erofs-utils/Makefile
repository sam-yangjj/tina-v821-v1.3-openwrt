#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=erofs-utils
PKG_VERSION:=1.6
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/xiang/erofs-utils.git
PKG_MD5SUM:=b7bddcc5551347270ed3662e90ec2b54

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/erofs-utils-$(PKG_VERSION)

HOST_PREBUILT:=1

HOST_FIXUP:=autoreconf

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_LDFLAGS += -Wl,-rpath=$(STAGING_DIR_HOST)/lib/

HOST_CONFIGURE_ARGS += \
		--prefix=$(STAGING_DIR_HOST) \
		--enable-shared \
		--enable-static \
		--without-uuid

ifeq ($(HOST_OS),Linux)
MAKE_STATIC := STATIC=1
endif

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) $(MAKE_STATIC)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mkfs/mkfs.erofs $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mkfs.erofs
endef

$(eval $(call HostBuild))
