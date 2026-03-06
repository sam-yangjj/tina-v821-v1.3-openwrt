include $(TOPDIR)/rules.mk

PKG_NAME:=ramparser-bin
PKG_VERSION:=1.3.2
PKG_RELEASE:=1.3.2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ramparser-bin
  SECTION:=test
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE:=ramparser tools
endef


CHIP_DIR := $(LICHEE_CHIP)
BIN_PATH := $(if $(CONFIG_USE_GLIBC),glibc,musl)

define Package/ramparser-bin/description
  ramparser tools.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./$(CHIP_DIR)/$(BIN_PATH)/ramparser $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/ramparser-bin/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ramparser $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ramparser-bin))
