include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libcedarc
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia
#PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
PKG_BUILD_PARALLEL := 1
include $(INCLUDE_DIR)/package.mk
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
ifeq ($(KERNEL_PATCHVER),5.15)
CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1020
else
CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v830
endif
CEDARC_VENC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1320/v2
KERNEL_VERSION_ION = CONF_KERNEL_CMA
CEDARC_LD_FLAGS = -ldl
CEDARC_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)
CEDARC_EXTRA_CXXFLAGS =
-include ../machinfo/$(LICHEE_IC)/config.mk
-include ../machinfo/$(LICHEE_IC)/config-linux-$(KERNEL_PATCHVER).mk
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
    LIBCEDARC_DIR = mpp_libcedarc
    NEW_ENC_API = 1
    PKG_FILE_DEPENDS := $(SRC_CODE_DIR)/libtmedia/$(LIBCEDARC_DIR)
else ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
    ifeq ($(LICHEE_CHIP), sun8iw12p1)
        LIBCEDARC_DIR := libcedarc_mpp/v5
    else ifeq ($(LICHEE_CHIP), sun8iw16p1)
        LIBCEDARC_DIR := libcedarc_mpp/v316
    else ifeq ($(LICHEE_CHIP), sun8iw19p1)
        LIBCEDARC_DIR := libcedarc_mpp/sun8iw19p1
    else ifeq ($(LICHEE_CHIP), sun8iw21p1)
        LIBCEDARC_DIR := libcedarc_mpp/sun8iw21
    else ifeq ($(LICHEE_CHIP), sun300iw1p1)
        LIBCEDARC_DIR := libcedarc_mpp/sun300iw1
    else ifeq ($(LICHEE_CHIP), sun252iw1p1)
        LIBCEDARC_DIR := libcedarc_mpp/sun252iw1
    else
        $(error fatal error! $(LICHEE_CHIP) is not support by libcedarc!)
    endif
    SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)
    PKG_FILE_DEPENDS := $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)
else ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
    LIBCEDARC_DIR = libcedarc_v2.0
    FLAG_ENC_VERSION = CONF_VE_ENCODER_VERSION_2
    NEW_ENC_API = 0
    PKG_FILE_DEPENDS := $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)
else
    LIBCEDARC_DIR = libcedarc
    FLAG_ENC_VERSION = CONF_VE_ENCODER_VERSION_1
    NEW_ENC_API = 0
    PKG_FILE_DEPENDS := $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)
endif

ifeq ($(USE_CEDARC_LOW_MEM_CONF),y)
    CEDARC_CONF_FILE_PATH = $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)/conf/low_mem_cedarc.conf
else
    CEDARC_CONF_FILE_PATH = $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)/conf/cedarc.conf
endif

ifeq ($(DISABLE_ISP_VENC_FLAG),y)
    ISP_VENC_FLAG = DISABLE_ENC_ISP
else
    ISP_VENC_FLAG = ENABLE_ENC_ISP
endif

ifeq ($(DISABLE_DEC_ATM_FLAG),y)
    DEC_ATM_FLAG = DISABLE_DEC_ATM
else
    DEC_ATM_FLAG = ENABLE_DEC_ATM
endif

ifeq ($(CONFIG_CEDARC_ENABLE_AFTERTREATMENT),y)
	CONFIG_AFTERTREATMENT += --enable-aftertreatment=yes
else
	CONFIG_AFTERTREATMENT += --enable-aftertreatment=no
endif

ifeq ($(CONFIG_CEDARC_ENABLE_VDECODERDEMO),y)
	CONFIG_VDECODERDEMO += --enable-vdecoderdemo=yes
else
	CONFIG_VDECODERDEMO += --enable-vdecoderdemo=no
endif

ifeq ($(CONFIG_CEDARC_ENABLE_VENCODERDEMO),y)
	CONFIG_VENCODERDEMO += --enable-vencoderdemo=yes
else
	CONFIG_VENCODERDEMO += --enable-vencoderdemo=no
endif

ifeq ($(CONFIG_CEDARC_VENC_DEFAULT_LIB),y)
	CONFIG_VENC_DEFAULT_LIB += --enable-venc-default-lib=yes
else
	CONFIG_VENC_DEFAULT_LIB += --enable-venc-default-lib=no
endif

