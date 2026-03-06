#
# Copyright (C) 2019 Jakub Piotr Cłapa <jpc@loee.pl>
# Copyright (C) 2020 Alexandru Ardelean <ardeleanalex@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=Cython
PKG_VERSION:=0.29.32

PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=8733cf4758b79304f2a4e39ebfac5e92341bce47bcceb26c1254398b2f8c1af7

PKG_MAINTAINER:=cython-devel@python.org

PKG_LICENSE:=Apache Software License (Apache)

# yes, zip... sigh
PYPI_SOURCE_EXT:=tar.gz
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=python3
HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=The Cython compiler for writing C extensions for the Python language.
  URL:=https://pypi.org/project/Cython/
  DEPENDS:=+python3
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-$(PKG_NAME)))
$(eval $(call BuildPackage,python3-$(PKG_NAME)))
