##############################################
# OpenWrt Makefile for demo_doorlock program
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
PKG_NAME:=demo_doorlock
PKG_VERSION:=0.1
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

############################################################################
#below items are used for mpp configuration of previous EyeseeLinux,
#for smooth migration to tina, we reserve these config items, define them here and export.
#ref to mpp_config.mk of EyeseeLinux.
#[config item], valid in makefile.
#config chip version: AW1721
ifeq ($(LICHEE_CHIP), sun8iw12p1)
    MPPCFG_CHIP := AW1721
else ifeq ($(LICHEE_CHIP), sun8iw16p1)
    MPPCFG_CHIP := AW1816
else ifeq ($(LICHEE_CHIP), sun8iw19p1)
    MPPCFG_CHIP := AW1817
else ifeq ($(LICHEE_CHIP), sun8iw21p1)
    MPPCFG_CHIP := AW1886
else ifeq ($(LICHEE_CHIP), sun300iw1p1)
    MPPCFG_CHIP := AW1882
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
    MPPCFG_CHIP := AW1918
else
    $(error fatal error! $(LICHEE_CHIP) is not support by eyesee-mpp!)
endif
export MPPCFG_CHIP
#config log level for compile
ifeq ($(CONFIG_mpp_log_level_close),y)
	MPPCFG_LOG_LEVEL := CLOSE
else ifeq ($(CONFIG_mpp_log_level_error),y)
    MPPCFG_LOG_LEVEL := ERROR
else ifeq ($(CONFIG_mpp_log_level_warn),y)
    MPPCFG_LOG_LEVEL := WARN
else ifeq ($(CONFIG_mpp_log_level_debug),y)
    MPPCFG_LOG_LEVEL := DEBUG
else ifeq ($(CONFIG_mpp_log_level_verbose),y)
    MPPCFG_LOG_LEVEL := VERBOSE
else
    MPPCFG_LOG_LEVEL := DEBUG
endif
export MPPCFG_LOG_LEVEL
#config write file method: LINUX_VFS, DIRECT_IO
ifeq ($(CONFIG_mpp_fwrite_method_vfs),y)
    MPPCFG_FILE_SYSTEM := LINUX_VFS
else ifeq ($(CONFIG_mpp_fwrite_method_directIO),y)
    MPPCFG_FILE_SYSTEM := DIRECT_IO
else
    MPPCFG_FILE_SYSTEM := LINUX_VFS
endif
export MPPCFG_FILE_SYSTEM

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/eyesee-apps/demo_doorlock
TARGET_CFLAGS += -DTINA_5_0

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/demo_doorlock
	TITLE:=demo_doorlock for Tina Linux
	SUBMENU:=eyesee-apps
	CATEGORY:=Allwinner
	DEPENDS:= +alsa-lib +rt_media
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/demo_doorlock/description
	demo_doorlock is based on rt_media
endef

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
	$(CP) $(SRC_CODE_DIR)/files $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

TARGET_CFLAGS += -DOS_NET_LINUX_OS

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then \
		$(MAKE) -C $(PKG_BUILD_DIR) clean; \
	else \
		echo "$(PKG_BUILD_DIR) is not exist!"; \
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
define Package/demo_doorlock/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/models

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo-doorlock $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/doorlock_usbconfig $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aw_alg/libpix_facekit/models/* $(1)/etc/models/
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,demo_doorlock))