ifeq ($(CONFIG_CEDARC_VENC_V_LINE_LIB),y)
	CONFIG_VENC_V_LINE_LIB += --enable-venc-v-line-lib=yes
else
	CONFIG_VENC_V_LINE_LIB += --enable-venc-v-line-lib=no
endif

TINA_LINUX_API = 1
VIDEO_DEC_LIBS := MPEG2 \
			 H264 \
			 AVS \
			 H265 \
			 MJPEG \
			 MPEG4BASE \
			 MPEG4DX \
			 MPEG4H263 \
			 MPEG4NORMAL \
			 MPEG4VP6 \
			 VP8 \
			 VP9HW \
			 VP6SOFT

LIBCEDARC_LIBS := SUPPORT_VIDEO_DEC_LIBS \
				   SUPPORT_VDEC_COMMON_LIBS \
				   SUPPORT_VIDEO_ENC_LIBS \
				   SUPPORT_VENC_COMMON_LIBS \
				   SUPPORT_OMX_LIBS
			

SUPPORT_VIDEO_DEC_LIBS = $(foreach c,$(VIDEO_DEC_LIBS), \
				$(if $(CONFIG_CEDAR_VDECODER_$(c)),libaw$(shell echo $(c) | tr '[A-Z]' '[a-z]').so))

ifneq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_MJPEG),libawmjpegplus.so)
endif

SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_VP9),libawvp9HwAL.so)
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libsbm.so libvdecoder.so libVE.so libvideoengine.so libvenc_codec.so)
else ifeq ($(ENABLE_V_LINE_ENC_LIB),y)
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libfbm.so libsbm.so libvdecoder.so libVE.so libvideoengine.so)
else
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libaftertreatment.so libfbm.so libsbm.so libscaledown.so libvdecoder.so libVE.so libvideoengine.so)
endif

ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so)
else ifeq ($(ENABLE_V_LINE_ENC_LIB),y)
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so libvenc_codec.so)
else
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so libvenc_common.so libvenc_h264.so libvenc_h265.so libvenc_jpeg.so)
endif

SUPPORT_VENC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_COMMON_ALL), libcdc_base.so libMemAdapter.so libVE.so)

SUPPORT_OMX_LIBS := $(if $(CONFIG_OMX),libOmxCore.so libOmxVdec.so libOmxVenc.so)

