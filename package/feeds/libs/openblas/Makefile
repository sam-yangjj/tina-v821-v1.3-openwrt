include $(TOPDIR)/rules.mk

PKG_NAME:=OpenBLAS
PKG_VERSION:=0.3.23
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_INSTALL:=1

PKG_LICENSE:=BSD

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:= an optimized BLAS library
endef

define Package/$(PKG_NAME)/description
	OpenBLAS is an optimized BLAS library based on GotoBLAS2 1.13 BSD version.
	Please read the documents on OpenBLAS wiki pages http://github.com/xianyi/OpenBLAS/wiki.
endef

PKG_JOBS:=-j
CMAKE_OPTIONS += -DBUILD_TESTING=OFF \
		-DBUILD_SHARED_LIBS=ON \
#		-DBUILD_WITHOUT_LAPACK=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenblas*.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenblas*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
