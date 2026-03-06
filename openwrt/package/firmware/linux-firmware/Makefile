#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-firmware
PKG_RELEASE:=1

PKG_MAINTAINER:=Allwinner
PKG_CONFIG_DEPENDS := CONFIG_FIRMWARE_PATH CONFIG_XR829_USE_40M_SDD CONFIG_XR819S_USE_40M_SDD CONFIG_XR819S_USE_SIP_SDD

SCAN_DEPS = *.mk

ifeq ($(CONFIG_FIRMWARE_PATH),)
FIRMWARE_PATH:=/lib/firmware/
else
FIRMWARE_PATH:=$(CONFIG_FIRMWARE_PATH)
endif

CHIPS_DIR := $(LICHEE_TOP_DIR)/device/config/chips
AMP_FW_PATH := $(shell if [ -d $(CHIPS_DIR)/$(LICHEE_IC)/configs/$(LICHEE_BOARD)/bin ]; then \
echo $(CHIPS_DIR)/$(LICHEE_IC)/configs/$(LICHEE_BOARD)/bin; \
else if [ -d $(CHIPS_DIR)/$(LICHEE_IC)/bin ]; then \
echo $(CHIPS_DIR)/$(LICHEE_IC)/bin; fi fi)

AMP_FW := $(wildcard $(AMP_FW_PATH)/amp_*.bin)
PKG_FILE_DEPENDS += $(AMP_FW)

RTL8821CS_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/rtl8821cs
PKG_FILE_DEPENDS += $(RTL8821CS_FW)

RTL8823DS_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/rtl8723ds
PKG_FILE_DEPENDS += $(RTL8823DS_FW)

XR829_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/xr829
PKG_CONFIG_DEPENDS +=CONFIG_XR829_USE_40M_SDD
PKG_FILE_DEPENDS += $(XR829_FW)

RTL8733BS_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/rtl8733bs
PKG_FILE_DEPENDS += $(RTL8733BS_FW)

XR819S_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/xr819s
PKG_CONFIG_DEPENDS +=CONFIG_XR819S_USE_40M_SDD
PKG_FILE_DEPENDS += $(XR819S_FW)

AIC8800_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/aic8800
PKG_FILE_DEPENDS += $(AIC8800_FW)

XR819_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/xr819
PKG_FILE_DEPENDS += $(XR819_FW)

V821_WLAN_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/v821_wlan
PKG_FILE_DEPENDS += $(V821_WLAN_FW)

V821B_WLAN_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/v821b_wlan
PKG_FILE_DEPENDS += $(V821B_WLAN_FW)

XR819A_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/xr819a
PKG_FILE_DEPENDS += $(XR819A_FW)

TXW901_FW := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/firmware/txw901
PKG_FILE_DEPENDS += $(TXW901_FW)

include $(INCLUDE_DIR)/package.mk

define firmware/config
$(info Config:
  config FIRMWARE_PATH
  string
  prompt "Firmware's directory"
  default "/lib/firmware/"
  help
     Configure the location of firmware for system.

  config XR829_USE_40M_SDD
  bool
  prompt "xr829 with 40M sdd"
  depends on PACKAGE_xr829-firmware
  default n
  help
     Configure to Y to choose 40M firmware for XR829

  config XR819S_USE_40M_SDD
  bool
  prompt "xr819s with 40M sdd"
  depends on PACKAGE_xr819s-firmware
  default n
  help
     Configure to Y to choose 40M firmware for XR819S

  config AIC8800_USE_USB_SDD
  bool
  prompt "aic8800 with usb sdd"
  depends on PACKAGE_aic8800-firmware
  default n
  help
     Configure to Y to choose usb firmware for AIC8800

  config USES_BCMDHD
	bool

  config USES_V821_WLAN
	bool

  config USES_V821B_WLAN
	bool

  config USES_XR829
	bool

  config USES_XR819S
	bool

  config USES_REALTEK
	bool

  config USES_ESP8089
	bool
@@
)
endef

define Package/firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=$(1)
endef


define Build/Compile

endef


include $(wildcard */*.mk)
include $(wildcard ./*.mk)

ifneq ($(DUMP),)
$(call firmware/config,)
endif

#$(eval $(call BuildPackage,linux-firmware))
