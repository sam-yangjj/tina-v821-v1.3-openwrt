#
# Copyright (C) 2008-2013 OpenWrt.org
# Copyright (C) 2017-2019 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libunwind
PKG_VERSION:=1.8.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/$(PKG_NAME)
PKG_HASH:=ddf0e32dd5fafe5283198d37e4bf9decf7ba1770b6e7e006c33e6df79e6a6157

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>
PKG_LICENSE:=X11
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:libunwind_project:libunwind

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_CONFIG_DEPENDS:= CONFIG_LIBUNWIND_COREDUMP \
		CONFIG_LIBUNWIND_COREDUMP \
		CONFIG_LIBUNWIND_NTO \
		CONFIG_LIBUNWIND_SETJMP \
		CONFIG_LIBUNWIND_HANDLE_CPP_EXCEPTIONS \
		CONFIG_LIBUNWIND_DYNAMIC_ONLY \
		CONFIG_LIBUNWIND_STATIC_ONLY \
		CONFIG_LIBUNWIND_BUILD_BOTH

include $(INCLUDE_DIR)/package.mk

define Package/libunwind
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The libunwind project
  URL:=http://www.nongnu.org/libunwind/
  DEPENDS:=@((mips||mipsel||mips64||x86_64||arm||aarch64||riscv32||riscv64)||(USE_GLIBC&&(powerpc||i386))) +zlib
  ABI_VERSION:=8
endef

define Package/libunwind/description
  Libunwind defines a portable and efficient C programming interface (API) to determine the call-chain of a program.
endef

CONFIGURE_ARGS += \
	--disable-documentation \
	--disable-tests \
	--disable-minidebuginfo

NEED_LIBUNWIND_PLAT:=n

ifneq ($(CONFIG_LIBUNWIND_COREDUMP),y)
	# build libunwind-coredump library [default=autodetect]
	CONFIGURE_ARGS += --disable-coredump
else
	LIBUNWIND_LIBS += coredump
	NEED_LIBUNWIND_PLAT:=y
endif

ifneq ($(CONFIG_LIBUNWIND_PTRACE),y)
	# build libunwind-ptrace library [default=autodetect]
	CONFIGURE_ARGS += --disable-ptrace
else
	LIBUNWIND_LIBS += ptrace
	NEED_LIBUNWIND_PLAT:=y
endif

ifneq ($(CONFIG_LIBUNWIND_NTO),y)
	# build libunwind-nto library [default=autodetect]
	CONFIGURE_ARGS += --disable-nto
else
	LIBUNWIND_LIBS += nto
	NEED_LIBUNWIND_PLAT:=y
endif

ifneq ($(CONFIG_LIBUNWIND_SETJMP),y)
	# build libunwind-setjmp library [default=autodetect]
	CONFIGURE_ARGS += --disable-setjmp
else
	LIBUNWIND_LIBS += setjmp
	NEED_LIBUNWIND_PLAT:=y
endif

ifneq ($(CONFIG_LIBUNWIND_HANDLE_CPP_EXCEPTIONS),y)
	# use libunwind to handle C++ exceptions [default=autodetect]
	CONFIGURE_ARGS += --disable-cxx-exceptions
endif

#TARGET_LDFLAGS += $(if $(CONFIG_USE_MUSL),-lssp_nonshared)

UNWIND_PLAT:=$(ARCH)
ifeq ($(ARCH),riscv32)
	UNWIND_PLAT:=riscv
endif
ifeq ($(ARCH),riscv64)
	UNWIND_PLAT:=riscv
endif

NEED_LIBUNWIND_DYNAMIC:=y
NEED_LIBUNWIND_STATIC:=y

ifeq ($(CONFIG_LIBUNWIND_DYNAMIC_ONLY),y)
	NEED_LIBUNWIND_STATIC:=n
endif
ifeq ($(CONFIG_LIBUNWIND_STATIC_ONLY),y)
	NEED_LIBUNWIND_DYNAMIC:=n
endif

ifeq ($(NEED_LIBUNWIND_PLAT),y)
	LIBUNWIND_LIBS += $(UNWIND_PLAT)
endif

define Package/libunwind/config
	source "$(SOURCE)/Config.in"
endef

define Package/libunwind/install
	$(INSTALL_DIR) $(1)/usr/lib
ifeq ($(NEED_LIBUNWIND_DYNAMIC),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwind.so* $(1)/usr/lib/
	$(foreach lib,$(LIBUNWIND_LIBS), \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwind-$(lib).so* $(1)/usr/lib/
	)
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
ifeq ($(NEED_LIBUNWIND_DYNAMIC),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwind.so* $(1)/usr/lib
	$(foreach lib,$(LIBUNWIND_LIBS), \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwind-$(lib).so* $(1)/usr/lib/
	)
endif
ifeq ($(NEED_LIBUNWIND_STATIC),y)
	$(foreach lib,$(LIBUNWIND_LIBS), \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwind-$(lib).a* $(1)/usr/lib/
	)
endif
endef

$(eval $(call BuildPackage,libunwind))
