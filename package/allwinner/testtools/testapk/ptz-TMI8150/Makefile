include $(TOPDIR)/rules.mk

PKG_NAME:=ptz-TMI8150
PKG_VERSION:=1
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=test
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE	:=TMI8150 ptz drivet
  DEPENDS:=
endef

define Package/$(PKG_NAME)/description
	TMI8150 PTZ driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#libpeq.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ptz $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
