#
# Copyright (C) 2006-2015 OpenWrt.org
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# ---
#
# The above is the version of the MIT "Expat" License used by X.org:
#
#    http://cgit.freedesktop.org/xorg/xserver/tree/COPYING
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk
include ../sunxifb.mk

PKG_NAME:=lv_86_boxes
PKG_VERSION:=8.3.2
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/thirdparty/gui/lvgl-8/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Littlevgl
  CATEGORY:=Gui
  DEPENDS:=+LVGL8_USE_SUNXIFB_G2D:libuapi +LVGL8_USE_SUNXIFB_G2D:kmod-sunxi-g2d \
           +LVGL8_USE_FREETYPE:libfreetype +LVGL8_86_BOX_USE_WIFI2_BT4:wifimanager-v2.0-lib \
           +LVGL8_86_BOX_USE_WIFI2_BT4:btmanager-v4.0 +LVGL8_86_BOX_USE_WIFI2_BT4:libjson-c \
           +LVGL8_86_BOX_USE_WIFI2_BT4:glib2 +libpthread +libtmedia
  TITLE:=lvgl 86 box example use lvgl-8.3.2
  MAINTAINER:=anruliu <anruliu@allwinnertech.com>
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_LVGL8_USE_SUNXIFB_DOUBLE_BUFFER \
	CONFIG_LVGL8_USE_SUNXIFB_CACHE \
	CONFIG_LVGL8_USE_SUNXIFB_G2D \
	CONFIG_LVGL8_USE_SUNXIFB_G2D_ROTATE

define Package/$(PKG_NAME)/config
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  a lvgl monitor v8.3.2
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/res $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/../lvgl $(PKG_BUILD_DIR)/src/
	$(CP) -r $(SRC_CODE_DIR)/../lv_drivers $(PKG_BUILD_DIR)/src/
endef

define Build/Configure
endef

TARGET_CFLAGS+=-I$(PKG_BUILD_DIR)/src

ifeq ($(CONFIG_LVGL8_USE_SUNXIFB_G2D),y)
TARGET_CFLAGS+=-DLV_USE_SUNXIFB_G2D_FILL \
				-DLV_USE_SUNXIFB_G2D_BLEND \
				-DLV_USE_SUNXIFB_G2D_BLIT \
				-DLV_USE_SUNXIFB_G2D_SCALE
endif

ifeq ($(CONFIG_LVGL8_86_BOX_USE_WIFI2_BT4),y)
TARGET_CFLAGS+=-DLV_USE_LINUX_WIFI2_MODE -DLV_USE_LINUX_BT4_MODE \
				-DLV_USE_LINUX_TPLAYER -DLV_USE_LINUX_DISPALY
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/allwinner \
				-I$(STAGING_DIR)/usr/include/allwinner/include
TARGET_LDFLAGS+=-lwifimg-v2.0 -lwirelesscom \
				-lbtmg -lshared-mainloop -lbluetooth -lbluetooth-internal -lgio-2.0 -ljson-c -ldbus-1 \
				-lpthread -ltplayer
else
TARGET_CFLAGS+=-DLV_USE_LINUX_TPLAYER -DLV_USE_LINUX_DISPALY
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/allwinner \
				-I$(STAGING_DIR)/usr/include/allwinner/include
TARGET_LDFLAGS+=-lpthread -ltplayer
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/lib
#endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/lv_86_boxes
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/res/* $(1)/usr/share/lv_86_boxes
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
