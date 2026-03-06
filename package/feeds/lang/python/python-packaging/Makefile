#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=python-packaging
PKG_VERSION:=21.3
PKG_RELEASE:=1

PYPI_NAME:=packaging
PKG_HASH:=dd47c42927d89ab911e606518907cc2d3a1f38bbd026385970643f9c5b8ecfeb

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=Apache-2.0 BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE.APACHE LICENSE.BSD

HOST_BUILD_DEPENDS:= python-tomli/host
PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host python-tomli

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-packaging
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Core utilities for Python packages
  URL:=https://github.com/pypa/packaging
  DEPENDS:=+python3-light +python3-pyparsing +python3-six +python3-logging +python3-distutils +python3-tomli
endef

define Package/python3-packaging/description
  The packaging project includes version handling, specifiers,
  markers, requirements, tags, utilities.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-packaging))
$(eval $(call BuildPackage,python3-packaging))
$(eval $(call BuildPackage,python3-packaging-src))
