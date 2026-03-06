include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tinycedar
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/multimedia_demo/tinycedar

include $(INCLUDE_DIR)/package.mk

TINA_LINUX_API = 1

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina $(PKG_NAME) manager
  DEPENDS:=+libcedarx
endef

define Package/$(PKG_NAME)/description
	Tina $(PKG_NAME) manager
endef

define Package/$(PKG_NAME)/config
  if PACKAGE_tinycedar
    config PACKAGE_tinycedar_static
	bool "tinycedar compile static"
	default n
	help
	  enable tinycedar stastic compile
    config PACKAGE_tinycedar_video_test
	bool "tinycedar video file"
        default n
        help
           copy test h264 file.
  endif
endef

PKG_FILE_DEPENDS := $(SRC_CODE_DIR)
PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_tinycedar_static
PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_tinycedar_video_test

ifeq ($(TARGET_PRODUCT), f133_evb1)
LOCAL_CFLAGS += -D__SUNXI_DISPLAY2__
LOCAL_CFLAGS += -DCONFIG_F133
LOCAL_CFLAGS += -I$(LINUX_DIR)/include/video
endif

ifeq ($(TARGET_PRODUCT), f133-b_evb1)
LOCAL_CFLAGS += -D__SUNXI_DISPLAY2__
LOCAL_CFLAGS += -DCONFIG_F133
LOCAL_CFLAGS += -I$(LINUX_DIR)/include/video
endif

LOCAL_CFLAGS += -DCONF_KERNEL_IOMMU

ifeq ($(LICHEE_USE_INDEPENDENT_BSP),true)
LOCAL_CFLAGS+=-I$(LICHEE_BSP_DIR)/include/video
else
LOCAL_CFLAGS+=-I$(LICHEE_KERN_DIR)/include/video
endif

LOCAL_CFLAGS += -I$(STAGING_DIR)/usr/include/allwinner
LOCAL_CFLAGS += -I$(STAGING_DIR)/usr/include/allwinner/include

ifeq ($(CONFIG_SUNXI_DISPLAY_TWO),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY2__
# TARGET_CFLAGS+=-D$(KERNEL_VERSION)
ifeq ($(LICHEE_USE_INDEPENDENT_BSP),true)
TARGET_CFLAGS+=-I$(LINUX_DIR)/user_headers/include/bsp/video
else
TARGET_CFLAGS+=-I$(LINUX_DIR)/user_headers/include/video
endif
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
ifeq ($(CONFIG_PACKAGE_tinycedar_video_test),y)
	$(CP) -r $(SRC_CODE_DIR)/video $(PKG_BUILD_DIR)/
endif
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LOCAL_CFLAGS="$(LOCAL_CFLAGS)" \
		ENABLE_COMPILE_STATIC="$(CONFIG_PACKAGE_tinycedar_static)" \
		STATIC_LIBRARY_PATH="$(CONFIG_ARCH)_$(CONFIG_LIBC)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tinycedar  $(1)/usr/bin/
ifeq ($(CONFIG_PACKAGE_tinycedar_video_test),y)
	$(INSTALL_DIR) $(1)/usr/share/video
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/video/*.h264 $(1)/usr/share/video
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
