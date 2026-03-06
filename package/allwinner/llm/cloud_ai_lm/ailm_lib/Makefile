include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/package.mk
-include $(INCLUDE_DIR)/version.mk
-include $(BUILD_DIR)/package.mk

PKG_NAME:=libailm
PKG_VERSION:=0.1
PKG_RELEASE:=1
#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/llm/cloud_ai_lm

define Package/$(PKG_NAME)
  SUBMENU:=AiLargeModel
  CATEGORY:=Libraries
  TITLE:=libailm
  DEPENDS:= +libopenssl +cJSON
endef

define Package/$(PKG_NAME)/description
	Tina ailm lib
endef

define Build/Prepare
	rm $(PKG_BUILD_DIR) -rf
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/core $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/core/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		all
endef

define Build/InstallDev/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/core/*.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/include/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/core/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
