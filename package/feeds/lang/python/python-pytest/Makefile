#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pytest
PKG_VERSION:=7.2.0
PKG_RELEASE:=1

PYPI_NAME:=pytest
PKG_HASH:=c4014eb40e10f11f355ad4e3c2fb2c6c6d1919c73f3b5a433de4708202cade59

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host
HOST_BUILD_DEPENDS:=python-py/host python-exceptiongroup/host python-pluggy/host python-iniconfig/host python-attrs/host
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python testing framework
  URL:=https://docs.pytest.org/en/latest/
  DEPENDS:= \
	+python3 \
	+python3-more-itertools \
	+python3-py \
	+python3-attrs \
	+python3-pluggy \
	+python3-packaging \
	+python3-toml \
	+python3-iniconfig \
	+python3-py \
	+python3-exceptiongroup
endef

define Package/python3-pytest/description
  The pytest framework makes it easy to write small tests, yet scales to support
  complex functional testing for applications and libraries.
endef

define Host/Compile
	$(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-pytest))
$(eval $(call BuildPackage,python3-pytest))
$(eval $(call BuildPackage,python3-pytest-src))
