include $(TOPDIR)/rules.mk

PKG_NAME:=libuapi
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/display/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
#config the kernel version,which depends on $(KERNEL_PATCHVER)
ifeq ($(KERNEL_PATCHVER),3.4)
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif

ifeq ($(KERNEL_PATCHVER),3.10)
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif

ifeq ($(KERNEL_PATCHVER),4.4)
	KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif

ifeq ($(KERNEL_PATCHVER),4.9)
	KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif

ifeq ($(KERNEL_PATCHVER),5.4)
	KERNEL_VERSION = CONF_KERNEL_VERSION_5_4
endif

ifeq ($(KERNEL_PATCHVER),5.10)
	KERNEL_VERSION = CONF_KERNEL_VERSION_5_10
	TARGET_CFLAGS+=-DCONF_USE_DMA_HEAP
endif

ifeq ($(KERNEL_PATCHVER),5.15)
	KERNEL_VERSION = CONF_KERNEL_VERSION_5_15
	TARGET_CFLAGS+=-DCONF_USE_DMA_HEAP
endif
ifeq ($(KERNEL_PATCHVER),6.6)
	KERNEL_VERSION = CONF_KERNEL_VERSION_6_6
endif

ifeq ($(CONFIG_SUNXI_DISPLAY_GPU),y)
	GPU_DEPENDS = +libdrm +libgpu
endif

#end config the kernel version

export CONFIG_SUNXI_DRMCOMPOSITOR

define Package/$(PKG_NAME)
  SUBMENU:=Display
  SECTION:=allwinner
  CATEGORY:=Allwinner
  TITLE:=display unit api for Allwinner
  MENU:=1
  DEPENDS:=@PACKAGE_libuapi_config $(GPU_DEPENDS) +SUPPORT_CONFIG_FILE:libuci +libawion \
    +(SUNXI_DRM||SUNXI_DRMCOMPOSITOR):libdrm +SUNXI_DRMCOMPOSITOR:libglogCWrapper
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  unit api for allwinner
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

ifeq ($(CONFIG_SUNXI_DISPLAY_TWO),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY2__
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
ifeq ($(LICHEE_USE_INDEPENDENT_BSP),true)
TARGET_CFLAGS+=-I$(LINUX_DIR)/user_headers/include/bsp/video
else
TARGET_CFLAGS+=-I$(LINUX_DIR)/user_headers/include/video
endif
endif
ifeq ($(CONFIG_SUNXI_DISPLAY_ONE),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY__
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
endif
ifeq ($(CONFIG_SUNXI_DRM),y)
TARGET_CFLAGS+=-D__SUNXI_DRM__
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libdrm
TARGET_LDFLAGS+=-ldrm
endif
ifeq ($(CONFIG_SUNXI_DRMCOMPOSITOR),y)
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libdrm
TARGET_LDFLAGS+=-ldrm
endif

ifeq ($(CONFIG_SUNXI_TRANS_DISPBUF_FD),y)
TARGET_CFLAGS+=-DTRANS_DISPBUF_FD
endif
ifeq ($(CONFIG_SUNXI_TRANS_DISPBUF_PHYADDR),y)
TARGET_CFLAGS+=-DTRANS_DISPBUF_PHYADDR
endif

ifeq ($(CONFIG_LUAPI_LAYER_ALLOC_CHN),y)
TARGET_CFLAGS+=-DLUAPI_LAYER_ALLOC_CHN
endif
ifeq ($(CONFIG_LUAPI_LAYER_ALLOC_LAY),y)
TARGET_CFLAGS+=-DLUAPI_LAYER_ALLOC_LAY
endif

ifeq ($(CONFIG_SUNXI_DISPLAY_GPU),y)
	#TARGET_CFLAGS+=-DSUNXI_DISPLAY_GPU
	TARGET_CFLAGS+=-DSUNXI_DISPLAY_GPU -DSUNXI_DISPLAY_GPU_BUF
	TARGET_LDFLAGS+=-lEGL -lMali -lGLESv2
else
	TPLAYER_EXTRA_CFLAGS+=-DSUNXI_DISPLAY_DE
endif

ifeq ($(CONFIG_SUPPORT_CONFIG_FILE),y)
TARGET_CFLAGS+=-DSUPPORT_CONFIG_FILE
TARGET_LDFLAGS+=-luci
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
ifeq ($(CONFIG_SUPPORT_CONFIG_FILE),y)
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) files/libuapi $(1)/etc/config/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
