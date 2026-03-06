include $(TOPDIR)/rules.mk

PKG_NAME:=docutils
PKG_VERSION:=0.19
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=33995a6753c30b7f577febfc2c50411fec6aac7f7ffeb7c4cfe5991072dcf9e6

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT

HOST_BUILD_DEPENDS:=python3/host

include ../pypi.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-docutils
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=docutils
  URL:=http://docutils.sourceforge.net
  DEPENDS:=+python3 \
	+python3-distutils
endef

define Package/python3-docutils/description
Docutils is a modular system for processing documentation into useful formats,
such as HTML, XML, and LaTeX. For input Docutils supports reStructuredText,
an easy-to-read, what-you-see-is-what-you-get plaintext markup syntax.
endef

PYTHON3_PKG_SETUP_ARGS:=

define Host/Compile
        $(call HostPython3/ModSetup,,install --prefix="" --root="$(STAGING_DIR_HOSTPKG)")
endef

Host/Install:=

$(eval $(call HostBuild))

$(eval $(call Py3Package,python3-docutils))
$(eval $(call BuildPackage,python3-docutils))
$(eval $(call BuildPackage,python3-docutils-src))
