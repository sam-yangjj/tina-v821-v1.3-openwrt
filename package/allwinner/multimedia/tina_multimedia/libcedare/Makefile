include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libcedare
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk
CEDARE_LD_FLAGS = -ldl -lcrypto -lz
CEDARE_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
    CEDARE_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    CEDARE_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv64")
    CEDARE_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
endif

-include ../machinfo/$(LICHEE_IC)/config.mk
-include ../machinfo/$(LICHEE_IC)/config-linux-$(KERNEL_PATCHVER).mk

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)

define Package/$(PKG_NAME)/config
source "$(SOURCE)/../machinfo/$(LICHEE_IC)/$(PKG_NAME)/Config.in"
endef

ifeq ($(CONFIG_CEDARE_AEENC_COMP_DEMO), y)
	CONFIG_DEMO += --enable-aenc_comp_demo=yes
else
	CONFIG_DEMO += --enable-aenc_comp_demo=no
endif

ifeq ($(CONFIG_CEDARE_VENC_COMP_DEMO), y)
	CONFIG_DEMO += --enable-venc_comp_demo=yes
else
	CONFIG_DEMO += --enable-venc_comp_demo=no
endif

ifeq ($(CONFIG_CEDARE_RECORDER_DEMO), y)
	CONFIG_DEMO += --enable-recorder_demo=yes
else
	CONFIG_DEMO += --enable-recorder_demo=no
endif


define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=$(PKG_NAME) for Allwinner
  DEPENDS:=+libstdcpp \
	+libcedarx \
	+libexpat
endef


define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	$(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -Lvr $(SRC_CODE_DIR)/libcedare/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/install --host=$(CEDARE_COMPILE_TOOL_CHAIN) $(CONFIG_DEMO) CFLAGS="-D__ENABLE_ZLIB__" CPPFLAGS="-D__ENABLE_ZLIB__" LDFLAGS="$(CEDARE_LD_FLAGS) -L$(PKG_BUILD_DIR)/install/lib "


endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -Wno-format-overflow -Wno-format -Wno-format-security -Wno-maybe-uninitialized -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARE_LD_FLAGS) -L$(PKG_BUILD_DIR)/install/lib -L$(STAGING_DIR)/usr/lib" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/

	cd $(PKG_INSTALL_DIR)/usr/lib/

	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	echo "**install path:$(1)***"

	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so $(1)/usr/lib

ifeq ($(CONFIG_CEDARE_AEENC_COMP_DEMO), y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/install/bin/aenc_comp_demo			$(1)/usr/bin/
endif
ifeq ($(CONFIG_CEDARE_VENC_COMP_DEMO), y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/install/bin/venc_comp_demo			$(1)/usr/bin/
endif
ifeq ($(CONFIG_CEDARE_RECORDER_DEMO), y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/install/bin/recorder_demo			$(1)/usr/bin/
endif
endef

endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
