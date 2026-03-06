include $(TOPDIR)/rules.mk

PKG_NAME:=argcomplete
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PYPI_NAME:=argcomplete
PKG_HASH:=6372ad78c89d662035101418ae253668445b391755cfe94ea52f1b9d22425b20

PKG_MAINTAINER:=kislyuk@gmail.com
PKG_LICENSE:=Apache Software License

HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Bash tab completion for argparse
  URL:=https://pypi.org/project/argcomplete/
  DEPENDS:=+python3
endef

define Package/python3-$(PKG_NAME)/description
  Bash tab completion for argparse.
endef

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-$(PKG_NAME)))
$(eval $(call BuildPackage,python3-$(PKG_NAME)))
