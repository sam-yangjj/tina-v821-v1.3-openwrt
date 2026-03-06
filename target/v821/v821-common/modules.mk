#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.
#

define KernelPackage/sunxi-g2d
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=sunxi-g2d support
  KCONFIG:=\
	  CONFIG_AW_G2D=m \
	  CONFIG_G2D_RCQ=y \
	  CONFIG_G2D_LEGACY=n \
	  CONFIG_G2D_MIXER=y \
	  CONFIG_G2D_ROTATE=y \
	  CONFIG_G2D_SYNCFENCE=n
  FILES+=$(LINUX_DIR)/bsp/drivers/g2d/g2d_sunxi.ko
  AUTOLOAD:=$(call AutoLoad,20,g2d_sunxi,1)
endef

define KernelPackage/sunxi-g2d/description
  Kernel modules for sunxi-g2d support
endef

$(eval $(call KernelPackage,sunxi-g2d))

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

#cp ve ko to rootfs
#if kernel CONFIG_AW_VIDEO_SUNXI_VIN=y or m, sunxi-ve.ko depend vin ko
#if kernel #CONFIG_AW_VIDEO_SUNXI_VIN is not set, sunxi-ve.ko dont depend vin
define KernelPackage/sunxi-ve
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Video encoder and decoder support (staging)
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/ve/cedar-ve/sunxi-ve.ko
	AUTOLOAD:=$(call AutoProbe,sunxi-ve)
endef

define KernelPackage/sunxi-ve/extra_provides
  echo 'vin_v4l2.ko'
endef

define KernelPackage/sunxi-ve/description
	Kernel modules for video encoder and decoder support
endef

$(eval $(call KernelPackage,sunxi-ve))

define KernelPackage/xradio-bt-smd
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:= v821 xradio bt smd support
  KCONFIG:=\
	CONFIG_BT_XR_SMD=m \

  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/bluetooth/xradio_bt_smd/xradio_bt_smd.ko
  AUTOLOAD:=$(call AutoProbe, xradio_bt_smd.ko)
endef

define KernelPackage/xradio-bt-smd/description
 Kernel modules for v821 xradio bt smd support
endef

$(eval $(call KernelPackage,xradio-bt-smd))
