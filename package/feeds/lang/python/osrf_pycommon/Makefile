#
# Copyright (C) 2019 Jakub Piotr Cłapa <jpc@loee.pl>
# Copyright (C) 2020 Alexandru Ardelean <ardeleanalex@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=osrf_pycommon
PKG_VERSION:=2.1.2
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)

PKG_LICENSE:=Apache-2.0 license
PKG_LICENSE_FILES:=LICENSE

# yes, zip... sigh
PYPI_SOURCE_EXT:=tar.gz
PKG_MD5SUM:=a242aab376dfc04c4235ce37a207fbb4
PKG_BUILD_DEPENDS:= python3

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Commonly needed Python modules, used by Python software developed at OSRF
  URL:=https://github.com/osrf/osrf_pycommon
  DEPENDS:=+python3
endef

$(eval $(call Py3Package,python3-$(PKG_NAME)))
$(eval $(call BuildPackage,python3-$(PKG_NAME)))
