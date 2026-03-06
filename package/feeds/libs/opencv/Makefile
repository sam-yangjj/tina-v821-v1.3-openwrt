#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opencv
PKG_VERSION:=5.0.0-alpha
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_EXTRA_NAME:=opencv_contrib
PKG_EXTRA_SOURCE:=$(PKG_EXTRA_NAME)-$(PKG_VERSION).zip
PKG_EXTRA_SOURCE_SUBDIR:=$(PKG_EXTRA_NAME)-$(PKG_VERSION)

ADE_FRAMEWORK:=ade
ADE_FRAMEWORK_VERSION:=0.1.2e
ADE_FRAMEWORK_SOURCE:=$(ADE_FRAMEWORK)-$(ADE_FRAMEWORK_VERSION).zip
ADE_FRAMEWORK_SOURCE_SUBDIR:=$(ADE_FRAMEWORK)-$(ADE_FRAMEWORK_VERSION)

PKG_LICENSE:=Apache-2.0 license
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
CMAKE_BINARY_SUBDIR:=openwrt-build
PKG_JOBS:=-j

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Open Source Computer Vision Library
  URL:=https://opencv.org/
  ABI_VERSION:=2
  DEPENDS:= +zlib +libjpeg-turbo +libpng +libstdcpp +OpenBLAS PACKAGE_libgpu:libgpu
endef

define Package/$(PKG_NAME)/description
  OpenCV: Open Source Computer Vision Library
endef

ifeq ($(CONFIG_PACKAGE_kmod-mali-g57-km),y)
OPENCL_LIB:=$(STAGING_DIR)/usr/lib/libmali.so
endif
ifeq ($(CONFIG_PACKAGE_kmod-ge8300-km),y)
OPENCL_LIB:=$(STAGING_DIR)/usr/lib/libPVROCL.so
endif

define Package/$(PKG_NAME)/config
	if PACKAGE_$(PKG_NAME)
		config ENABLE_BUILD_EXAMPLES
			bool "Enabel build examples"
			default n
	endif
endef

ifeq ($(CONFIG_ENABLE_BUILD_EXAMPLES),y)
	ENABLE_BUILD_EXAMPLES_VALUE:=ON
else
	ENABLE_BUILD_EXAMPLES_VALUE:=OFF
endif

CMAKE_OPTIONS += \
	-DENABLE_SHARED=ON \
	-DENABLE_STATIC=ON \
	-DWITH_GTK=OFF \
	-DWITH_TIFF=OFF \
	-DWITH_QT=OFF \
	-DWITH_GSTREAMER=OFF \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_CONFIGURATION_TYPES=Release \
	-DOPENCV_INCLUDE_INSTALL_PATH=/usr/include \
	-DBUILD_PERF_TESTS=OFF \
	-DBUILD_TESTS=OFF \
	-DBUILD_EXAMPLES=$(ENABLE_BUILD_EXAMPLES_VALUE) \
	-DINSTALL_BIN_EXAMPLES=$(ENABLE_BUILD_EXAMPLES_VALUE) \
	-DOpenCL_LIBRARY=$(OPENCL_LIB) \
	-DOPENCV_PYTHON_SKIP_DETECTION=ON \
	-DOPENCV_UNIFONT_URL=$(DL_DIR) \
	-DOPENCV_EXTRA_MODULES_PATH=$(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/$(PKG_EXTRA_SOURCE_SUBDIR)/modules

define ade_unpack
	mkdir -p $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/3rdparty/ade
	unzip -d $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/3rdparty/ade $(DL_DIR)/$(ADE_FRAMEWORK_SOURCE);
endef

define opencv_contrib_unpack
	mkdir -p $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)
	unzip -d $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) $(DL_DIR)/$(PKG_EXTRA_SOURCE);
endef

define copy_files
	mkdir -p $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/downloads/wechat_qrcode
	cp ./wechat_qrcode/* $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/downloads/wechat_qrcode
	mkdir -p $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/downloads/xfeatures2d
	cp ./xfeatures2d/* $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/downloads/xfeatures2d
	mkdir -p $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/share/opencv4/testdata/cv/face/
	cp ./face/* $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/share/opencv4/testdata/cv/face/
	$(call PatchDir,$(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/$(PKG_EXTRA_SOURCE_SUBDIR),$(PATCH_DIR)/../opencv_contrib_patches,);
endef

Hooks/Prepare/Post += ade_unpack
Hooks/Prepare/Post += opencv_contrib_unpack
Hooks/Prepare/Post += copy_files

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
