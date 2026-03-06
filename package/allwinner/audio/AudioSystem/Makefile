#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=AudioSystem
PKG_VERSION:=0.1.20
PKG_RELEASE:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/audio/AudioSystem/

ASCORE_LIBS_PATH = arm-openwrt-linux-gnu

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_PCM),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-pcm=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-pcm=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_MULTI_PCM),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-multipcm=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-multipcm=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_AMP),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-amp=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-amp=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_BT),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-bt=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-bt=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_USB),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-usb=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-usb=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_EQ),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-eq=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-eq=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_AW_EQ),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-sweq=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-sweq=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_DRC),y)
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-drc=yes
else
	CONFIG_AUDIO_SYSTEM_HW += --enable-hw-drc=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_TRACK),y)
	CONFIG_AUDIO_SYSTEM_TRACK += --enable-as-track=yes
else
	CONFIG_AUDIO_SYSTEM_TRACK += --enable-as-track=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_RECORD),y)
	CONFIG_AUDIO_SYSTEM_RECORD += --enable-as-record=yes
else
	CONFIG_AUDIO_SYSTEM_RECORD += --enable-as-record=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_UTILS),y)
	CONFIG_AUDIO_SYSTEM_UTILS += --enable-utils=yes
else
	CONFIG_AUDIO_SYSTEM_UTILS += --enable-utils=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_UTILS_LOOP),y)
	CONFIG_AUDIO_SYSTEM_UTILS += --enable-utils-loop=yes
else
	CONFIG_AUDIO_SYSTEM_UTILS += --enable-utils-loop=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_BIT_CONVERT),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-bitconvert=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-bitconvert=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_CHANNEL_MAP),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-channelmap=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-channelmap=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_RATE),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-rate=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-rate=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_SOFT_VOLUME),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-softvol=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-softvol=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_RESAMPLE_ADJUST),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-adjust=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-adjust=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_MIXER_CONTROL),y)
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-mixerctl=yes
else
	CONFIG_AUDIO_SYSTEM_PLUGIN += --enable-plugin-mixerctl=no
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PRASE_CONFIG_PARAMS),y)
	CONFIG_AUDIO_SYSTEM_PRASE_PARMAS += --enable-parse-params=yes
else
	CONFIG_AUDIO_SYSTEM_PRASE_PARMAS += --enable-parse-params=no
endif

ifeq ($(CONFIG_ARCH),"arm")
    ifeq ($(CONFIG_LIBC),"musl")
        ASCORE_LIBS_PATH = arm-openwrt-linux-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        ASCORE_LIBS_PATH = arm-openwrt-linux-gnu
    endif
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    ifeq ($(CONFIG_LIBC),"musl")
        ASCORE_LIBS_PATH = aarch64-openwrt-linux-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        ASCORE_LIBS_PATH = aarch64-openwrt-linux-gnu
    endif
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv32")
    ifeq ($(CONFIG_LIBC),"musl")
        ASCORE_LIBS_PATH = riscv32-unknown-linux-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        ASCORE_LIBS_PATH = riscv32-unknown-linux-gnu
    endif
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv64")
    ifeq ($(CONFIG_LIBC),"musl")
        ASCORE_LIBS_PATH = ariscv64-unknown-linux-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        ASCORE_LIBS_PATH = riscv64-unknown-linux-gnu
    endif
endif

define Package/$(PKG_NAME)
  SUBMENU:=Audio
  SECTION:=libs
  CATEGORY:=Allwinner
  TITLE:=AudioSystem library
  DEPENDS:=+libpthread +tinyalsa-lib +libspeexdsp +COMPONENTS_AW_AUDIO_SYSTEM_PRASE_CONFIG_PARAMS:libjson-c
endef

define Package/$(PKG_NAME)/description
  A library for audio process with mutli phtread read and record.
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

PKG_CONFIG_DEPENDS := \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_PCM \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_MULTI_PCM \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_AMP \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_BT \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_USB \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_EQ \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_AW_EQ \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_DRC \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_TRACK \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_RECORD \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_UTILS \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_UTILS_LOOP \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_BIT_CONVERT \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_CHANNEL_MAP \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_RATE \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_SOFT_VOLUME \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_RESAMPLE_ADJUST \
	CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_PLUGIN_MIXER_CONTROL

CONFIGURE_ARGS+= \
		--disable-debug \
		--without-debug \
		$(CONFIG_AUDIO_SYSTEM_HW) \
		$(CONFIG_AUDIO_SYSTEM_PLUGIN) \
		$(CONFIG_AUDIO_SYSTEM_UTILS) \
		$(CONFIG_AUDIO_SYSTEM_TRACK) \
		$(CONFIG_AUDIO_SYSTEM_RECORD) \
		$(CONFIG_AUDIO_SYSTEM_PRASE_PARMAS) \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		--with-versioned=no

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaudiosystem.{la,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/lib/libs/$(ASCORE_LIBS_PATH)/libaudiosystem_core.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/AudioSystem.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/AudioRecord.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/AudioTrack.h $(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaudiosystem.so.* $(1)/usr/lib/
	if test "$(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_UTILS)" = "y" ; then \
		$(INSTALL_DIR) $(1)/usr/bin; \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/; \
	fi;
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/src/conf/$(LICHEE_IC)/as_config.json \
		$(1)/etc/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
