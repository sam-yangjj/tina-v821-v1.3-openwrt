include $(TOPDIR)/rules.mk

PKG_NAME:=tplayercastdemo
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/tplayercastdemo
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:= tplayercastdemo for pdc-pd1 use H line libaw_dlna.so
  MAINTAINER:=xukaitao <xukaitao@allwinnertech.com>
  DEPENDS:= +libtmedia
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
endef

define Package/$(PKG_NAME)/description
	Tina $(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/dlna $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
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
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/res/misc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dlna/lib/* $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dlna/res/misc/* $(1)/usr/res/misc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tplayer_cast_demo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
