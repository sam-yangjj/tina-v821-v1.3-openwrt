#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pluggy
PKG_VERSION:=1.0.0
PKG_RELEASE:=4

PYPI_NAME:=pluggy
PKG_HASH:=4224373bacce55f955a878bf9cfa763c1e360858e330072059e10bad68531159

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=setuptools-scm python-pyparsing/host
HOST_BUILD_DEPENDS:=python3/host setuptools-scm/host
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pluggy
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Plugin and hook calling mechanisms for Python
  URL:=https://github.com/pytest-dev/pluggy
  DEPENDS:=+python3-light \
	+python3-tomli \
	+python3-pyparsing
endef

define Package/python3-pluggy/description
  A minimalist production ready plugin system for python
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-pluggy))
$(eval $(call BuildPackage,python3-pluggy))
$(eval $(call BuildPackage,python3-pluggy-src))
