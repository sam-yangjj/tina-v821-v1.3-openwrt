include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/package.mk
-include $(INCLUDE_DIR)/version.mk
-include $(BUILD_DIR)/package.mk

PKG_NAME:=wifimanager-v2.0
PKG_VERSION:=0.1
PKG_RELEASE:=1

# VERSION_CODE has value only on Tina5.0 or later
ifneq ($(VERSION_CODE),)
SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/wifimanager
TARGET_CFLAGS += -DTINA_5_0
else
SRC_CODE_DIR := ./
endif

PKG_CONFIG_DEPENDS:= \
	CONFIG_WMG_PLATFORM_LINUX \
	CONFIG_WMG_PLATFORM_XRLINK \
	CONFIG_WMG_CONFIG_PATH \
	CONFIG_WMG_SUPPORT_EXPAND \
	CONFIG_WMG_SUPPORT_STA_MODE \
	CONFIG_WMG_SUPPORT_AP_MODE \
	CONFIG_WMG_SUPPORT_MONITOR_MODE \
	CONFIG_WMG_SUPPORT_P2P_MODE \
	CONFIG_WMG_PROTOCOL_BLE \
	CONFIG_WMG_PROTOCOL_JL6328 \
	CONFIG_WMG_PROTOCOL_SOFTAP \
	CONFIG_WMG_PROTOCOL_XCONFIG \
	CONFIG_WMG_PROTOCOL_SOUNDWAVE \
	CONFIG_WMG_PROTOCOL_QRCODE \
	CONFIG_PACKAGE_kmod-net-xr819 \
	CONFIG_PACKAGE_kmod-net-xr819s \
	CONFIG_PACKAGE_kmod-net-xr829 \
	CONFIG_PACKAGE_kmod-net-aic8800 \
	CONFIG_LINUX_4_9 \
	CONFIG_LINUX_5_4 \
	CONFIG_LINUX_5_10 \
	CONFIG_LINUX_5_15 \
	CONFIG_LINUX_6_6 \
	CONFIG_WMG_DEMO_TEST_TOOL \
	CONFIG_WMG_CMD_SOCKET_ACK \
	CONFIG_WMG_LOCK_MAC_ADDR \
	CONFIG_WMG_DEMO_STA \
	CONFIG_WMG_DEMO_AP \
	CONFIG_WMG_DEMO_MONITOR \
	CONFIG_WMG_DEMO_P2P \
	CONFIG_WMG_DEMO_STA_AP_COEXIST \
	CONFIG_WMG_DEMO_NET_CONFIG \
	CONFIG_WMG_DEMO_FRAME

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Package/$(PKG_NAME)/Default
  SUBMENU:=Wireless
  CATEGORY:=Allwinner
  TITLE:=Tina wifimanager-v2.0
endef

define Package/$(PKG_NAME)
  MENU:=1
  DEFAULT:=
  $(call Package/wifimanager-v2.0/Default)
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)-demo/config
source "$(SOURCE)/Config-demo.in"
endef

define Package/$(PKG_NAME)-lib
  $(call Package/wifimanager-v2.0/Default)
  TITLE:=Tina wifimanager-v2.0 lib
  DEPENDS:= wifimanager-v2.0 +libpthread +libnl-tiny +wirelesscommon \
            +WMG_PROTOCOL_BLE:btmanager-v4.0 \
            +WMG_PROTOCOL_BLE:libjson-c \
            +WMG_PROTOCOL_JL6328:libjson-c \
            +WMG_PROTOCOL_XCONFIG:libopenssl \
            +WMG_PROTOCOL_SOUNDWAVE:alsa-lib \
            +WMG_PROTOCOL_SOUNDWAVE:libuci \
            +WMG_PROTOCOL_QRCODE:eyesee-mpp-middleware \
            +WMG_PROTOCOL_QRCODE:libzbar \
            +WMG_PROTOCOL_QRCODE:libawion \
            +WMG_PROTOCOL_QRCODE:libexpat \
            +WMG_PROTOCOL_QRCODE:libunwind \
            +WMG_PROTOCOL_QRCODE:alsa-lib
endef

define Package/$(PKG_NAME)-demo
  $(call Package/wifimanager-v2.0/Default)
  TITLE:=Tina wifimanager-v2.0 app demo
  DEPENDS:= wifimanager-v2.0 wifimanager-v2.0-lib +wirelesscommon
endef

define Package/$(PKG_NAME)/description
	Tina wifi manager v2.0
endef

define Package/$(PKG_NAME)-lib/description
	Tina wifimanager-v2.0 lib
endef

define Package/$(PKG_NAME)-demo/description
	Tina wifimanager-v2.0 app demo
