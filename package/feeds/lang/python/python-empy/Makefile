#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-empy
PKG_VERSION:=3.3.4
PKG_RELEASE:=1

PYPI_NAME:=empy
PKG_HASH:=73ac49785b601479df4ea18a7c79bc1304a8a7c34c02b9472cf1206ae88f01b3

PKG_MAINTAINER:=software@alcyone.com
PKG_LICENSE:=
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3
HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-empy
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A templating system for Python.
  URL:=https://pypi.org/project/empy
  DEPENDS:= \
	+python3
endef

define Package/python3-empy/description
  A templating system for Python.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-empy))
$(eval $(call BuildPackage,python3-empy))
