include $(TOPDIR)/rules.mk

PKG_NAME:=uvcout
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/uvcout


include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  MENU:=1
  TITLE:=uvcout middle
  DEPENDS:= +libcedarx +libAWIspApi
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
		TARGET="$(PKG_NAME)" \
		all
endef
#TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt -lstdc++)


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) 	$(1)/usr/bin/
	$(CP) $(SRC_CODE_DIR)/src/uvc_gadget.sh $(1)/etc/
	$(CP) $(SRC_CODE_DIR)/src/sample_uvcout.conf $(1)/etc/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

