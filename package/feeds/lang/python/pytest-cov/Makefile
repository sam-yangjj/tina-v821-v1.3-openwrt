#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pytest-cov
PKG_VERSION:=4.0.0
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=996b79efde6433cdbd0088872dbc5fb3ed7fe1578b68cdbba634f14bb8dd0470

PKG_MAINTAINER:=marc.schlaich@gmail.com
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3
HOST_BUILD_DEPENDS:=python-coverage/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest-cov
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Pytest plugin for measuring coverage
  URL:=https://pypi.org/project/pytest-cov
  DEPENDS:= \
	+python3 \
	+python3-pytest \
	+python3-coverage
endef

define Package/python3-pytest-cov/description
  Pytest plugin for measuring coverage.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-pytest-cov))
$(eval $(call BuildPackage,python3-pytest-cov))
