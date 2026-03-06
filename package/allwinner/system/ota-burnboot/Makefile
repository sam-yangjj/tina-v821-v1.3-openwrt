#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ota-burnboot
PKG_VERSION:=0.1
PKG_RELEASE:=0.1

include $(INCLUDE_DIR)/package.mk
PKG_FILE_DEPENDS += $(LICHEE_CBBPKG_DIR)/allwinner/system/ota-burnboot/src

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(LICHEE_CBBPKG_DIR)/allwinner/system/ota-burnboot/src $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=burn boot0/uboot for ota.
  DEPENDS:= libc
endef

define Package/$(PKG_NAME)/description
  This package is used for ota to burn boot0/uboot.

  Contains:
   - ota-burnboot0
   - ota-burnuboot
   - libota-burnboot.so
endef

define Package/$(PKG_NAME)/config
	if PACKAGE_$(PKG_NAME)
	menu "Configuration"
		config OTA_BURNBOOT0_TO_EMMC_BOOT_AREA
			bool "burn boot0 to emmc boot area"
			default n
			help
				"burn boot0 to emmc boot area, not boot partition"
		config OTA_BURNBOOT0_DRAM_PARA_NO_UPDATE
			bool "update boot0 para"
			default n
			help
				"update dram para, default not update"

		config OTA_BURNKERNEL
			bool "update kernel image for nand fastboot"
			default n
			help
				"update kernel image only for nand fastboot"

		config OTA_GET_INTO_FROM_EXTERNAL_FILE
			bool "get info from external file"
			default n
			help
				"get info from /mnt/UDISK/ota_info, not /proc/cmdline"
	endmenu
	endif
endef

PKG_CONFIG_DEPENDS += CONFIG_OTA_BURNBOOT0_TO_EMMC_BOOT_AREA CONFIG_OTA_BURNBOOT0_DRAM_PARA_NO_UPDATE \
					CONFIG_OTA_BURNKERNEL OTA_GET_INTO_FROM_EXTERNAL_FILE

ifeq ($(CONFIG_OTA_BURNBOOT0_TO_EMMC_BOOT_AREA),y)
BURNBOOT_CONFIG+=-DOTA_BURNBOOT0_TO_EMMC_BOOT_AREA=1
endif

ifeq ($(CONFIG_OTA_BURNBOOT0_DRAM_PARA_NO_UPDATE),y)
BURNBOOT_CONFIG+=-DOTA_BURNBOOT0_DRAM_PARA_NO_UPDATE=1
endif

ifeq ($(CONFIG_OTA_BURNKERNEL),y)
BURNBOOT_CONFIG+=-DOTA_BURNKERNEL=1
endif

ifeq ($(CONFIG_OTA_GET_INTO_FROM_EXTERNAL_FILE),y)
BURNBOOT_CONFIG+=-DOTA_GET_INTO_FROM_EXTERNAL_FILE=1
endif

define Build/Compile
	$(MAKE) -B -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BURNBOOT_FLAGS="$(BURNBOOT_CONFIG)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/OTA_BurnBoot.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/*.so $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/{ota-burnboot0,ota-burnuboot} $(1)/sbin
ifeq ($(CONFIG_OTA_BURNKERNEL),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ota-burnkernel $(1)/sbin
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
