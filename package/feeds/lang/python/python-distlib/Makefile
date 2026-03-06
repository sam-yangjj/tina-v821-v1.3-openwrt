#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-distlib
PKG_VERSION:=0.3.4
PKG_RELEASE:=1

PYPI_NAME:=distlib
PKG_HASH:=e4b58818180336dc9c529bfb9a0b58728ffc09ad92027a3f30b7cd91e3458579

PKG_SOURCE:=$(PYPI_NAME)-$(PKG_VERSION).zip

PKG_MAINTAINER:=vinay_sajip@red-dove.com
PKG_LICENSE:=Python Software Foundation
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3
HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-distlib
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Distribution utilities
  URL:=https://pypi.org/project/distlib
  DEPENDS:= \
	+python3
endef

define Package/python3-distlib/description
  Distribution utilities.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-distlib))
$(eval $(call BuildPackage,python3-distlib))
