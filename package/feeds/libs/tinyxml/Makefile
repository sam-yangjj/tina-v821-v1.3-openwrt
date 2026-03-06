include $(TOPDIR)/rules.mk

PKG_NAME:=tinyxml
PKG_VERSION:=2.6.2
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_LICENSE:=BSD

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +libstdcpp
  TITLE:= tinyxml library
endef

define Package/$(PKG_NAME)/description
  TinyXML is a simple, small, C++ XML parser that can be easily integrating into other programs.
endef

define add_CMakeLists
	cp CMakeLists.txt $(PKG_BUILD_DIR)
endef

Hooks/Prepare/Post += add_CMakeLists

CMAKE_OPTIONS=-DTIDEBUG=1 -DTIXML_USE_STL=1

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(STAGING_DIR)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
