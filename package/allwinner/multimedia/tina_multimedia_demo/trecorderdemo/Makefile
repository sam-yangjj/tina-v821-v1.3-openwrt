include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=trecorderdemo
PKG_VERSION:=1
PKG_RELEASE:=1

#PKG_BUILD_DEPENDS:=libcedarx
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/trecorderdemo

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	comment "$(PKG_NAME) should not be used together with ONLY_ENABLE_AUDIO in libcedarx"
	depends on ONLY_ENABLE_AUDIO
endef

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use trecorder interface in tina_multimedia
  DEPENDS:= @TRECORDER libtmedia +!CEDAR_DISABLE_CEDARC:libuapi
endef

ifeq ($(CONFIG_CEDAR_DISABLE_CEDARC), y)
	CEDAR_DISABLE_CEDARC_CONFIG =
else
	CEDAR_DISABLE_CEDARC_CONFIG = y
endif

define Package/$(PKG_NAME)/description
	CedarX2.8 recorder
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		ENABLE_CEDARC="$(CEDAR_DISABLE_CEDARC_CONFIG)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/ $(1)/etc/res/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/trecorderdemo					 $(1)/usr/bin/
	$(INSTALL_DATA) $(SRC_CODE_DIR)/src/wm_res/*.bmp				$(1)/etc/res/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
