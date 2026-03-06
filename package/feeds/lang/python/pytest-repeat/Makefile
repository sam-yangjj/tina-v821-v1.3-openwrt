#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pytest-repeat
PKG_VERSION:=0.8.0
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=52bc413ab1a772e72c953a5592196ddd266fd2e76280eaac0c2a8378e9ac6dd9

PKG_MAINTAINER:=bsilverberg@mozilla.com
PKG_LICENSE:=Mozilla Public License 2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host setuptools-scm
HOST_BUILD_DEPENDS:=python3/host setuptools-scm/host
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest-repeat
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=pytest plugin to re-run tests to eliminate flaky failures
  URL:=https://pypi.org/project/pytest-repeat
  DEPENDS:= \
	+python3 \
	+python3-pytest \
	+python3-setuptools-scm
endef

define Package/python3-pytest-repeat/description
  pytest-repeat is a plugin for pytest that makes it easy to repeat a single test, or multiple tests, a specific number of times.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-pytest-repeat))
$(eval $(call BuildPackage,python3-pytest-repeat))
