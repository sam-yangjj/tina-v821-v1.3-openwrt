#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lzo
PKG_VERSION:=2.10
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/lzo/download/
PKG_HASH:=c0f892943208266f9b6543b3ae308fab6284c5c90e627931446fb49b4221a072

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
CMAKE_BINARY_SUBDIR:=openwrt-build

HOST_PREBUILT:=1

include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/host-build.mk

CMAKE_OPTIONS += \
	-DENABLE_SHARED=ON \
	-DENABLE_STATIC=ON \
	\
	-DBUILD_TESTING=OFF \
	-DCMAKE_SKIP_INSTALL_RPATH=ON

$(eval $(call HostBuild))
