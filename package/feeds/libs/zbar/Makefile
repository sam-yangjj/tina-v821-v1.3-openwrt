include $(TOPDIR)/rules.mk

PKG_NAME:=zbar
PKG_VERSION:=0.10
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
#PKG_SOURCE_URL:=https://nchc.dl.sourceforge.net/project/zbar/zbar/$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libzbar
	SECTION:=libs
	CATEGORY:=Libraries
	DEPENDS:=
	TITLE:=A library for QR Code scanner
	#URL:=https://nchc.dl.sourceforge.net/project/zbar/zbar/$(PKG_VERSION)
endef

define Package/libzbar/description
	This is an open source QR code identification Library
endef

CONFIGURE_ARGS+= \
	CFLAGS="" \
	--disable-video \
	--enable-shared \
	--enable-static \
	--without-imagemagick \
	--without-jpeg \
	--without-python \
	--without-gtk \
	--without-qt \
	--disable-video

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/include/zbar
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig

	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/zbar/*.h $(1)/usr/include/zbar/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzbar.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

define Package/libzbar/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzbar*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libzbar))
