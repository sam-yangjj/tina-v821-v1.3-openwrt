include $(TOPDIR)/rules.mk

PKG_NAME:=wds
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:= https://github.com/intel/wds/tree/v1.1.0
PKG_MD5SUM:=2070ffb80f6744c6551dee551960f576

PKG_LICENSE:=GNU LGPL
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
CMAKE_BINARY_SUBDIR:=openwrt-build
PKG_JOBS:=-j1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Wireless Display Software For Linux OS
  URL:=https://github.com/intel/wds
  DEPENDS:= +zlib +libstdcpp +glib2 \
	  		+gstreamer1-plugins-aw +gst1-mod-scrcastsrc \
	  		+gst-omx \
			+gstreamer1-plugins-good +gst1-mod-udp +gst1-mod-rtp +gst1-mod-isomp4 +gst1-mod-videofilter\
			+gstreamer1-plugins-bad +gst1-mod-mpegtsmux +gst1-mod-videoparsersbad \
			+libcedarx \
			+gstreamer1-libs +libgstreamer1 +gstreamer1
endef

define Package/$(PKG_NAME)/config
  select OMX if PACKAGE_libcedarx
endef

define Package/$(PKG_NAME)/description
  Wireless Display Software For Linux OS
endef

CMAKE_OPTIONS += \
		-DENABLE_SHARED=ON \
		-DENABLE_STATIC=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/desktop_source/desktop-source-test $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/wds
	$(INSTALL_BIN) ./files/wds.config $(1)/etc/wds/wds.config
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
