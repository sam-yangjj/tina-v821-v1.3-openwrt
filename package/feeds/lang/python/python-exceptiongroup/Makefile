#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-exceptiongroup
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PYPI_NAME:=exceptiongroup
PKG_HASH:=73866f7f842ede6cb1daa42c4af078e2035e5f7607f0e2c762cc51bb31bbe7b2

PKG_MAINTAINER:=alex.gronholm@nextday.fi
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host flit/host
HOST_BUILD_DEPENDS:=python-py/host flit/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-exceptiongroup
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Backport of PEP 654 (exception groups)
  URL:=https://pypi.org/project/exceptiongroup/
  DEPENDS:= \
	+python3
endef

define Package/python3-exceptiongroup/description
  Backport of PEP 654 (exception groups)
endef

# flit build setup.pypi
HOST_PYTHON3_VARS+=FLIT_NO_NETWORK=ture
HOST_FLIT:=$(HOST_PYTHON3_DIR)/bin/flit

define FLIT_BUILD
        cd $(PKG_BUILD_DIR); \
		$(HOST_PYTHON3_VARS) \
        $(HOST_FLIT) build --setup-py;
endef

Hooks/Prepare/Post += FLIT_BUILD

define Build/Compile
	$(HOST_PYTHON3_PIP) \
		--disable-pip-version-check \
		--cache-dir "$(DL_DIR)/pip-cache" \
		install \
		--ignore-installed \
		--root=$(PKG_INSTALL_DIR) \
		--prefix=/usr \
		$(PKG_BUILD_DIR)/dist/$(PYPI_NAME)-$(PKG_VERSION)-py3-none-any.whl
endef

define HOST_FLIT_BUILD
        cd $(HOST_BUILD_DIR); \
		$(HOST_PYTHON3_VARS) \
        $(HOST_FLIT) build --setup-py;
endef

Hooks/HostConfigure/Post += HOST_FLIT_BUILD

define Host/Compile
	$(HOST_PYTHON3_PIP) \
		--disable-pip-version-check \
		--cache-dir "$(DL_DIR)/pip-cache" \
		install \
		--ignore-installed \
		$(HOST_BUILD_DIR)/dist/$(PYPI_NAME)-$(PKG_VERSION)-py3-none-any.whl
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-exceptiongroup))
$(eval $(call BuildPackage,python3-exceptiongroup))
