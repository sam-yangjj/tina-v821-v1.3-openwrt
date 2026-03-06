include $(TOPDIR)/rules.mk

PKG_NAME := libstdopus
PKG_VERSION := 1.3.1
PKG_RELEASE := 1
#PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/llm/chatbox/libstdopus

PKG_CONFIG_DEPENDS := 
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -DUSE_ALLOCA -DOPUS_BUILD
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm -ldl)

define Package/$(PKG_NAME)
	SECTION := libs
	CATEGORY := Allwinner
	SUBMENU := AiLargeModel
	TITLE := Standard OPUS Audio Codec Library
	URL := https://opus-codec.org/
	MAINTAINER := Allwinner
endef

define Package/$(PKG_NAME)/description
	This package contains the standard OPUS audio codec library.
	OPUS is a versatile audio codec designed for interactive speech and music
	transmission over the Internet.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CPU="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/opus
	$(CP) $(PKG_BUILD_DIR)/opus_header/*.h $(1)/usr/include/opus/
	$(CP) $(PKG_BUILD_DIR)/celt_header/*.h $(1)/usr/include/opus/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libstdopus.a $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libstdopus.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))