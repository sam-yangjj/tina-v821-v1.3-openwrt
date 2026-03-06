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
PKG_NAME:=rt_media
PKG_VERSION:=1.0
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia/rt_media
CONFIG_IN_DIR :=
ifeq ($(LICHEE_CHIP), sun8iw21p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw21
    CONFIG_IN_DIR := v853
else ifeq ($(LICHEE_CHIP), sun300iw1p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun300iw1
    CONFIG_IN_DIR := v821
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun252iw1
    CONFIG_IN_DIR := v861
else
    $(error fatal error! $(LICHEE_CHIP) is not support by rt_media!)
endif

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

PKG_BUILD_DEPENDS := libbaselist

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
	SUBMENU:=Multimedia
	SECTION:=rt_media
	CATEGORY:=Allwinner
	TITLE:=rt_media for Tina Linux
	DEPENDS:=+libawion +libglogCWrapper +rt_media_venc_param_debug:libexpat
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/machinfo/$(CONFIG_IN_DIR)/Config.in"
endef

define Package/$(PKG_NAME)/description
	rt-media fast user adapter
endef

define Package/$(PKG_NAME)-demo-common
	SUBMENU:=Multimedia
	SECTION:=$(PKG_NAME)-demo-common
	CATEGORY:=Allwinner
	TITLE:=build rt_media demo common
	DEPENDS:=$(PKG_NAME) +person_detection +motion_detection
endef

define Package/$(PKG_NAME)-demo-common/description
	build rt-media demo common
endef

define Package/$(PKG_NAME)-demo
	SUBMENU:=Multimedia
	SECTION:=$(PKG_NAME)-demo
	CATEGORY:=Allwinner
	TITLE:=build rt_media demo
ifeq ($(LICHEE_CHIP), sun252iw1p1)
	DEPENDS:=$(PKG_NAME) +(rt_media_demo_aiisp):person_detection_rlion +(rt_media_demo_aiisp):libawaiisp
else
	DEPENDS:=$(PKG_NAME) +$(PKG_NAME)-demo-common
endif
endef

define Package/$(PKG_NAME)-demo/config
source "$(SOURCE)/machinfo/$(CONFIG_IN_DIR)/Config-demo.in"
endef

define Package/$(PKG_NAME)-demo/description
	build rt-media demo
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
	$(MAKE) -C $(SRC_COMPILE_DIR)/api_adapter \
		CC="$(TARGET_CC)" AR="$(TARGET_AR)"  \
		CXX="$(TARGET_CXX)" \
		CFLAGS0="$(TARGET_CFLAGS)" \
		CXXFLAGS0="$(TARGET_CXXFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers"

ifneq ($(findstring y,$(CONFIG_PACKAGE_$(PKG_NAME)-demo-common)),)
	$(MAKE) -C $(SRC_COMPILE_DIR)/demo/common \
		CC="$(TARGET_CC)" \
		CFLAGS0="$(TARGET_CFLAGS)" \
		CXXFLAGS0="$(TARGET_CXXFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers"
endif

ifneq ($(findstring y,$(CONFIG_PACKAGE_$(PKG_NAME)-demo)),)
	$(MAKE) -C $(SRC_COMPILE_DIR)/demo \
		CC="$(TARGET_CC)" \
		CFLAGS0="$(TARGET_CFLAGS)" \
		CXXFLAGS0="$(TARGET_CXXFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers"
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/rt_media
	$(INSTALL_DIR) $(1)/usr/include/rt_media
	if [ -d $(SRC_COMPILE_DIR)/api_adapter/InstallDev ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/api_adapter/InstallDev/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/api_adapter/InstallDev not exist?"; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/demo/common/InstallDev ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/demo/common/InstallDev/* $(1); \
	else \
		echo "$(SRC_COMPILE_DIR)/demo/common/InstallDev not exist"; \
	fi
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		$(MAKE) -C $(SRC_COMPILE_DIR)/api_adapter clean; \
		$(MAKE) -C $(SRC_COMPILE_DIR)/demo/common clean; \
		$(MAKE) -C $(SRC_COMPILE_DIR)/demo clean; \
	else \
		echo "$(SRC_COMPILE_DIR) is not exist!"; \
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
	if [ -d $(SRC_COMPILE_DIR)/api_adapter/install ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/api_adapter/install/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/api_adapter/install not exist?"; \
	fi
endef

define Package/$(PKG_NAME)-demo-common/install
	if [ -d $(SRC_COMPILE_DIR)/demo/common/install ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/demo/common/install/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/demo/common/install not exist?"; \
	fi
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
ifneq ($(findstring y,$(CONFIG_rt_media_demo_encoder)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_encoder 			$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_force_i_frame)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_force_i_frame 	$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_get_yuv)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_get_yuv 			$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_isp_api)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_isp_api 			$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_motion_search)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_motion_search 	$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_orl)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_orl 				$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_osd)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_osd 				$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_reset_size)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_reset_size 		$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_roi)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_roi 				$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_video_in_3chn)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_video_in_3chn 	$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_video_in)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_video_in 		$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_pdet)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_pdet 			$(1)/bin
endif
ifneq ($(findstring y,$(CONFIG_rt_media_demo_aiisp)),)
	$(CP) $(SRC_COMPILE_DIR)/demo/demo_aiisp 			$(1)/bin
endif
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo-common))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
