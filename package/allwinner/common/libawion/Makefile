include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libawion
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/common/$(PKG_NAME)
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

PKG_BUILD_DEPENDS := libbaselist

include $(INCLUDE_DIR)/package.mk

CONFIG_IN_DIR := default
ifneq ($(filter v821%, $(LICHEE_IC)),)
    CONFIG_IN_DIR := v821
endif
ifneq ($(filter v861%, $(LICHEE_IC)),)
    CONFIG_IN_DIR := v861
endif

KERNEL_VERSION = CONF_KERNEL_VERSION_LATEST

#config the kernel version,which depends on $(LICHEE_KERN_VER)
ifeq ($(LICHEE_KERN_VER),linux-3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif
ifeq ($(LICHEE_KERN_VER),linux-3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif
ifeq ($(LICHEE_KERN_VER),linux-4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif
ifeq ($(LICHEE_KERN_VER),linux-4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif
ifeq ($(LICHEE_KERN_VER),linux-5.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4
endif

ifeq ($(LICHEE_KERN_VER),linux-5.10-origin)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_10
endif

ifeq ($(LICHEE_KERN_VER),linux-5.10-rt)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_10
endif

ifeq ($(LICHEE_KERN_VER),linux-5.4-ansc)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4_ANSC
endif
ifneq (,$(findstring linux-5.15,$(LICHEE_KERN_VER)))
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_15
endif
ifeq ($(LICHEE_KERN_VER),linux-6.6-xuantie)
    KERNEL_VERSION = CONF_KERNEL_VERSION_6_6
endif

export KERNEL_VERSION

#end config the kernel version
TARGET_CFLAGS+=-D$(KERNEL_VERSION)


define Package/$(PKG_NAME)
  SUBMENU:=Other
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=ion api for multimedia
  MENU:=1
  DEPENDS:=+libbaselist
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  ion api for allwinner
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/machinfo/$(CONFIG_IN_DIR)/Config.in"
endef

ifeq ($(CONFIG_AW_ION_ALLOC_IOMMU),y)
TARGET_CFLAGS+=-DCONF_KERNEL_IOMMU
endif
ifeq ($(CONFIG_AW_ION_ALLOC_CMA),y)
TARGET_CFLAGS+=-DCONF_KERNEL_CMA
endif
ifeq ($(CONFIG_AW_ION_ALLOC_POOL),y)
TARGET_CFLAGS+=-DCONF_AWION_SIZE_POOL_HEAP
endif

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
endef

ifeq ($(CONFIG_PACKAGE_rt_media),y)
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/lib/
endef
else
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef
endif

$(eval $(call BuildPackage,$(PKG_NAME)))
