include $(TOPDIR)/rules.mk
-include mpp_config.mk

PKG_NAME:= chatbox_xiaozhi
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
#PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/llm/chatbox/xiaozhi

PKG_CONFIG_DEPENDS:=

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Package/$(PKG_NAME)
	SUBMENU := AiLargeModel
	CATEGORY := Allwinner
	TITLE := xiaozhi chat box
	DEPENDS := +cJSON +librws +libopenssl +libstdcpp +libawion \
				+rt_media +eyesee-mpp-middleware +libexpat \
				+eyesee-mpp-private-rtsp +rt_media-demo-common \
				+wirelesscommon +wifimanager-v2.0-lib +libstdopus
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)/description
	this is chat box for xiaozhi application.
endef

LVGL_SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/thirdparty/gui/lvgl-8

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
ifeq ($(CONFIG_CHATBOX_XIAOZHI_DISPLAY),y)
	$(CP) -r $(SRC_CODE_DIR)/port/lvgl/*.h $(PKG_BUILD_DIR)/
	$(CP) -r $(LVGL_SRC_CODE_DIR)/lvgl $(PKG_BUILD_DIR)/
	$(CP) -r $(LVGL_SRC_CODE_DIR)/lv_drivers $(PKG_BUILD_DIR)/
endif
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm -ldl)

define Build/Configure
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_CHATBOX_XIAOZHI_DISPLAY

ifeq ($(CONFIG_PACKAGE_chatbox_xiaozhi),y)
define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_LIBC="$(CONFIG_LIBC)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CPU="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		CONFIG_CHATBOX_XIAOZHI_DISPLAY=$(CONFIG_CHATBOX_XIAOZHI_DISPLAY) \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chatbox_xiaozhi $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/port/sample_takePicture.conf $(1)/etc/sample_takePicture.conf
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
