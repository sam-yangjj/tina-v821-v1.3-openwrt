#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=riscv32
BOARD:=v821-avaota_f1
BOARDNAME:=v821-avaota_f1
FEATURES:=audio
CPU_TYPE:=a27l2
CPU_SUBTYPE:=rv32imac_zicsr_zifencei
MAINTAINER:=Allwinner
CFLAGS:=-mabi=ilp32d -march=rv32imafdcxandes -mcmodel=medlow

KERNEL_PATCHVER:=5.4
UBOOT_PATCHVER:=2018
KERNELNAME:=Image.gz dtbs

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES = base-files libc libgcc busybox
TARGET_CFLAGS += -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -funwind-tables -Wno-unused-but-set-variable -Wno-unused-variable \
    -Wno-unused-function -Wno-unused-label -Wno-unused-const-variable -Wno-comment -Wno-unused-value
#-DUSE_STD_LOG
TARGET_CXXFLAGS += -std=c++11 -fno-rtti
TARGET_LDFLAGS += -Wl,-rpath=/usr/lib

$(eval $(call BuildTarget))
