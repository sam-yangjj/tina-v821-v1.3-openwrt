#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.
#cp vin ko to rootfs
define KernelPackage/vin-v4l2
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Video input support (staging)
	DEPENDS:=
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/drivers/media/common/videobuf2/videobuf2-common.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/drivers/media/common/videobuf2/videobuf2-memops.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/drivers/media/common/videobuf2/videobuf2-dma-contig.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/drivers/media/common/videobuf2/videobuf2-v4l2.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/vin_io.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/vin_v4l2.ko
	AUTOLOAD:=$(call AutoProbe,videobuf2-common videobuf2-memops videobuf2-dma-contig videobuf2-v4l2 vin_io vin_v4l2)
endef

define KernelPackage/vin-v4l2/description
	Kernel modules for video input support
endef

$(eval $(call KernelPackage,vin-v4l2))

#cp sensor ko to rootfs
define KernelPackage/sensor-sp1405
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor sp1405 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/sp1405_mipi.ko
	AUTOLOAD:=$(call AutoProbe,sp1405_mipi)
endef

define KernelPackage/sensor-sp1405/description
	Kernel modules for sensor-sp1405 support
endef

$(eval $(call KernelPackage,sensor-sp1405))

#cp sensor ko to rootfs
define KernelPackage/sensor-gc4663
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor gc4663 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/gc4663_mipi.ko
	AUTOLOAD:=$(call AutoProbe,gc4663_mipi)
endef

define KernelPackage/sensor-gc4663/description
	Kernel modules for sensor-gc4663 support
endef

$(eval $(call KernelPackage,sensor-gc4663))

#cp sensor ko to rootfs
define KernelPackage/sensor-nvp6158c
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor nvp6158c support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/nvp6158/nvp6158.ko
	AUTOLOAD:=$(call nvp6158c)
endef

define KernelPackage/sensor-nvp6158c/description
	Kernel modules for sensor-nvp6158c support
endef

$(eval $(call KernelPackage,sensor-nvp6158c))

#cp sensor ko to rootfs
define KernelPackage/sensor-os02g10
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor os02g10 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/os02g10_dvp.ko
	AUTOLOAD:=$(call AutoProbe,os02g10_dvp)
endef

define KernelPackage/sensor-os02g10/description
	Kernel modules for sensor-os02g10 support
endef

$(eval $(call KernelPackage,sensor-os02g10))

#cp sensor ko to rootfs
define KernelPackage/sensor-gc1084
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor gc1084 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/gc1084_mipi.ko
	AUTOLOAD:=$(call AutoProbe,gc1084_mipi)
endef

define KernelPackage/sensor-gc1084/description
	Kernel modules for sensor-gc1084 support
endef

$(eval $(call KernelPackage,sensor-gc1084))

#cp sensor ko to rootfs
define KernelPackage/sensor-gc2083
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor gc2083 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/gc2083_mipi.ko
	AUTOLOAD:=$(call gc2083_mipi)
endef

define KernelPackage/sensor-gc2083/description
	Kernel modules for sensor-gc2083 support
endef

$(eval $(call KernelPackage,sensor-gc2083))

#cp sensor ko to rootfs
define KernelPackage/sensor-sc2336
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor sc2336 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/sc2336_mipi.ko
	AUTOLOAD:=$(call sc2336_mipi)
endef

define KernelPackage/sensor-sc2336/description
	Kernel modules for sensor-sc2336 support
endef

$(eval $(call KernelPackage,sensor-sc2336))

#cp sensor ko to rootfs
define KernelPackage/sensor-sc3336
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor sc3336 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/sc3336_mipi.ko
	AUTOLOAD:=$(call sc3336_mipi)
endef

define KernelPackage/sensor-sc3336/description
	Kernel modules for sensor-sc3336 support
endef

$(eval $(call KernelPackage,sensor-sc3336))

#cp sensor ko to rootfs
define KernelPackage/sensor-imx258
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor imx258 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/imx258_mipi.ko
	AUTOLOAD:=$(call imx258_mipi)
endef

define KernelPackage/sensor-imx258/description
	Kernel modules for sensor-imx258 support
endef

$(eval $(call KernelPackage,sensor-imx258))

#cp sensor ko to rootfs
define KernelPackage/sensor-imx386
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor imx386 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/imx386_2lane_mipi.ko
	AUTOLOAD:=$(call imx386_mipi)
endef

define KernelPackage/sensor-imx386/description
	Kernel modules for sensor-imx386 support
endef
$(eval $(call KernelPackage,sensor-imx386))

#cp sensor ko to rootfs
define KernelPackage/sensor-f37p
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor f37p support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/f37p_mipi.ko
	AUTOLOAD:=$(call f37p_mipi)
endef

define KernelPackage/sensor-f37p/description
	Kernel modules for sensor-f37p support
endef

$(eval $(call KernelPackage,sensor-f37p))

#cp sensor ko to rootfs
define KernelPackage/sensor-s5k3p8
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor s5k3p8 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/s5k3p8_mipi.ko
	AUTOLOAD:=$(call s5k3p8_mipi)
endef

define KernelPackage/sensor-s5k3p8/description
	Kernel modules for sensor-s5k3p8 support
endef

$(eval $(call KernelPackage,sensor-s5k3p8))

#cp Actuator ko to rootfs
define KernelPackage/actuator-dw9714
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Actuator dw9714 support (staging)
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/actuator/actuator.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/actuator/dw9714_act.ko
	AUTOLOAD:=$(call actuator.ko dw9714_act.ko)
endef

define KernelPackage/actuator-dw9714/description
	Kernel modules for actuator-dw9714 support
endef

$(eval $(call KernelPackage,actuator-dw9714))

define KernelPackage/sunxi-uvc
  SUBMENU:=$(USB_MENU)
  TITLE:=sunxi uvc support (staging)
  DEPENDS:=+kmod-vin-v4l2
  KCONFIG:=\
	CONFIG_USB_SUNXI_GADGET=y \
	CONFIG_USB_SUNXI_CONFIGFS_F_UVC=m \

  FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/drivers/media/common/videobuf2/videobuf2-vmalloc.ko
  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/usb/gadget/function/usb_sunxi_f_uvc.ko
  AUTOLOAD:=$(call AutoProbe, usb_sunxi_f_uvc.ko)
endef

define KernelPackage/sunxi-uvc/description
 Kernel modules for v821 sunxi-uvc support
endef

$(eval $(call KernelPackage,sunxi-uvc))

define KernelPackage/sunxi-uac1
  SUBMENU:=$(USB_MENU)
  TITLE:=sunxi uac1 support (staging)
  DEPENDS:=+kmod-vin-v4l2
  KCONFIG:=\
	CONFIG_USB_SUNXI_GADGET=y \
	CONFIG_USB_SUNXI_CONFIGFS_F_UAC1=m \

  FILES+=$(LICHEE_OUT_DIR)/$(LICHEE_IC)/kernel/build/bsp/drivers/usb/gadget/function/usb_sunxi_f_uac1.ko
  AUTOLOAD:=$(call AutoProbe, usb_sunxi_f_uac1.ko)
endef

define KernelPackage/sunxi-uac1/description
 Kernel modules for v821 sunxi-uac1 support
endef

$(eval $(call KernelPackage,sunxi-uac1))