endef

define Build/Prepare
	rm $(PKG_BUILD_DIR) -rf
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/core $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

WIFIMG_HASH=$(shell git -C $(SRC_CODE_DIR) log --oneline -1 | cut -c1-7)
WIFIMG_DIRTY=$(shell [[ -z `git -C $(SRC_CODE_DIR) status -s` ]] || echo 'modified/untracked')
WIFIMG_DATE=$(shell date +"%Y%m%d-%H:%M:%S")

TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_ERROR),-DDEFAULT_DEBUG_LV_ERROR)
TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_WARN),-DDEFAULT_DEBUG_LV_WARN)
TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_INFO),-DDEFAULT_DEBUG_LV_INFO)
TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_DEBUG),-DDEFAULT_DEBUG_LV_DEBUG)
TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_DUMP),-DDEFAULT_DEBUG_LV_DUMP)
TARGET_CFLAGS += $(if $(CONFIG_WMG_DEFAULT_DEBUG_LV_EXCE),-DDEFAULT_DEBUG_LV_EXCE)

TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_STA_MODE),-DSUPPORT_STA_MODE)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_AP_MODE),-DSUPPORT_AP_MODE)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_MONITOR_MODE),-DSUPPORT_MONITOR_MODE)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_P2P_MODE),-DSUPPORT_P2P_MODE)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_STA_AP_MODE),-DSUPPORT_STA_AP_MODE)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_STA_P2P_MODE),-DSUPPORT_STA_P2P_MODE)

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lrt -ldl -lm -lresolv)
TARGET_CFLAGS += $(if $(CONFIG_WMG_SUPPORT_EXPAND),-DSUPPORT_EXPAND)
TARGET_CFLAGS += $(if $(CONFIG_WMG_PLATFORM_LINUX),-DOS_NET_LINUX_OS)
TARGET_CFLAGS += $(if $(CONFIG_WMG_PLATFORM_FREERTOS),-DOS_NET_FREERTOS_OS)
TARGET_CFLAGS += $(if $(CONFIG_WMG_PLATFORM_XRLINK),-DOS_NET_XRLINK_OS)

TARGET_CFLAGS += $(if $(CONFIG_IPV6),-DIPV6)

TARGET_CFLAGS += -DWMG_CONFIG_PATH=\\\"$(CONFIG_WMG_CONFIG_PATH)\\\"
TARGET_CFLAGS += $(if $(WIFIMG_HASH),-DWMG_HASH=\\\"\ $(WIFIMG_HASH)\\\")
TARGET_CFLAGS += $(if $(WIFIMG_DIRTY),-DWMG_DIRTY=\\\"\ $(WIFIMG_DIRTY)\\\")
TARGET_CFLAGS += $(if $(WIFIMG_DATE),-DWMG_DATE=\\\"\ $(WIFIMG_DATE)\\\")

TARGET_CFLAGS += -I $(STAGING_DIR)/usr/include/libnl-tiny
TARGET_LDFLAGS += -lnl-tiny -lwirelesscom

ifeq ($(CONFIG_WMG_PROTOCOL_QRCODE),y)
TARGET_CFLAGS += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/middleware/include/ -type d))
TARGET_CFLAGS += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/middleware/media/ -type d))
TARGET_CFLAGS += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/middleware/sample/ -type d))
TARGET_CFLAGS += $(addprefix -I, $(shell find $(STAGING_DIR)/usr/include/eyesee-mpp/system/public/include/ -type d))
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libcedarc/include/ -I$(STAGING_DIR)/usr/include/libcedarc/base/include/

