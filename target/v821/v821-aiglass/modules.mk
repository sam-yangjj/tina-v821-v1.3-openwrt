#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.
#

define KernelPackage/net-aic8800
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=aic8800 support (staging)
  DEPENDS:= +aic8800-firmware +@IPV6 +@PACKAGE_aic8800-rftest
  KCONFIG:=\
  CONFIG_AIC_WLAN_SUPPORT=y\
  CONFIG_AW_RFKILL=y\
  CONFIG_AIC8800_WLAN_SUPPORT=m\
  CONFIG_AIC8800_BTLPM_SUPPORT=m\
  CONFIG_AW_MACADDR_MGT=y\
  CONFIG_BT=y\
  CONFIG_BT_RFCOMM=y\
  CONFIG_BT_RFCOMM_TTY=y\
  CONFIG_BT_HCIUART=y\
  CONFIG_BT_HCIUART_H4=y\
  CONFIG_RFKILL=y\
  CONFIG_RFKILL_GPIO=y

  FILES:=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/aic8800/aic8800_bsp/aic8800_bsp.ko
  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/aic8800/aic8800_btlpm/aic8800_btlpm.ko
  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/aic8800/aic8800_fdrv/aic8800_fdrv.ko
  AUTOLOAD:=$(call AutoProbe, aic8800_bsp aic8800_btlpm aic8800_fdrv)
endef

define KernelPackage/net-aic8800/description
 Kernel modules for aic8800 support
endef

$(eval $(call KernelPackage,net-aic8800))

define KernelPackage/net-xr819s
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=xr819s support (staging)
  DEPENDS:= +xr819s-firmware +@IPV6 +@USES_XR819S +@PACKAGE_xr819s-rftest
  KCONFIG:=\
	CONFIG_XR819_WLAN=m \
	CONFIG_PM=y\
	CONFIG_BT=n \
	CONFIG_BT_BREDR=n \
	CONFIG_BT_RFCOMM=n \
	CONFIG_BT_RFCOMM_TTY=n \
	CONFIG_BT_DEBUGFS=n \
	CONFIG_XR_BT_LPM=n \
	CONFIG_XR_BT_FDI=n \
	CONFIG_BT_HCIUART=n \
	CONFIG_BT_HCIUART_H4=n \
	CONFIG_HFP_OVER_PCM=n \
	CONFIG_RFKILL=n \
	CONFIG_RFKILL_PM=n \
	CONFIG_RFKILL_GPIO=n

  #FILES:=$(LINUX_DIR)/drivers/net/wireless/xr819s/wlan/xradio_core.ko
  #FILES+=$(LINUX_DIR)/drivers/net/wireless/xr819s/wlan/xradio_wlan.ko
  #FILES+=$(LINUX_DIR)/drivers/net/wireless/xr819s/umac/xradio_mac.ko
  #AUTOLOAD:=$(call AutoProbe, xradio_mac xradio_core xradio_wlan)

ifeq ($(CONFIG_XR819S_USE_SIP_SDD), y)
  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/xr819s/xr819s.ko
  AUTOLOAD:=$(call AutoProbe, xr819)
else
  FILES:=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/xr819s/wireless_switch/wireless_switch.ko
  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/xr819s/xr819s.ko
  AUTOLOAD:=$(call AutoProbe, wireless_switch, xr819s)
endif
endef

define KernelPackage/net-xr819s/description
 Kernel modules for xr819s support
endef

$(eval $(call KernelPackage,net-xr819s))

define KernelPackage/net-v821_fmac
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=v821_fmac support (staging)
  DEPENDS:= +@IPV6
  KCONFIG:=\
	CONFIG_V821_FMAC_WLAN=m \

  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/v821_fmac/v821_fmac.ko
  AUTOLOAD:=$(call AutoProbe, v821_fmac.ko)
endef

define KernelPackage/net-v821_fmac/description
 Kernel modules for v821_fmac support
endef

$(eval $(call KernelPackage,net-v821_fmac))

define KernelPackage/net-v821_smac
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=v821_smac support (staging)
  DEPENDS:= +@IPV6
  KCONFIG:=\
	CONFIG_V821_sMAC_WLAN=m \

  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/net/wireless/v821_smac/v821_smac.ko
  AUTOLOAD:=$(call AutoProbe, v821_smac.ko)
endef

define KernelPackage/net-v821_smac/description
 Kernel modules for v821_smac support
endef

$(eval $(call KernelPackage,net-v821_smac))

define KernelPackage/aglink
  SUBMENU:=$(OTHER_MENU)
  TITLE:=aglink support (staging)
  KCONFIG:=\
    CONFIG_AG_LINK=m \

  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/misc/aglink/aglink.ko
  AUTOLOAD:=$(call AutoProbe, aglink.ko)
endef

define KernelPackage/aglink/description
 Kernel modules fo aglink support
endef

$(eval $(call KernelPackage,aglink))
