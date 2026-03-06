#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=audio-process
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=Audio
  TITLE:= Audio process
endef

define Package/$(PKG_NAME)-common-lib
  $(call Package/$(PKG_NAME))
  TITLE:=Tina audio-process common lib
  DEPENDS:=+alsa-lib
endef

define Package/$(PKG_NAME)-pre-lib
  $(call Package/$(PKG_NAME))
  TITLE:=Tina audio-pre-process lib
  DEPENDS:=+alsa-lib +librpbuf
endef

define Package/$(PKG_NAME)-post-lib
  $(call Package/$(PKG_NAME))
  TITLE:=Tina audio-post-process lib
  DEPENDS:=+alsa-lib +librpbuf +aactd-lib
endef

define Package/$(PKG_NAME)-rpbuf-audio-lib
  $(call Package/$(PKG_NAME))
  TITLE:=Tina rpbuf-audio-process lib
  DEPENDS:=+alsa-lib +librpbuf
endef

define Package/$(PKG_NAME)-pre-demo
  $(call Package/$(PKG_NAME))
  TITLE:=Tina audio-pre-process app demo
  DEPENDS:=audio-process-common-lib audio-process-pre-lib +alsa-lib
endef

define Package/$(PKG_NAME)-post-demo
  $(call Package/$(PKG_NAME))
  TITLE:=Tina audio-post-process app demo
  DEPENDS:=audio-process-common-lib audio-process-post-lib +alsa-lib
endef

define Package/$(PKG_NAME)-rpbuf-aec-demo
  $(call Package/$(PKG_NAME))
  TITLE:=Tina rpbuf aec demo
  DEPENDS:=audio-process-common-lib audio-process-rpbuf-audio-lib +alsa-lib
endef

define Package/$(PKG_NAME)-rpbuf-eq-demo
  $(call Package/$(PKG_NAME))
  TITLE:=Tina rpbuf eq demo
  DEPENDS:=audio-process-common-lib audio-process-rpbuf-audio-lib +alsa-lib
endef

define Package/$(PKG_NAME)-rpbuf-audio-raw-demo
  $(call Package/$(PKG_NAME))
  TITLE:=Tina rpbuf-audio app raw demo
  DEPENDS:=audio-process-common-lib audio-process-rpbuf-audio-lib +alsa-lib
endef

define Package/$(PKG_NAME)/description
  Support audio process. Using Remote Processor Buffer transmit/receive audio data to/from remote dsp
  processor.
endef

define Package/$(PKG_NAME)-common-lib/description
	Tina audio-process-common lib, Support audio input/output common interface.
endef

define Package/$(PKG_NAME)-pre-lib/description
	Tina audio-process-pre lib
endef

define Package/$(PKG_NAME)-post-lib/description
	Tina audio-process-post lib
endef

define Package/$(PKG_NAME)-rpbuf-audio-lib/description
	Tina audio-process-rpbuf-audio lib
endef

define Package/$(PKG_NAME)-pre-demo/description
	Tina audio-pre-process app demo
endef

define Package/$(PKG_NAME)-post-demo/description
	Tina audio-post-process app demo
endef

define Package/$(PKG_NAME)-rpbuf-aec-demo/description
	Tina rpbuf aec demo
endef

define Package/$(PKG_NAME)-rpbuf-eq-demo/description
	Tina rpbuf eq demo
endef

define Package/$(PKG_NAME)-rpbuf-audio-raw-demo/description
	Tina rpbuf-audio app raw demo
endef

define Package/$(PKG_NAME)-common-lib/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-Werror

AUDIO_PROCESS_COMMON_CFLAGS :=

ifeq ($(CONFIG_AUDIO_PROCESS_SUNXI_AUDIO_ONE), y)
	AUDIO_PROCESS_COMMON_CFLAGS += -DAUDIO_PROCESS_SUNXI_AUDIO_ONE
endif

ifeq ($(CONFIG_AUDIO_PROCESS_SUNXI_AUDIO_TWO), y)
	AUDIO_PROCESS_COMMON_CFLAGS += -DAUDIO_PROCESS_SUNXI_AUDIO_TWO
endif

define Build/Compile/$(PKG_NAME)-common-lib
	$(MAKE) -C $(PKG_BUILD_DIR)/src/audio_common \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(AUDIO_PROCESS_COMMON_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-pre-lib
	$(MAKE) -C $(PKG_BUILD_DIR)/src/pre_process \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-post-lib
	$(MAKE) -C $(PKG_BUILD_DIR)/src/post_process \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-rpbuf-audio-lib
	$(MAKE) -C $(PKG_BUILD_DIR)/src/rpbuf_audio \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-pre-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src/utils/pre \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-post-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src/utils/post \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-rpbuf-aec-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src/utils/rpbuf_aec_demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-rpbuf-eq-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src/utils/rpbuf_eq_demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-rpbuf-audio-raw-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src/utils/rpbuf_audio_raw_demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/$(PKG_NAME)-common-lib)