MPP_LIB_DIR := $(STAGING_DIR)/usr/lib/eyesee-mpp/
CEDA_LIB_DIR := $(STAGING_DIR)/usr/lib/
MPP_LIBS := $(addprefix -l, $(patsubst lib%,%,$(patsubst %.a,%,$(notdir $(foreach dir,$(MPP_LIB_DIR),$(wildcard $(dir)/*.a))))))
CEDA_LIBS := -lcdc_base -lvencoder -lvdecoder -lMemAdapter -lvenc_common -lawmjpegplus -lVE -lvenc_base -lvenc_h264 -lvenc_jpeg -lvideoengine


MPP_LIB := \
  -Wl,-Bstatic -Wl,--whole-archive -Wl,--start-group \
  $(MPP_LIBS) \
  $(CEDA_LIBS) \
  -lawion \
  -Wl,--end-group \
  -Wl,--no-whole-archive \
  -Wl,-Bdynamic \
  -lexpat \
  -lglog \
  -lasound \
  -lz \
  -lstdc++

TARGET_LDFLAGS += $(MPP_LIB) -L$(MPP_LIB_DIR) -L$(CEDA_LIB_DIR)
endif

ifneq ($(CONFIG_WMG_PROTOCOL_BLE)$(CONFIG_WMG_PROTOCOL_JL6328)$(CONFIG_WMG_PROTOCOL_SOFTAP)$(CONFIG_WMG_PROTOCOL_XCONFIG)$(CONFIG_WMG_PROTOCOL_SOUNDWAVE)$(CONFIG_WMG_PROTOCOL_QRCODE), )
TARGET_CFLAGS += -DSUPPORT_LINKD
endif

TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_TEST_TOOL),wifi_test_tool)
TARGET_CFLAGS += $(if $(CONFIG_WMG_CMD_SOCKET_ACK),-DWMG_CMD_SOCKET_ACK)

TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_STA),wifi_sta,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_AP),wifi_ap,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_MONITOR),wifi_monitor,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_P2P),wifi_p2p,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_STA_AP_COEXIST),wifi_sta_ap_coexist,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_NET_CONFIG),wifi_net_config,$(TARGET_DEMO))
TARGET_DEMO:=$(if $(CONFIG_WMG_DEMO_FRAME),wifi_frame,$(TARGET_DEMO))
TARGET_DEMO_DIR:=$(if $(TARGET_DEMO),$(TARGET_DEMO),wifi_test_tool)

LOCAL_ARGS= \
		CONFIG_WMG_PLATFORM_LINUX=$(CONFIG_WMG_PLATFORM_LINUX) \
		CONFIG_WMG_PLATFORM_XRLINK=$(CONFIG_WMG_PLATFORM_XRLINK) \
		CONFIG_WMG_CONFIG_PATH=$(CONFIG_WMG_CONFIG_PATH) \
		CONFIG_WMG_SUPPORT_EXPAND=$(CONFIG_WMG_SUPPORT_EXPAND) \
		CONFIG_WMG_SUPPORT_STA_MODE=$(CONFIG_WMG_SUPPORT_STA_MODE) \
		CONFIG_WMG_SUPPORT_AP_MODE=$(CONFIG_WMG_SUPPORT_AP_MODE) \
		CONFIG_WMG_SUPPORT_MONITOR_MODE=$(CONFIG_WMG_SUPPORT_MONITOR_MODE) \
		CONFIG_WMG_SUPPORT_P2P_MODE=$(CONFIG_WMG_SUPPORT_P2P_MODE) \
		CONFIG_WMG_PROTOCOL_BLE=$(CONFIG_WMG_PROTOCOL_BLE) \
		CONFIG_WMG_PROTOCOL_JL6328=$(CONFIG_WMG_PROTOCOL_JL6328) \
		CONFIG_WMG_PROTOCOL_SOFTAP=$(CONFIG_WMG_PROTOCOL_SOFTAP) \
		CONFIG_WMG_PROTOCOL_XCONFIG=$(CONFIG_WMG_PROTOCOL_XCONFIG) \
		CONFIG_WMG_PROTOCOL_SOUNDWAVE=$(CONFIG_WMG_PROTOCOL_SOUNDWAVE) \
		CONFIG_WMG_PROTOCOL_QRCODE=$(CONFIG_WMG_PROTOCOL_QRCODE) \
		CONFIG_PACKAGE_kmod-net-xr819=$(CONFIG_PACKAGE_kmod-net-xr819) \
		CONFIG_PACKAGE_kmod-net-xr819s=$(CONFIG_PACKAGE_kmod-net-xr819s) \
		CONFIG_PACKAGE_kmod-net-xr829=$(CONFIG_PACKAGE_kmod-net-xr829) \
		CONFIG_PACKAGE_kmod-net-aic8800=$(CONFIG_PACKAGE_kmod-net-aic8800) \
		CONFIG_LINUX_4_9=$(CONFIG_LINUX_4_9) \
		CONFIG_LINUX_5_4=$(CONFIG_LINUX_5_4) \
		CONFIG_LINUX_5_10=$(CONFIG_LINUX_5_10) \
		CONFIG_LINUX_5_15=$(CONFIG_LINUX_5_15) \
		CONFIG_LINUX_6_6=$(CONFIG_LINUX_6_6) \
		CONFIG_ARCH=$(CONFIG_ARCH) \
		CONFIG_WMG_DEMO_TEST_TOOL=$(CONFIG_WMG_DEMO_TEST_TOOL) \
		CONFIG_WMG_CMD_SOCKET_ACK=$(CONFIG_WMG_CMD_SOCKET_ACK) \
		CONFIG_WMG_DEMO_STA=$(CONFIG_WMG_DEMO_STA) \
		CONFIG_WMG_DEMO_AP=$(CONFIG_WMG_DEMO_AP) \
		CONFIG_WMG_DEMO_MONITOR=$(CONFIG_WMG_DEMO_MONITOR) \
		CONFIG_WMG_DEMO_P2P=$(CONFIG_WMG_DEMO_P2P) \
		CONFIG_WMG_DEMO_STA_AP_COEXIST=$(CONFIG_WMG_DEMO_STA_AP_COEXIST) \
		CONFIG_WMG_DEMO_NET_CONFIG=$(CONFIG_WMG_DEMO_NET_CONFIG) \
		CONFIG_WMG_DEMO_FRAME=$(CONFIG_WMG_DEMO_FRAME) \
		CONFIG_LIBC="${CONFIG_LIBC}"

ifeq ($(CONFIG_PACKAGE_wifimanager-v2.0-lib),y)
define Build/Compile/$(PKG_NAME)-lib
	#libwifimg-v2.0.so
	$(MAKE) -C $(PKG_BUILD_DIR)/core/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		$(LOCAL_ARGS) \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_wifimanager-v2.0-demo),y)
define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/$(TARGET_DEMO_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		$(LOCAL_ARGS) \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-lib)
	$(Build/Compile/$(PKG_NAME)-demo)
endef

ifeq ($(CONFIG_PACKAGE_wifimanager-v2.0-lib),y)
define Build/InstallDev/$(PKG_NAME)-lib
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg-v2.0.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef
endif

ifeq ($(CONFIG_PACKAGE_wifimanager-v2.0-demo),y)
define Build/InstallDev/$(PKG_NAME)-demo
endef
endif

define Build/InstallDev
	$(Build/InstallDev/$(PKG_NAME))
	$(Build/InstallDev/$(PKG_NAME)-lib)
	$(Build/InstallDev/$(PKG_NAME)-demo)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)$(CONFIG_WMG_CONFIG_PATH)
	$(INSTALL_DIR) $(1)/etc/init.d
ifeq ($(CONFIG_WMG_PLATFORM_LINUX),y)
	$(INSTALL_DIR) $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_supplicant
	$(INSTALL_DIR) $(1)$(CONFIG_WMG_CONFIG_PATH)/hostapd
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/hostapd.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/hostapd
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/wpa_supplicant.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_supplicant
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/wpa_supplicant_p2p.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_supplicant
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/wpa_supplicant_src.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_supplicant
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/wpa_supplicant_overlay.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_supplicant
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/wifi_monitor.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/monitor
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/udhcpc_wlan0.init $(1)$(CONFIG_WMG_CONFIG_PATH)/udhcpc_wlan0
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wifi_start.init $(1)/etc/init.d/wifi_start
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/udhcpc_p2p-wlan0-0.init $(1)$(CONFIG_WMG_CONFIG_PATH)/udhcpc_p2p-wlan0-0
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wmg_udhcpc.script $(1)$(CONFIG_WMG_CONFIG_PATH)/wmg_udhcpc.script
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wmg_udhcpc6.script $(1)$(CONFIG_WMG_CONFIG_PATH)/wmg_udhcpc6.script
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/dnsmasq.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/dnsmasq.conf
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/basic_network_check.sh $(1)$(CONFIG_WMG_CONFIG_PATH)/basic_network_check.sh
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/gatewayip_check.sh $(1)$(CONFIG_WMG_CONFIG_PATH)/gatewayip_check.sh
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wpa_enable.sh $(1)$(CONFIG_WMG_CONFIG_PATH)/wpa_enable.sh
endif
ifeq ($(CONFIG_WMG_PROTOCOL_QRCODE),y)
	$(INSTALL_DATA) $(SRC_CODE_DIR)/files/qrcode.conf $(1)$(CONFIG_WMG_CONFIG_PATH)/
endif
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wifimg.config $(1)$(CONFIG_WMG_CONFIG_PATH)/wifimg.def
ifeq ($(CONFIG_WMG_LOCK_MAC_ADDR),y)
	$(INSTALL_BIN) $(SRC_CODE_DIR)/files/wifi_lock_mac $(1)/etc/init.d/wifi_lock_mac
endif
endef

define Package/$(PKG_NAME)-lib/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg-v2.0.so $(1)/lib
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
ifeq ($(TARGET_DEMO_DIR),wifi_test_tool)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_daemon $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_sta)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_sta $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_ap)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_ap $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_monitor)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_monitor $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_p2p)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_p2p $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_sta_ap_coexist)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_sta_ap_coexist $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_net_config)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_net_config $(1)/bin
endif
ifeq ($(TARGET_DEMO_DIR),wifi_frame)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_frame $(1)/bin
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
