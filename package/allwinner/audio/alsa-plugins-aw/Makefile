include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-plugins-aw
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

AW_PLUGINS_COMPILE_TOOL_CHAIN = arm-openwrt-linux-gnu
AW_PLUGINS_RATE_LIBS_PATH = aw/libs/arm-openwrt-linux-gnu
AW_PLUGINS_EQ_LIBS_PATH = p-eq/libs/arm-openwrt-linux-gnu


ifeq ($(CONFIG_ARCH),"arm")
    ifeq ($(CONFIG_LIBC),"glibc")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = arm-openwrt-linux-gnu
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = arm-openwrt-linux-musl
    endif
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    ifeq ($(CONFIG_LIBC),"glibc")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux-gnu
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux-musl
    endif
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv64")
    ifeq ($(CONFIG_LIBC),"glibc")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-musl
    endif
endif

AW_PLUGINS_RATE_LIBS_PATH = aw/libs/$(AW_PLUGINS_COMPILE_TOOL_CHAIN)
AW_PLUGINS_EQ_LIBS_PATH = p-eq/libs/$(AW_PLUGINS_COMPILE_TOOL_CHAIN)

ifeq ($(CONFIG_ARCH),"riscv32")
    ifeq ($(CONFIG_LIBC),"glibc")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = riscv32-unknown-linux-gnu
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        AW_PLUGINS_COMPILE_TOOL_CHAIN = riscv32-unknown-linux-musl
    endif
    AW_PLUGINS_RATE_LIBS_PATH = aw/libs/$(AW_PLUGINS_COMPILE_TOOL_CHAIN)/$(LICHEE_CHIP)
    AW_PLUGINS_EQ_LIBS_PATH = p-eq/libs/$(AW_PLUGINS_COMPILE_TOOL_CHAIN)/$(LICHEE_CHIP)
endif


define Package/$(PKG_NAME)
  SUBMENU:=Audio
  SECTION:=allwinner
  CATEGORY:=Allwinner
  TITLE:=ALSA plugins from Allwinner
  DEPENDS:=+alsa-lib +ALSA_PLUGINS_AW_AWEQUAL:aactd-lib
endef

define Package/$(PKG_NAME)/config
	menu "Customize plugins"
		depends on PACKAGE_alsa-plugins-aw

	config ALSA_PLUGINS_AW_AWRATE
		bool "awrate"
		default y
	config ALSA_PLUGINS_AW_AWEQUAL
		bool "awequal"
		default y

	endmenu
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_ALSA_PLUGINS_AW_AWRATE \
	CONFIG_ALSA_PLUGINS_AW_AWEQUAL \

TARGET_CFLAGS+=$(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS+=$(if $(CONFIG_USE_GLIBC),-lm)

# The variable $(TARGET_CPU_VARIANT) would be "cortex-a7", "arm926ej-s" or "cortex-a53"
# The variable $(CONFIG_LIBC) would be "musl" or "glibc"
CONFIGURE_ARGS += \
		--with-cpuvariant=$(TARGET_CPU_VARIANT) \
		--with-libc=$(CONFIG_LIBC) \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/$(AW_PLUGINS_RATE_LIBS_PATH) -L$(PKG_BUILD_DIR)/$(AW_PLUGINS_EQ_LIBS_PATH)" \
		--enable-awrate=$(if $(filter $(CONFIG_ALSA_PLUGINS_AW_AWRATE),y),yes,no) \
		--enable-awequal=$(if $(filter $(CONFIG_ALSA_PLUGINS_AW_AWEQUAL),y),yes,no)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/$(AW_PLUGINS_RATE_LIBS_PATH) -L$(PKG_BUILD_DIR)/$(AW_PLUGINS_EQ_LIBS_PATH)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	if [ -d "$(PKG_INSTALL_DIR)/usr/lib" ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/; \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/ $(1)/usr/lib/; \
	fi
endef

define Package/$(PKG_NAME)/install
	if [ -d "$(PKG_INSTALL_DIR)/usr/lib/alsa-lib" ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/alsa-lib; \
		$(CP) \
			$(PKG_INSTALL_DIR)/usr/lib/alsa-lib/libasound_module*.so \
			$(1)/usr/lib/alsa-lib; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
