#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=parallel_insmod
PKG_RELEASE:=1.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_MODULE_DEPEND_FILE := mods.dep
PKG_EARLY_MODULE_DEPEND_FILE := mods.early.dep

include $(INCLUDE_DIR)/package.mk

define Package/parallel_insmod
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libpthread
  TITLE:=parallel insmod
endef

define Package/parallel_insmod/config
  source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	python3 ./files/modparse.py \
			$(LICHEE_TOP_DIR)/out/kernel/staging \
			$(LICHEE_KERNEL_VERSION) \
			--builtin=$(LICHEE_TOP_DIR)/out/kernel/build/modules.builtin \
			--input=$(LICHEE_TOP_DIR)/out/kernel/build/modules.order \
			--output=./files/$(PKG_MODULE_DEPEND_FILE) \
			--early=$(CONFIG_PARALLEL_INSMOD_EARLY_MODS) \
			--earlyout=./files/$(PKG_EARLY_MODULE_DEPEND_FILE) \
			2>&1 > /dev/null
	$(CP) ./files/$(PKG_MODULE_DEPEND_FILE) $(PKG_BUILD_DIR)/
	$(CP) ./files/async_modules $(PKG_BUILD_DIR)/
	-$(CP) ./files/$(PKG_EARLY_MODULE_DEPEND_FILE) $(PKG_BUILD_DIR)/
	-$(CP) ./files/early_async_modules $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/parallel_insmod/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_MODULE_DEPEND_FILE) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/async_modules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/parallel_insmod $(1)/usr/sbin/
	$(if $(wildcard $(PKG_BUILD_DIR)/$(PKG_EARLY_MODULE_DEPEND_FILE)),
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_EARLY_MODULE_DEPEND_FILE) $(1)/etc/config
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/early_async_modules $(1)/etc/init.d
	,)
endef

$(eval $(call BuildPackage,parallel_insmod))
