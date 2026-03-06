#
# Copyright (C) 2006-2015 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stressapptest
PKG_VERSION:=1.0.4
PKG_RELEASE:=2

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/thirdparty/utils/stressapptest/

PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

define Package/stressapptest
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=stressapptest is a memory interface test
  URL:=
  DEPENDS:=+libstdcpp
endef

define Package/stressapptest/description
It tries to maximize randomized traffic to memory from processor and I/O, with the intent of creating a realistic high load situation in order to test the existing hardware devices in a computer. It has been used at Google for some time and now it is available under the apache 2.0 license.
endef

CONFIGURE_ARGS += \
	--prefix="/usr"

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
endef

define Package/stressapptest/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stressapptest $(1)/usr/bin/
endef

$(eval $(call BuildPackage,stressapptest))
