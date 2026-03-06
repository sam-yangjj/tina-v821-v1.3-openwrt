#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-tomli
PKG_VERSION:=2.0.1
PKG_RELEASE:=1

PYPI_NAME:=tomli
PKG_HASH:=de526c12914f0c550d15924c62d72abc48d6fe7364aa87328337a31007fe8a4f

PKG_MAINTAINER:=hukkin@users.noreply.github.com
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DEPENDS:=flit/host python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-tomli
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A lil' TOML parser
  URL:=https://pypi.org/project/tomli
  DEPENDS:=+python3
endef

define Package/python3-tomli/description
  A lil' TOML parser
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

Hooks/HostPrepare/Post += HOST_FLIT_BUILD

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
$(eval $(call Py3Package,python3-tomli))
$(eval $(call BuildPackage,python3-tomli))
