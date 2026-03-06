include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=lcd_fb_de_rtwb_test
PKG_VERSION:=1
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/lcd_fb_de_rtwb_test
	SECTION:=test
	CATEGORY:=TestTools
	SUBMENU:=testapk
	TITLE:=LCD Framebuffer and RTWB Display Interface Test Tool
	DEPENDS:=
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/lcd_fb_de_rtwb_test/description
	Test tool for LCD Framebuffer and RTWB display interface, used to test LCD display functionality,
	DMA buffer operations, display configuration settings, and color bar drawing. Supports configuration
	of different display modes, resolutions, and color formats, suitable for display interface testing
	under Tina Linux system.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL_DIR="$(LINUX_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Package/lcd_fb_de_rtwb_test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lcd_fb_de_rtwb_test $(1)/usr/bin/
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,lcd_fb_de_rtwb_test))
