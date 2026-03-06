#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=setuptools-scm
PKG_VERSION:=7.0.5
PKG_RELEASE:=1

SOURCE_NAME:=setuptools_scm
PKG_SOURCE:=$(SOURCE_NAME)-$(PKG_VERSION).tar.gz

PYPI_NAME:=$(SOURCE_NAME)
PKG_HASH:=031e13af771d6f892b941adb6ea04545bbf91ebc5ce68c78aaf3fff6e1fb4844

PKG_MAINTAINER:=opensource@ronnypfannschmidt.de
PKG_LICENSE:=MIT License (MIT)
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python3 $(PKG_NAME)/host python-typing-extensions
HOST_BUILD_DEPENDS:=python3/host python-packaging/host python-typing-extensions/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-setuptools-scm
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=the blessed package to manage your versions by scm tags
  URL:=https://pypi.org/project/setuptools-scm
  DEPENDS:= \
	+python3 \
	+python3-packaging \
	+python3-typing-extensions
endef

define Package/python3-setuptools-scm/description
  setuptools_scm extract Python package versions from git or hg metadata instead of declaring them as the version argument or in a SCM managed file.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-setuptools-scm))
$(eval $(call BuildPackage,python3-setuptools-scm))
