include $(TOPDIR)/rules.mk
-include machinfo/$(LICHEE_IC)/build.mk

PKG_NAME:=libAWIspApi
PKG_VERSION:=1
PKG_RELEASE:=1
LIBISP_DEPENDS:=

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

ifeq ($(ISP_SRC_IN_CBB), y)
SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/vision/libAWIspApi/$(ISP_DIR)
else
SRC_CODE_DIR := ./src/$(ISP_DIR)
endif

PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

ifeq ($(ISP_LIB_USE_IN_MPP), y)
-include machinfo/$(LICHEE_IC)/libisp_config.mk

LIBISP_DEPENDS += +libcedarc +libawion +libglogCWrapper
export LICHEE_PLATFORM_OS:=tina

define Package/$(PKG_NAME)/config
source "$(SOURCE)/machinfo/$(LICHEE_IC)/Config.in"
endef

endif # ISP_LIB_USE_IN_MPP

define Package/$(PKG_NAME)
  SUBMENU:=Vision
  SECTION:=allwinner
  CATEGORY:=Allwinner
  TITLE:=Camera VIN ISP api for Allwinner
  DEPENDS:=$(LIBISP_DEPENDS)
endef

define Package/$(PKG_NAME)/description
  Camera VIN ISP api for allwinner
endef

ifeq ($(ISP_VERSION),isp601)
TARGET_CFLAGS += -D__ISP_VERSION_601__

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "10.2.0")
LIB_GCC_VERSION="-gcc10_2_0"
else ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "11.3.0")
LIB_GCC_VERSION="-gcc11_3_0"
else ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "13.2.0")
LIB_GCC_VERSION="-gcc13_2_0"
endif
else ifeq ($(ISP_VERSION),isp606)
TARGET_CFLAGS += -D__ISP_VERSION_606__

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "13.2.0")
LIB_GCC_VERSION="-gcc13_2_0"
endif
endif

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src
	$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/src
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		ISP_VERSION="$(ISP_VERSION)" \
		C_LIB_TYPE="$(CONFIG_LIBC)$(LIB_GCC_VERSION)$(if $(CONFIG_EXT_TOOLCHAIN_GCC_8_3_0),-gcc8_3_0)" \
		CPU_VARIANT="$(TARGET_CPU_VARIANT)$(if $(CONFIG_COMPLILE_KERNEL64_USER32),-user-32bit)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
endef

ifeq ($(ISP_LIB_USE_IN_MPP), y)
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib/
ifneq ($(findstring y,$(CONFIG_libisp_build_libispapi)),)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libAWIspApi.so $(1)/usr/lib/
endif
ifneq ($(findstring y,$(CONFIG_libisp_compile_dynamic_lib)),)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endif
ifneq ($(findstring y,$(CONFIG_libisp_build_tunning_app)),)
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(SRC_CODE_DIR)/libisp/tuning_app
endif
endef
else
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef
endif

$(eval $(call BuildPackage,$(PKG_NAME)))
