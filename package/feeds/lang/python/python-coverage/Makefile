#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-coverage
PKG_VERSION:=6.5.0
PKG_RELEASE:=1

PYPI_NAME:=coverage
PKG_HASH:=f642e90754ee3e06b0e7e51bce3379590e76b7f76b708e1a71ff043f87025c84

PKG_MAINTAINER:=ned@nedbatchelder.com
PKG_LICENSE:=Apache Software License (Apache 2.0)
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3
HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-coverage
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Code coverage measurement for Python
  URL:=https://pypi.org/project/coverage
  DEPENDS:= \
	+python3
endef

define Package/python3-coverage/description
  Code coverage measurement for Python
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-coverage))
$(eval $(call BuildPackage,python3-coverage))
