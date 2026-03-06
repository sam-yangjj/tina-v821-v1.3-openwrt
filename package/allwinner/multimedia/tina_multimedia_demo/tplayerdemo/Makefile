include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tplayerdemo
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/tplayerdemo
PKG_CONFIG_DEPENDS :=
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use tplayer interface in tina_multimedia
  DEPENDS:=libtmedia +libstdcpp
endef

define Package/$(PKG_NAME)/description
	CedarX2.8 tplayerdemo
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-ldl -lm)

GET_EXTRA_DEMO_CONFIG =
ifeq ($(CONFIG_TPLAYER_POWERKEY_MONITOR), y)
	GET_EXTRA_DEMO_CONFIG += TPLAYER_POWER_MONITOR
endif
ifeq ($(CONFIG_TPLAYER_SUSPEND_TEST), y)
	GET_EXTRA_DEMO_CONFIG += TPLAYER_SUSPEND_TEST
endif

ifeq ($(CONFIG_CEDAR_DISABLE_CEDARC), y)
	CEDAR_DISABLE_CEDARC_CONFIG =
else
	CEDAR_DISABLE_CEDARC_CONFIG = y
endif

PKG_CONFIG_DEPENDS += CONFIG_TPLAYER_POWERKEY_MONITOR \
					  CONFIG_TPLAYER_SUSPEND_TEST


define Build/Compile

	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)" \
		ONLY_ENABLE_AUDIO="$(CONFIG_ONLY_ENABLE_AUDIO)" \
		ENABLE_CEDARC="$(CEDAR_DISABLE_CEDARC_CONFIG)" \
		GET_EXTRA_DEMO_CONFIG="$(GET_EXTRA_DEMO_CONFIG)"
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tplayerdemo					 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