ifeq ($(CONFIG_PACKAGE_audio-process-pre-lib),y)
	$(Build/Compile/$(PKG_NAME)-pre-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-post-lib),y)
	$(Build/Compile/$(PKG_NAME)-post-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-audio-lib),y)
	$(Build/Compile/$(PKG_NAME)-rpbuf-audio-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-pre-demo),y)
	$(Build/Compile/$(PKG_NAME)-pre-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-post-demo),y)
	$(Build/Compile/$(PKG_NAME)-post-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-aec-demo),y)
	$(Build/Compile/$(PKG_NAME)-rpbuf-aec-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-eq-demo),y)
	$(Build/Compile/$(PKG_NAME)-rpbuf-eq-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-audio-raw-demo),y)
	$(Build/Compile/$(PKG_NAME)-rpbuf-audio-raw-demo)
endif
endef

define Build/InstallDev/$(PKG_NAME)-common-lib
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaudiocommon.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/audio_common.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/thread_sem.h $(1)/usr/include
endef

define Build/InstallDev/$(PKG_NAME)-pre-lib
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libpreaec.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/pre_aec.h $(1)/usr/include
endef

define Build/InstallDev/$(PKG_NAME)-post-lib
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libpostequal.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/post_equal.h $(1)/usr/include
endef

define Build/InstallDev/$(PKG_NAME)-rpbuf-audio-lib
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librpbufaudio.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_eq.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_aec.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_raw.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_raw.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_aec.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_eq.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl.h $(1)/usr/include
endef

define Build/InstallDev/$(PKG_NAME)-pre-demo
endef

define Build/InstallDev/$(PKG_NAME)-post-demo
endef

define Build/InstallDev/$(PKG_NAME)-rpbuf-aec-demo
endef

define Build/InstallDev/$(PKG_NAME)-rpbuf-eq-demo
endef

define Build/InstallDev/$(PKG_NAME)-rpbuf-audio-raw-demo
endef

define Build/InstallDev
	$(Build/InstallDev/$(PKG_NAME)-common-lib)
ifeq ($(CONFIG_PACKAGE_audio-process-pre-lib),y)
	$(Build/InstallDev/$(PKG_NAME)-pre-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-post-lib),y)
	$(Build/InstallDev/$(PKG_NAME)-post-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-audio-lib),y)
	$(Build/InstallDev/$(PKG_NAME)-rpbuf-audio-lib)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-pre-demo),y)
	$(Build/InstallDev/$(PKG_NAME)-pre-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-post-demo),y)
	$(Build/InstallDev/$(PKG_NAME)-post-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-aec-demo),y)
	$(Build/InstallDev/$(PKG_NAME)-rpbuf-aec-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-eq-demo),y)
	$(Build/InstallDev/$(PKG_NAME)-rpbuf-eq-demo)
endif
ifeq ($(CONFIG_PACKAGE_audio-process-rpbuf-audio-raw-demo),y)
	$(Build/InstallDev/$(PKG_NAME)-rpbuf-audio-raw-demo)
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR)  $(1)/etc
	$(INSTALL_DATA) $$(PKG_BUILD_DIR)/src/post_process/eq/awequal.conf $(1)/etc
endef

define Package/$(PKG_NAME)-common-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaudiocommon.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/audio_common.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/thread_sem.h $(1)/usr/include
endef

define Package/$(PKG_NAME)-pre-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libpreaec.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/pre_aec.h $(1)/usr/include
endef

define Package/$(PKG_NAME)-post-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libpostequal.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/post_equal.h $(1)/usr/include
endef

define Package/$(PKG_NAME)-rpbuf-audio-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librpbufaudio.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_eq.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_aec.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl_raw.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_raw.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_aec.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_eq.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/rpbuf_audio_ctrl.h $(1)/usr/include
endef

define Package/$(PKG_NAME)-pre-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pre_aec_test $(1)/usr/bin/
endef

define Package/$(PKG_NAME)-post-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/post_eq_test $(1)/usr/bin/
endef

define Package/$(PKG_NAME)-rpbuf-aec-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpbuf_aec_demo $(1)/usr/bin/
endef

define Package/$(PKG_NAME)-rpbuf-eq-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpbuf_eq_demo $(1)/usr/bin/
endef

define Package/$(PKG_NAME)-rpbuf-audio-raw-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpbuf_audio_raw_demo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-common-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-pre-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-post-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-rpbuf-audio-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-pre-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-post-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-rpbuf-aec-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-rpbuf-eq-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-rpbuf-audio-raw-demo))
