include $(TOPDIR)/rules.mk
-include machinfo/$(LICHEE_IC)/build.mk

PKG_NAME:=person_detection
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_mpp_compile_static_lib),y)
    MPPCFG_COMPILE_STATIC_LIB := Y
else
    MPPCFG_COMPILE_STATIC_LIB := N
endif
export MPPCFG_COMPILE_STATIC_LIB
ifeq ($(CONFIG_mpp_compile_dynamic_lib),y)
    MPPCFG_COMPILE_DYNAMIC_LIB := Y
else
    MPPCFG_COMPILE_DYNAMIC_LIB := N
endif
export MPPCFG_COMPILE_DYNAMIC_LIB

define Package/$(PKG_NAME)
  SUBMENU:=Vision
  SECTION:=allwinner
  CATEGORY:=Allwinner
  TITLE:=person detection for allwinner
  DEPENDS:=+libstdcpp
endef

define Package/$(PKG_NAME)/description
  person detection for allwinner
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src
	$(CP) ./src/* $(PKG_BUILD_DIR)/src
endef


ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
	LIB_PERSON_DETECTION_TOOLCHAIN_LIBC := musl
else ifeq ($(CONFIG_TOOLCHAIN_LIBC), "glibc")
	LIB_PERSON_DETECTION_TOOLCHAIN_LIBC := glibc
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		PERSON_DETECTION_LIBC="$(LIB_PERSON_DETECTION_TOOLCHAIN_LIBC)" \
		all
endef

define Build/InstallDev
	if [ -d $(PKG_BUILD_DIR)/src/InstallDev ]; then \
		$(CP) -r $(PKG_BUILD_DIR)/src/InstallDev/* $(1); \
	else \
		echo "fatal error! why $(PKG_BUILD_DIR)/src/InstallDev not exist?"; \
	fi
endef

define Package/$(PKG_NAME)/install
	if [ -d $(PKG_BUILD_DIR)/src/install ]; then \
		$(CP) -r $(PKG_BUILD_DIR)/src/install/* $(1); \
	else \
		echo "$(PKG_BUILD_DIR)/src/install not exist."; \
	fi
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then \
		$(MAKE) -C $(PKG_BUILD_DIR)/src clean; \
	else \
		echo "$(PKG_BUILD_DIR) is not exist!"; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
