#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pytest-rerunfailures
PKG_VERSION:=10.2
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=9e1e1bad51e07642c5bbab809fc1d4ec8eebcb7de86f90f1a26e6ef9de446697

PKG_MAINTAINER:=lklrmn@gmail.com
PKG_LICENSE:=Mozilla Public License 2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host
HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest-rerunfailures
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=pytest plugin to re-run tests to eliminate flaky failures
  URL:=https://pypi.org/project/pytest-rerunfailures
  DEPENDS:= \
	+python3
endef

define Package/python3-pytest-rerunfailures/description
  pytest plugin to re-run tests to eliminate flaky failures
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-pytest-rerunfailures))
$(eval $(call BuildPackage,python3-pytest-rerunfailures))
