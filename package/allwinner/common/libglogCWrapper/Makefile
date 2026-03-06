include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=libglogCWrapper
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/common/libbase/$(PKG_NAME)
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

# export menuconfig to source makefile
export CONFIG_glogCWrapper_use_glog
export CONFIG_glogCWrapper_use_printf

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SUBMENU:=Other
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=C Wrapper for libglog
  DEPENDS:=@PACKAGE_libglogCWrapper_config +glogCWrapper_use_glog:glog
  MENU:=
  DEFAULT:=
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  libglogCWrapper wraps C++ source code of libglog, provides C API for user to use it conveniently.
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src
	$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/src
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src -f tina.mk \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR)/src ]; then \
		$(MAKE) -C $(PKG_BUILD_DIR)/src -f tina.mk \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			clean; \
	else \
		echo "$(PKG_BUILD_DIR) is not exist!"; \
	fi
endef

define Build/InstallDev
	if [ -d $(PKG_BUILD_DIR)/src/InstallDev ]; then \
		$(CP) -r $(PKG_BUILD_DIR)/src/InstallDev/* $(1); \
	else \
		echo "fatal error! why $(PKG_BUILD_DIR)/InstallDev not exist?"; \
	fi
endef

define Package/$(PKG_NAME)/install
	if [ -d $(PKG_BUILD_DIR)/src/install ]; then \
		$(CP) -r $(PKG_BUILD_DIR)/src/install/* $(1); \
	else \
		echo "fatal error! why $(PKG_BUILD_DIR)/install not exist?"; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
