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
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/gc1084_mipi.ko
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/vin_v4l2.ko
	AUTOLOAD:=$(call AutoProbe,videobuf2-common videobuf2-memops videobuf2-dma-contig videobuf2-v4l2 vin_io gc1084_mipi vin_v4l2)
endef

define KernelPackage/vin-v4l2/description
	Kernel modules for video input support
endef

$(eval $(call KernelPackage,vin-v4l2))
