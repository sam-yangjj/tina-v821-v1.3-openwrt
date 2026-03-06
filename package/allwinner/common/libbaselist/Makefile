include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=libbaselist
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/common/libbase/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SUBMENU:=Other
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=dynamic list for developer
  DEPENDS:=
  MENU:=
  DEFAULT:=
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  baselist is a basic module copied from linux kernel, it provides dynamic list operation.
endef

define Package/$(PKG_NAME)/config
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/Clean
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/include/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/include
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
