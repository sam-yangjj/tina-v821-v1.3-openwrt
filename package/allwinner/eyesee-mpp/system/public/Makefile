############################################## # OpenWrt Makefile for eyesee-mpp-system
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=eyesee-mpp-system
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/eyesee-mpp/system/public

PKG_BUILD_DEPENDS:= libbaselist

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

############################################################################
# below items are mpp configuration that are needed by local package
#build component to static so if possible
ifeq ($(CONFIG_mpp_compile_static_lib),y)
    MPPCFG_COMPILE_STATIC_LIB := Y
else
    MPPCFG_COMPILE_STATIC_LIB := N
endif
export MPPCFG_COMPILE_STATIC_LIB
#build component to dynamic so if possible
ifeq ($(CONFIG_mpp_compile_dynamic_lib),y)
    MPPCFG_COMPILE_DYNAMIC_LIB := Y
else
    MPPCFG_COMPILE_DYNAMIC_LIB := N
endif
export MPPCFG_COMPILE_DYNAMIC_LIB
#config if enable mpp_vo
ifeq ($(CONFIG_mpp_vo),y)
    MPPCFG_VO := Y
else
    MPPCFG_VO := N
endif
export MPPCFG_VO
export CONFIG_mpp_hw_display

# below items are local package's config.
ifeq ($(CONFIG_mpp_system_libion),y)
    MPPSYSTEMCFG_LIBION := Y
else
    MPPSYSTEMCFG_LIBION := N
endif
export MPPSYSTEMCFG_LIBION

ifeq ($(CONFIG_mpp_system_newfs_msdos),y)
    MPPSYSTEMCFG_NEWFS_MSDOS := Y
else
    MPPSYSTEMCFG_NEWFS_MSDOS := N
endif
export MPPSYSTEMCFG_NEWFS_MSDOS

ifeq ($(CONFIG_mpp_system_reboot_efex),y)
    MPPSYSTEMCFG_REBOOT_EFEX := Y
else
    MPPSYSTEMCFG_REBOOT_EFEX := N
endif
export MPPSYSTEMCFG_REBOOT_EFEX

ifeq ($(CONFIG_mpp_system_luaconfig),y)
    MPPSYSTEMCFG_LUACONFIG := Y
else
    MPPSYSTEMCFG_LUACONFIG := N
endif
export MPPSYSTEMCFG_LUACONFIG

ifeq ($(CONFIG_mpp_system_wifi),y)
    MPPSYSTEMCFG_WIFI := Y
else
    MPPSYSTEMCFG_WIFI := N
endif
export MPPSYSTEMCFG_WIFI

ifeq ($(CONFIG_mpp_system_smartlink),y)
    MPPSYSTEMCFG_SMARTLINK := Y
else
    MPPSYSTEMCFG_SMARTLINK := N
endif
export MPPSYSTEMCFG_SMARTLINK

ifeq ($(CONFIG_mpp_system_rgb_ctrl),y)
    MPPSYSTEMCFG_RGB_CTRL := Y
else
    MPPSYSTEMCFG_RGB_CTRL := N
endif
export MPPSYSTEMCFG_RGB_CTRL

ifeq ($(CONFIG_mpp_system_ntpclient),y)
    MPPSYSTEMCFG_NTPCLIENT := Y
else
    MPPSYSTEMCFG_NTPCLIENT := N
endif
export MPPSYSTEMCFG_NTPCLIENT

export CONFIG_wifi_8189ftv
export CONFIG_wifi_ap6181
export CONFIG_wifi_ap6255
export CONFIG_wifi_ap6355
export CONFIG_wifi_xr819

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=eyesee-mpp-system
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=eyesee-mpp-system for Tina Linux
  DEPENDS:= +libglogCWrapper +mpp_system_luaconfig:liblua +eyesee-mpp-external
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-mpp-system is as eyesee-mpp low level libraries.
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		rm -rf $(SRC_COMPILE_DIR); \
	fi
	ln -s $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(SRC_COMPILE_DIR) -f tina.mk \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		$(MAKE) -C $(SRC_COMPILE_DIR) -f tina.mk \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			clean; \
	else \
		echo "$(SRC_COMPILE_DIR) is not exist!"; \
	fi
endef

define Build/InstallDev
	if [ -d $(SRC_COMPILE_DIR)/InstallDev ]; then \
		$(CP) $(SRC_COMPILE_DIR)/InstallDev/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/InstallDev not exist?"; \
	fi
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	if [ -d $(SRC_COMPILE_DIR)/install ]; then \
		$(CP) $(SRC_COMPILE_DIR)/install/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/install not exist?"; \
	fi
endef

#define Package/$(PKG_NAME)/InstallDev
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
