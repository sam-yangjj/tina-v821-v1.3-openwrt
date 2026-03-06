#
# Copyright (C) 2019 Jakub Piotr Cłapa <jpc@loee.pl>
# Copyright (C) 2020 Alexandru Ardelean <ardeleanalex@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pybind11
PKG_VERSION:=2.10.3
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=08cfe6d4f73746447cc85a400c8169a91608b8a00c5feecd8ff251a70565d12f

PKG_MAINTAINER:=wenzel.jakob@epfl.ch
PKG_LICENSE:=BSD
PKG_LICENSE_FILES:=LICENSE.txt

HOST_BUILD_DEPENDS:=python3/host
PKG_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Seamless operability between C++11 and Python
  URL:=https://pypi.org/project/pybind11/#description
endef

define Host/Compile
	$(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-$(PKG_NAME)))
$(eval $(call BuildPackage,python3-$(PKG_NAME)))
