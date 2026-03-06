include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=backplaydemo
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/backplaydemo

TINA_LINUX_API = 1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use tplayer interface in tina_multimedia
  DEPENDS:=libtmedia +libstdcpp
endef

define Package/$(PKG_NAME)/description
	a video player that can play in background
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-ldl -lm)

define Build/Compile

	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)" \
		ONLY_ENABLE_AUDIO="$(CONFIG_ONLY_ENABLE_AUDIO)"
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/backplaydemo					 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
