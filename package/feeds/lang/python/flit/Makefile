#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flit
PKG_VERSION:=3.7.1
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=3c9bd9c140515bfe62dd938c6610d10d6efb9e35cc647fc614fe5fb3a5036682

PKG_MAINTAINER:=thomas@kluyver.me.uk
PKG_LICENSE:=BSD License
PKG_LICENSE_FILES:=LICENSE

PKG_HOST_ONLY:=1
HOST_BUILD_DEPENDS:=python3/host python-requests/host python-urllib3/host python-chardet/host python-certifi/host python-idna/host python-docutils/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-flit
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A simple packaging tool for simple packages.
  URL:=https://pypi.org/project/flit
  DEPENDS:= \
	+python3 \
	+python3-requests \
	+python3-urllib3 \
	+python3-chardet \
	+python3-certifi \
	+python3-idna \
	+python3-docutils
  BUILDONLY:=1
endef

define Package/python3-flit/description
  A simple packaging tool for simple packages.
endef

HOST_PYTHON3_VARS+=FLIT_NO_NETWORK=ture

define Host/Compile
	$(call HostPython3/Run,$(HOST_BUILD_DIR),bootstrap_dev.py)
endef

Host/Install:=

$(eval $(call HostBuild))
$(eval $(call Py3Package,python3-flit))
$(eval $(call BuildPackage,python3-flit))
