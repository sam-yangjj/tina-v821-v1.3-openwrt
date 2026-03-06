include $(TOPDIR)/rules.mk

PKG_NAME:=lmbench3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.bitmover.com/lmbench/lmbench3.tar.gz
PKG_MD5SUM:=79f1861dfdd0110c6dd9d24d1d5473e7

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=librpc

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=lmbench microbenchmarks
  URL:=http://sourceforge.net/projects/lmbench
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  lmbench is a series of micro benchmarks intended to measure basic operating
  system and hardware system metrics.
endef

MAKE_FLAGS += OS="$(ARCH)"

define PartGen
define Package/$(PKG_NAME)-$(subst _,-,$(1))
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=$(1) microbenchmark
  URL:=http://sourceforge.net/projects/lmbench
  DEPENDS:=$(PKG_NAME) +librpc
endef
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/$(PKG_NAME)/bin/$(ARCH)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/$(ARCH)/lmbench $(1)/usr/sbin/$(PKG_NAME)/bin/$(ARCH)/

	$(INSTALL_DIR) $(1)/usr/sbin/$(PKG_NAME)/scripts
	$(CP) $(PKG_BUILD_DIR)/scripts/* $(1)/usr/sbin/$(PKG_NAME)/scripts
	$(INSTALL_BIN) ./run.sh $(1)/usr/sbin/$(PKG_NAME)/scripts

	$(INSTALL_DIR) $(1)/usr/sbin/$(PKG_NAME)/results
	$(CP) $(PKG_BUILD_DIR)/results/* $(1)/usr/sbin/$(PKG_NAME)/results
endef

define PartInstall
define Package/$(PKG_NAME)-$(subst _,-,$(1))/install
	$(INSTALL_DIR) $$(1)/usr/sbin/$(PKG_NAME)/bin/$(ARCH)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/$(ARCH)/$(1) $$(1)/usr/sbin/$(PKG_NAME)/bin/$(ARCH)/$(1)
endef
endef

LMBENCH_FILES:= \
	bw_file_rd \
	bw_mem \
	bw_mmap_rd \
	bw_pipe \
	bw_tcp \
	bw_unix \
	cache \
	disk \
	enough \
	flushdisk \
	hello \
	lat_cmd \
	lat_connect \
	lat_ctx \
	lat_dram_page \
	lat_fcntl \
	lat_fifo \
	lat_fs \
	lat_http \
	lat_mem_rd \
	lat_mmap \
	lat_ops \
	lat_pagefault \
	lat_pipe \
	lat_pmake \
	lat_proc \
	lat_rand \
	lat_rpc \
	lat_select \
	lat_sem \
	lat_sig \
	lat_syscall \
	lat_tcp \
	lat_udp \
	lat_unix \
	lat_unix_connect \
	lat_usleep \
	line \
	lmdd \
	lmhttp \
	loop_o \
	memsize \
	mhz \
	msleep \
	par_mem \
	par_ops \
	stream \
	timing_o \
	tlb

$(foreach file,$(LMBENCH_FILES),$(eval $(call PartGen,$(file))))
$(foreach file,$(LMBENCH_FILES),$(eval $(call PartInstall,$(file))))

$(eval $(call BuildPackage,$(PKG_NAME)))
$(foreach file,$(LMBENCH_FILES),$(eval $(call BuildPackage,$(PKG_NAME)-$(subst _,-,$(file)))))