SUPPORT_LIBCEDARC_LIBRARY_ALL := $(foreach c,$(LIBCEDARC_LIBS), $($c))
#config the kernel version,which depends on $(KERNEL_PATCHVER)
ifeq ($(KERNEL_PATCHVER),3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif

ifeq ($(KERNEL_PATCHVER),3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif

ifeq ($(KERNEL_PATCHVER),4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif

ifeq ($(KERNEL_PATCHVER),4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif

ifeq ($(KERNEL_PATCHVER),5.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4
endif

ifeq ($(KERNEL_PATCHVER),5.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_10
endif

ifeq ($(KERNEL_PATCHVER),5.15)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_15
endif

ifeq ($(KERNEL_PATCHVER),6.6)
    KERNEL_VERSION = CONF_KERNEL_VERSION_6_6
endif

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
    CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    CEDARC_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv64")
    CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
endif
#end config the toolchain

#config the cma type
ifeq ($(CONFIG_AW_ION_ALLOC_CMA),y)
    KERNEL_VERSION_ION = CONF_KERNEL_CMA
endif
ifeq ($(CONFIG_AW_ION_ALLOC_IOMMU),y)
    KERNEL_VERSION_ION = CONF_KERNEL_IOMMU
endif

#config the arm libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a7)
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-arm-musl
	ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
	ifneq ($(USE_MPP_LIBCEDARC_DIR),y)
        CEDARC_LIBS_PATH = library/toolchain-sunxi-musl-gcc-v830
	endif
        endif
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-glibc-gcc-v830
	ifeq ($(KERNEL_PATCHVER),5.10)
        CEDARC_LIBS_PATH = library/arm-linux-gnueabihf
        CEDARC_COMPILE_TOOL_CHAIN = arm-linux-gnueabihf
        endif
    endif
endif

#config the aarch64 libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a53)
    ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
        CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = library/toolchain-sunxi-arm-musl
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
	    CEDARC_LIBS_PATH = library/toolchain-sunxi-glibc-gcc-v830
        endif
    else
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-musl
	    ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
                CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-musl-gcc-v830
            endif
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
             CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc
	     ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
             CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v830
     endif
        endif
    endif
endif

#config the arm9 libs path
ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
    ifeq ($(CONFIG_LIBC),"musl")
	CEDARC_LIBS_PATH = library/toolchain-sunxi-arm9-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-arm9-glibc
    endif
endif

#config the riscv libs path
ifeq ($(TARGET_CPU_VARIANT),c910)
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
        CEDARC_LIBS_PATH = library/toolchain-sunxi-riscv-glibc
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-musl
        CEDARC_LIBS_PATH = library/toolchain-sunxi-riscv-musl64
    endif
endif

ifeq ($(TARGET_CPU_VARIANT),c906)
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
        CEDARC_LIBS_PATH = library/toolchain-sunxi-riscv-glibc
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-musl
        CEDARC_LIBS_PATH = library/toolchain-sunxi-riscv-musl64
    endif
endif

#config the aarch64 libs path
ifeq ($(CONFIG_ARCH),"aarch64")
	ifeq ($(CONFIG_LIBC),"glibc")
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"11.3.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1130
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.2.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1020
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"13.2.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1320
			ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
				CEDARC_VENC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1320/v2
			endif
		endif

	endif
endif

ifneq ($(filter y, $(MEDIA_TARGET_BOARD_PLATFORM) $(MEDIA_LIBCEDARC_ENABLE)),)

################################################################################
#libcedarc_mpp source code makefile need use these exported enviroment variables
ifeq ($(USE_LIBCEDARC_MPP_DIR),y)

#config the kernel version,which depends on $(LICHEE_KERN_VER)
ifeq ($(LICHEE_KERN_VER),linux-3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif
ifeq ($(LICHEE_KERN_VER),linux-3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif
ifeq ($(LICHEE_KERN_VER),linux-4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif
ifeq ($(LICHEE_KERN_VER),linux-4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif
ifeq ($(LICHEE_KERN_VER),linux-5.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4
endif
ifeq ($(LICHEE_KERN_VER),linux-5.4-ansc)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4_ANSC
endif
ifneq (,$(findstring linux-5.15,$(LICHEE_KERN_VER)))
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_15
endif
ifeq ($(LICHEE_KERN_VER),linux-6.6-xuantie)
    KERNEL_VERSION = CONF_KERNEL_VERSION_6_6_XUANTIE
endif
export KERNEL_VERSION

#below items are enviroment variables exported to libcedarc_mpp source code makfile.
#config if compile venc libs(all).
ifeq ($(CONFIG_libcedarc_venc),y)
    LIBCEDARCCFG_VENC := Y
else
    LIBCEDARCCFG_VENC := N
endif
export LIBCEDARCCFG_VENC
#config if use venc_h264
ifeq ($(CONFIG_libcedarc_venc_h264),y)
    USE_VENC_H264 := true
else
    USE_VENC_H264 := false
endif
export USE_VENC_H264
#config if use venc_h265
ifeq ($(CONFIG_libcedarc_venc_h265),y)
    USE_VENC_H265 := true
else
    USE_VENC_H265 := false
endif
export USE_VENC_H265
#config if use venc_jpeg
ifeq ($(CONFIG_libcedarc_venc_jpeg),y)
    USE_VENC_JPEG := true
else
    USE_VENC_JPEG := false
endif
export USE_VENC_JPEG
#config if enable venc_support_ext_param
ifeq ($(CONFIG_libcedarc_venc_support_ext_param),y)
    VENC_SUPPORT_EXT_PARAM := Y
else
    VENC_SUPPORT_EXT_PARAM := N
endif
export VENC_SUPPORT_EXT_PARAM
#config if compile vdec libs(all)
ifeq ($(CONFIG_libcedarc_vdec),y)
    LIBCEDARCCFG_VDEC := Y
else
    LIBCEDARCCFG_VDEC := N
endif
export LIBCEDARCCFG_VDEC
#config if use vdec_h264
ifeq ($(CONFIG_libcedarc_vdec_h264),y)
    USE_VDEC_H264 := true
else
    USE_VDEC_H264 := false
endif
export USE_VDEC_H264
#config if use vdec_h265
ifeq ($(CONFIG_libcedarc_vdec_h265),y)
    USE_VDEC_H265 := true
else
    USE_VDEC_H265 := false
endif
export USE_VDEC_H265
#config if use vdec_jpeg
ifeq ($(CONFIG_libcedarc_vdec_jpeg),y)
    USE_VDEC_JPEG := true
else
    USE_VDEC_JPEG := false
endif
export USE_VDEC_JPEG

#config if use iommu
ifeq ($(CONFIG_libcedarc_iommu),y)
    USE_IOMMU := true
else
    USE_IOMMU := false
endif
export USE_IOMMU

#config if use ion size pool heap
ifeq ($(CONFIG_libcedarc_ion_size_pool_heap),y)
    USE_ION_SIZE_POOL_HEAP := true
else
    USE_ION_SIZE_POOL_HEAP := false
endif
export USE_ION_SIZE_POOL_HEAP

#compile static libs
ifeq ($(CONFIG_libcedarc_compile_static_lib),y)
    LIBCEDARCCFG_COMPILE_STATIC_LIB := Y
else
    LIBCEDARCCFG_COMPILE_STATIC_LIB := N
endif
export LIBCEDARCCFG_COMPILE_STATIC_LIB
#compile dynamic libs
ifeq ($(CONFIG_libcedarc_compile_dynamic_lib),y)
    LIBCEDARCCFG_COMPILE_DYNAMIC_LIB := Y
else
    LIBCEDARCCFG_COMPILE_DYNAMIC_LIB := N
endif
export LIBCEDARCCFG_COMPILE_DYNAMIC_LIB
#toolchain libc
ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
    LIBCEDARCCFG_TOOLCHAIN_LIBC := musl
else ifeq ($(CONFIG_TOOLCHAIN_LIBC), "glibc")
    LIBCEDARCCFG_TOOLCHAIN_LIBC := glibc
endif
export LIBCEDARCCFG_TOOLCHAIN_LIBC
#config if compile patten demo
ifeq ($(CONFIG_libcedarc_pattern_demo),y)
    COMPILE_PATTERN_DEMO := Y
else
    COMPILE_PATTERN_DEMO := N
endif
export COMPILE_PATTERN_DEMO
#config if compile vencode demo
ifeq ($(CONFIG_libcedarc_vencode_demo),y)
    COMPILE_VENCODE_DEMO := Y
else
    COMPILE_VENCODE_DEMO := N
endif
export COMPILE_VENCODE_DEMO

export CONFIG_libcedarc_vdecode_demo
export CONFIG_libcedarc_jpeg_demo

endif #end of 'ifeq ($(USE_LIBCEDARC_MPP_DIR),y)'
################################################################################

define Package/$(PKG_NAME)/config
source "$(SOURCE)/../machinfo/$(LICHEE_IC)/$(PKG_NAME)/Config.in"
endef

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=$(PKG_NAME) for Allwinner
ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
  DEPENDS:=+libcedarc_venc_support_ext_param:libexpat
else
  DEPENDS:=+libstdcpp
endif
endef

PKG_CONFIG_DEPENDS += CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL \
                      CONFIG_CEDAR_VIDEO_ENCODER_ALL

define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
  $(PKG_NAME) provides video codec function.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
	$(CP) -Lvr $(SRC_CODE_DIR)/libtmedia/$(LIBCEDARC_DIR)/* $(PKG_BUILD_DIR)/
else ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		rm -rf $(SRC_COMPILE_DIR); \
	fi
	ln -s $(SRC_CODE_DIR)/$(LIBCEDARC_DIR) $(SRC_COMPILE_DIR)
else
	$(CP) -Lvr $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)/* $(PKG_BUILD_DIR)/
endif
endef

define Build/Configure

ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
else
	@echo "********target_board_platform = $MEDIA_TARGET_BOARD_PLATFORM***********"
	@echo "********chip type = $(TINA_CHIP_TYPE)*********"
	@echo "********kernel version = $(KERNEL_PATCHVER)*********"
	@echo "********Adapting and Checking KERNEL_VERSION = $(KERNEL_VERSION)*********"
	@echo "********Adapting and Checking KERNEL_VERSION_ION = $(KERNEL_VERSION_ION)*********"
	@echo "********toolchain = $(CONFIG_ARCH)_$(CONFIG_LIBC)*********"
	@echo "********toolchain-version = $(CONFIG_TOOLCHAIN_GCC_VERSION)*******"
	@echo "********cpu arch = $(TARGET_CPU_VARIANT)*********"
	@echo "********Adapting and Checking cedarc_libs_path: $(CEDARC_LIBS_PATH)*********"
	@echo "********Adapting and Checking cedarc compile tool chain: $(CEDARC_COMPILE_TOOL_CHAIN)*********"
	@echo "********Adapting and Checking STAGING_DIR: $(STAGING_DIR)*********"
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/install --host=$(CEDARC_COMPILE_TOOL_CHAIN) $(CONFIG_AFTERTREATMENT) $(CONFIG_VDECODERDEMO) $(CONFIG_VENCODERDEMO) $(CONFIG_VENC_DEFAULT_LIB) $(CONFIG_VENC_V_LINE_LIB) CFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(FLAG_ENC_VERSION) -D$(DEC_ATM_FLAG) -Wno-format-overflow" CPPFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(FLAG_ENC_VERSION) -D$(DEC_ATM_FLAG)" LDFLAGS="$(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)  -L$(PKG_BUILD_DIR)/$(CEDARC_VENC_LIBS_PATH) -L$(PKG_BUILD_DIR)/install/lib"
endif
endef

define Build/Compile

ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
	$(MAKE) $(PKG_JOBS) -C $(SRC_COMPILE_DIR) -f tina.mk \
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
else
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(FLAG_ENC_VERSION) -D$(DEC_ATM_FLAG) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API) $(CEDARC_EXTRA_CXXFLAGS) -Wno-format-overflow -Wno-format -Wno-format-security" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(FLAG_ENC_VERSION) -D$(DEC_ATM_FLAG) $(CEDARC_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH) -L$(PKG_BUILD_DIR)/$(CEDARC_VENC_LIBS_PATH) -L$(PKG_BUILD_DIR)/install/lib" \
		install
endif
endef

ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		LIBCEDARCCFG_COMPILE_DYNAMIC_LIB=Y \
		LIBCEDARCCFG_COMPILE_STATIC_LIB=Y \
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
endif

define Build/InstallDev

ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
	if [ -d $(SRC_COMPILE_DIR)/InstallDev ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/InstallDev/* $(1); \
	else \
		@echo "fatal error! why $(SRC_COMPILE_DIR)/InstallDev not exist?"; \
	fi
else
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/v2/*.so							$(PKG_INSTALL_DIR)/usr/lib/
endif
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	
	cd $(PKG_INSTALL_DIR)/usr/lib/
	## copy header files.
	
	## cdc_log.h
	$(CP) $(PKG_BUILD_DIR)/base/include/cdc_log.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## $(LIBCEDARC_DIR) vdecoder/vencoder
	-cp -r $(PKG_BUILD_DIR)/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/base/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/vencoder/base/include										$(PKG_INSTALL_DIR)/usr/include/allwinner/

	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr
endif
endef

define Package/$(PKG_NAME)/install

ifeq ($(USE_LIBCEDARC_MPP_DIR),y)
	if [ -d $(SRC_COMPILE_DIR)/install ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/install/* $(1); \
	else \
		@echo "fatal error! why $(SRC_COMPILE_DIR)/install not exist?"; \
	fi
else
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	mkdir -p $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so $(PKG_BUILD_DIR)/all_libs
ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/v2/*.so $(PKG_BUILD_DIR)/all_libs
endif
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so $(PKG_BUILD_DIR)/all_libs
	
	echo "**install path:$(1)***"

	for n in $(SUPPORT_LIBCEDARC_LIBRARY_ALL); \
	do \
		$(CP) $(PKG_BUILD_DIR)/all_libs/$$$$n $(1)/usr/lib; \
	done

ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
	$(CP) $(PKG_BUILD_DIR)/config/low_mem_cedarc.conf $(PKG_BUILD_DIR)/all_libs/cedarc.conf
else
	$(CP) $(CEDARC_CONF_FILE_PATH) $(PKG_BUILD_DIR)/all_libs/cedarc.conf
endif
	$(CP)          $(PKG_BUILD_DIR)/all_libs/cedarc.conf						 $(1)/etc/cedarc.conf

	cd $(1)/usr/lib/; rm -rf $(NOT_SUPPORT_VIDEO_LIBS)
endif
endef
	
endif #end of 'ifneq ($(filter y, $(MEDIA_TARGET_BOARD_PLATFORM) $(MEDIA_LIBCEDARC_ENABLE)),)'

ifneq ($(filter y, $(MEDIA_TARGET_BOARD_PLATFORM) $(MEDIA_LIBCEDARC_ENABLE)),)
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
