#
# Copyright (C) 2007-2015 OpenWrt.org
#
# It is licensed under the GPLv3 (see COPYING).
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glmark2
PKG_VERSION:=7215c0f337dae0b232535549c37fca441747a891
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://launchpad.net/glmark2/
PKG_MD5SUM:=d70a069a31848f87611ec9b7566d3e7b

PKG_LICENSE:=GPLv3+ SGIv1
PKG_LICENSE_FILES:=COPYING COPYING.SGI

#PKG_BUILD_DEPENDS:=python/host

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

define Package/glmark2
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=glmark2 is an OpenGL 2.0 and ES 2.0 benchmark
  URL:=https://launchpad.net/glmark2/
  MAINTAINER:=
  MENU:=1
  DEPENDS:= +libstdcpp +libpng +libjpeg @libgpu +PACKAGE_libgpu:libgpu
endef

define Package/glmark2/description
  glmark2 is developed by Alexandros Frantzis and Jesse Barker based on the
original glmark benchmark by Ben Smith.
endef

GLMARK2_FLAVORS_FBDEV += fbdev-glesv2
GLMARK2_FLAVORS_WAYLAND += wayland-glesv2 drm-glesv2
LDFLAGS+=-lpthread

ifeq ($(CONFIG_GLMARK2_FBDEV), y)
CONFIGURE_ARGS = \
	--with-flavors=$(subst $(space),$(comma),$(GLMARK2_FLAVORS_FBDEV)) \
	--for-fbdev \
	--prefix=$(CONFIGURE_PREFIX)
endif
ifeq ($(CONFIG_GLMARK2_WAYLAND), y)
CONFIGURE_ARGS = \
	--with-flavors=$(subst $(space),$(comma),$(GLMARK2_FLAVORS_WAYLAND)) \
	--prefix=$(CONFIGURE_PREFIX)
endif

ifeq ($(CONFIG_PACKAGE_kmod-mali-g57-km),y)
LDFLAGS+=-lmali
endif
ifeq ($(CONFIG_PACKAGE_kmod-ge8300-km),y)
CONFIGURE_ARGS += \
	--img_gpu
endif

define Build/Configure
	(cd $(PKG_BUILD_DIR); $(MAKE_FLAGS) python2 ./waf configure $(CONFIGURE_ARGS) XXFLAGS=-std=c++11)
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR); python2 ./waf)
endef

define Build/Install
	(cd $(PKG_BUILD_DIR); python2 ./waf install --destdir=$(PKG_INSTALL_DIR))
endef

define Package/glmark2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/glmark2
	$(CP) $(PKG_INSTALL_DIR)/usr/share/glmark2/* $(1)/usr/share/glmark2
endef

$(eval $(call BuildPackage,glmark2))
