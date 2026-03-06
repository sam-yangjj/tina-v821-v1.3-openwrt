include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME := pqd
PKG_VERSION := 0.0.1
PKG_RELEASE := 1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/display/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
TARGET_CFLAGS += -std=gnu99 -Wall -Wno-unused-function -I./hardwares/de20x/ -I./ -Iutils -I./include -g -DFCM_10BIT_ENABLE
TARGET_LDFLAGS += -lpthread -lm
#TARGET_LDFLAGS += -lasan
#TARGET_CFLAGS += -fsanitize=leak -fsanitize=address -fno-omit-frame-pointer -g

include $(INCLUDE_DIR)/package.mk

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

define Package/$(PKG_NAME)
  SUBMENU:=Display
  CATEGORY := Allwinner
  TITLE := pqd
  DEPENDS := +libc +(SUNXI_DRM||SUNXI_DRMCOMPOSITOR):libdrm
endef

ifneq ($(or $(CONFIG_SUNXI_DRM),$(CONFIG_SUNXI_DRMCOMPOSITOR)),)
	TARGET_CFLAGS+=-D__SUNXI_DRM__
	TARGET_CFLAGS+=-D$(KERNEL_VERSION)
	TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libdrm
	TARGET_LDFLAGS+=-ldrm
endif

define Package/$(PKG_NAME)/description
  tigerLCD application
endef

define Build/Prepare
	@$(INSTALL_DIR) $(PKG_BUILD_DIR)/src
	@$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/src
endef

define Build/Compile
	@$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/pqd $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
