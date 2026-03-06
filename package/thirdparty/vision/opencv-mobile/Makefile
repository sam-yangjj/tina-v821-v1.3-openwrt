#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=opencv-mobile
PKG_VERSION:=4.12.0

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE_URL:=./src/
PKG_SOURCE_PROTO:=linkfile
PKG_LICENSE:=Apache-2.0 license

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

CMAKE_SOURCE_DIR := $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR)
CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR)/usr/
CMAKE_OPTIONS += -DCMAKE_CXX_FLAGS="$(TARGET_CXXFLAGS) -I $(LINUX_DIR)/user_headers/include -I $(LINUX_DIR)/user_headers/include/bsp"
CMAKE_OPTIONS += -DCMAKE_C_FLAGS="$(TARGET_CFLAGS) -I $(LINUX_DIR)/user_headers/include -I $(LINUX_DIR)/user_headers/include/bsp"

CMAKE_OPTIONS += $(foreach option,$(shell cat $(SOURCE)/opencv4_cmake_options.txt 2>/dev/null),$(option))

ifeq ($(CONFIG_OPENCV_AWISPAPI),y)
CMAKE_OPTIONS += -DENABLE_SUNXIVINISP=ON
endif

ifeq ($(CONFIG_OPENCV_OPENMP),y)
CMAKE_OPTIONS += -DWITH_OPENMP=ON
else
CMAKE_OPTIONS += -DWITH_OPENMP=OFF
endif

define Package/$(PKG_NAME)
	SECTION:=OpenCV
	CATEGORY:=Vision
	DEPENDS:=+libstdcpp +libatomic +OPENCV_AWISPAPI:libAWIspApi
	TITLE:=OpenCV-Mobile v4.12.0
	URL:=https://github.com/nihui/opencv-mobile
endef

define Package/$(PKG_NAME)/description
	OpenCV (Open Source Computer Vision Library) is an open source computer
	vision and machine learning software library. OpenCV was built to provide
	a common infrastructure for computer vision applications and to accelerate
	the use of machine perception in the commercial products. Being a
	BSD-licensed product, OpenCV makes it easy for businesses to utilize
	and modify the code.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/opencv_build/
	mkdir -p $(PKG_BUILD_DIR)/opencv_build/patches
	unzip -d $(PKG_BUILD_DIR)/opencv_build/ $(DL_DIR)/$(PKG_SOURCE)

	cp ./patches/* $(PKG_BUILD_DIR)/opencv_build/patches
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0001-feat-videoio-add-support-for-Sunxi-VIN-ISP.patch
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0002-feat-core-add-isUMatVector-method-to-_InputArray.patch
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0003-refactor-videoio-remove-UMat-operator-overloads-for-.patch
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0004-refactor-videoio-replace-RTTI-with-template-based-ty.patch
endef

define Build/Compile
	$(MAKE) -j$(nproc) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		FC=$(TARGET_FORTRAN) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Build/Install
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CXXFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	LIBS="$(TARGET_LIBS)" \
	$(MAKE) -j$(nproc) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		ARCH="$(ARCH)" \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
