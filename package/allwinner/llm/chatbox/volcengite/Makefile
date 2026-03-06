include $(TOPDIR)/rules.mk
-include mpp_config.mk

PKG_NAME:= chatbox_volcengite
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
#PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/llm/chatbox/volcengite

PKG_CONFIG_DEPENDS:=

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Package/$(PKG_NAME)
	SUBMENU := AiLargeModel
	CATEGORY := Allwinner
	TITLE := volcengite chat box
	DEPENDS := +cJSON +librws +libopenssl +libstdcpp +libawion \
				+eyesee-mpp-middleware +libexpat \
				+eyesee-mpp-private-rtsp +ntpdate +zoneinfo-asia + +libstdopus
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)/description
	this is chat box for volcengite doubao application.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm -ldl)

define Build/Configure
endef

ifeq ($(CONFIG_CHATBOX_VOLCENGITE_VISION),y)
	CHATBOX_VOLCENGITE_VISION := y
endif
export CHATBOX_VOLCENGITE_VISION

ifeq ($(CONFIG_CHATBOX_VOLCENGITE_LOCAL_PREVIEW),y)
	CHATBOX_VOLCENGITE_LOCAL_PREVIEW := y
endif
export CHATBOX_VOLCENGITE_LOCAL_PREVIEW

ifeq ($(CONFIG_PACKAGE_chatbox_volcengite),y)
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
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib/libVolcEngineRTCLite.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chatbox_volcengite $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/volcengite
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtc_bot_config.json $(1)/etc/volcengite/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
