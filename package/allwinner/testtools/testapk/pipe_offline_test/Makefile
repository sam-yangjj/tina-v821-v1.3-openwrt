include $(TOPDIR)/rules.mk

PKG_NAME:=pipe_offline_test
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	config PACKAGE_$(PKG_NAME)_ENABLE_VIN_ISP
		bool "Enable vin isp support"
		default n
	help
		Enable vin isp support.
endef

define Package/$(PKG_NAME)
  SECTION:=test
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE:=pipe_offline_test demo
  DEPENDS:=+PACKAGE_$(PKG_NAME)_ENABLE_VIN_ISP:libAWIspApi +libcedarc \
           +libdmabufheap +libion +libawion +ai-sdk-viplite
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

TINA_VIPLITE_DIR:= viplite-tina
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		USE_VIN_ISP="$(CONFIG_PACKAGE_$(PKG_NAME)_ENABLE_VIN_ISP)" \
		C_LIB_TYPE=$(CONFIG_LIBC) \
		NPU_VERSION=$(NPU_VERSION) \
		NPU_SW_VERSION=$(NPU_SW_VERSION) \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) 	$(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
