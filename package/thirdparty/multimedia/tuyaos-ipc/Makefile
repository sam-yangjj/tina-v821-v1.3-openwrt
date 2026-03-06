include $(TOPDIR)/rules.mk

PKG_NAME:=tuyaos-ipc
PKG_VERSION:=3.10.16
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_v821b_wlan-firmware \
	CONFIG_TUYAOS_IPC_LOWPOWER

include $(INCLUDE_DIR)/package.mk

LOCAL_FLAGS += CONFIG_PACKAGE_v821b_wlan-firmware=$(CONFIG_PACKAGE_v821b_wlan-firmware)
LOCAL_FLAGS += CONFIG_TUYAOS_IPC_LOWPOWER=$(CONFIG_TUYAOS_IPC_LOWPOWER)

define Package/$(PKG_NAME)
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:= +libnl-tiny +wirelesscommon +librpbuf +librpmsg +wifimanager-v2.0-lib +libjson-c \
			+eyesee-mpp-middleware +libzbar +libawion +libexpat +libunwind +alsa-lib +libiconv +glog \
			+TUYAOS_IPC_LOWPOWER:rt_media \
			+PACKAGE_v821b_wlan-firmware:btmanager-v4.0
  TITLE:=tuyaos-ipc for tina linux
endef

define Package/$(PKG_NAME)/config
	if PACKAGE_tuyaos-ipc
		choice
			prompt "tuya os ipc support mode"
			default TUYAOS_IPC
		config TUYAOS_IPC
			bool "tuya os ipc"
		config TUYAOS_IPC_LOWPOWER
			bool "tuya os ipc with lowpower fastboot"
		endchoice
	endif
endef

define Package/$(PKG_NAME)/description
  tuyaos-ipc for tina linux
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		KERNEL_DIR="${LICHEE_TOP_DIR}/out/${LICHEE_IC}/kernel/build" \
		$(LOCAL_FLAGS) \
		all
endef

ifneq (,$(CONFIG_TUYAOS_IPC))
  FILE_DIR:= ipc
else ifneq (,$(CONFIG_TUYAOS_IPC_LOWPOWER))
  FILE_DIR:= ipc_lowpower
else
  FILE_DIR:=
endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tuyaos_demo_ipc $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/files/$(FILE_DIR)/etc/* $(1)/etc
	$(CP) $(PKG_BUILD_DIR)/files/$(FILE_DIR)/bin/* $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
