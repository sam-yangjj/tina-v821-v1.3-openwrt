include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2.8
PKG_RELEASE:=1
PKG_NAME:=libcedarx

PKG_BUILD_DIR := $(BUILD_DIR)/${PKG_NAME}
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
PKG_BUILD_PARALLEL := 1
include $(INCLUDE_DIR)/package.mk

#init the default value
TINA_CHIP_TYPE = G102
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux


CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-glibc-gcc-v830

KERNEL_VERSION_ION = CONF_KERNEL_CMA
COMPILE_CEDARX_LEVEL =
CONF_ONLY_DISABLE_AUDIO =

CEDARX_LD_FLAGS = -ldl -lcrypto -lz
CEDARX_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)
CEDARX_EXTRA_CXXFLAGS =
CEDARX_CONFIGURE_ARGS =
TINA_LINUX_API = 1
-include ../machinfo/$(LICHEE_IC)/config.mk
-include ../machinfo/$(LICHEE_IC)/config-linux-$(KERNEL_PATCHVER).mk
-include ../machinfo/$(LICHEE_IC)/$(PKG_NAME)/config_parser.mk
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
    NEW_ENC_API = 1
else ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
    FLAG_ENC_VERSION = CONF_VE_ENCODER_VERSION_2
    NEW_ENC_API = 0
else
    FLAG_ENC_VERSION = CONF_VE_ENCODER_VERSION_1
    NEW_ENC_API = 0
endif

AUDIO_DEC_LIBS := AAC \
			 ALAC \
			 AMR \
			 APE \
			 ATRC \
			 COOK \
			 FLAC \
			 MP3 \
			 OGG \
			 SIPR \
			 WAV \
			 RA \
			 DSD \
			 G729 \
			 OPUS


XPLAYER_MULTIMEDIA_LIBS := SUPPORT_AUDIO_DEC_LIBS \
				   SUPPORT_SUBTITLE_DEC_LIBS \
				   SUPPORT_LIVE555_LIBS \
				   SUPPORT_XPLAYER_PARSER \
				   SUPPORT_XPLAYER_STREAM \
				   SUPPORT_XPLAYER_COMMON \
				   SUPPORT_AUDIO_ENC_LIBS \
				   SUPPORT_XRECORDER_MUXER \
				   SUPPORT_XRECORDER_COMMON \
				   SUPPORT_XMETADATA 
ifneq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
XPLAYER_MULTIMEDIA_LIBS += SUPPORT_ADEC_COMMON_LIBS
SUPPORT_AUDIO_DEC_LIBS = $(foreach c,$(AUDIO_DEC_LIBS), \
				$(if $(CONFIG_CEDAR_ADECODER_$(c)),libaw_$(shell echo $(c) | tr '[A-Z]' '[a-z]')dec.so))
endif

SUPPORT_SUBTITLE_DEC_LIBS := $(if $(CONFIG_CEDAR_SUBTITLE_SUPPORT),libsubdecoder.so)

SUPPORT_LIVE555_LIBS := $(if $(CONFIG_XPLAYER),liblive555.so)


ifneq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
SUPPORT_ADEC_COMMON_LIBS := $(if $(CONFIG_XPLAYER),libadecoder.so)
endif
SUPPORT_XPLAYER_PARSER := $(if $(CONFIG_CEDAR_PARSER_ALL),libcdx_parser.so)

SUPPORT_XPLAYER_STREAM := $(if $(CONFIG_CEDAR_STREAM_ALL),libcdx_stream.so)

SUPPORT_XPLAYER_COMMON := $(if $(CONFIG_XPLAYER),libcdx_base.so libcdx_common.so libcdx_playback.so libxplayer.so)


SUPPORT_AUDIO_ENC_LIBS := $(if $(CONFIG_CEDAR_AENCODER_ALL),libaencoder.so)


SUPPORT_AENC_COMMON_LIBS :=

SUPPORT_XRECORDER_MUXER := $(if $(CONFIG_CEDAR_MUXER_ALL),libcdx_muxer.so)

SUPPORT_XRECORDER_COMMON := $(if $(CONFIG_XRECORDER),libawrecorder.so libcdx_base.so libcdx_common.so)

## TMETA related libraries
SUPPORT_XMETADATA := $(if $(CONFIG_TMETADATARETRIVER),libxmetadataretriever.so)

XPLAYER_SUPPORT_LIBRARY_ALL := $(foreach c,$(XPLAYER_MULTIMEDIA_LIBS), $($c))

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
#end config the kernel version

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
    CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    CEDARX_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv64")
    CEDARX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
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
        CEDARX_LIBS_PATH = external/lib32/openwrt-arm-musl
        ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-musl-gcc-v830
        endif
        ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"13.2.0")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-musl-gcc-v1320
        endif
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARX_LIBS_PATH = external/lib32/openwrt-arm-glibc
        ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-glibc-gcc-v830
        endif
        ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"13.2.0")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-glibc-gcc-v1320
        endif
        ifeq ($(KERNEL_PATCHVER),5.10)
            CEDARX_LIBS_PATH = external/lib32/arm-linux-gnueabihf
            CEDARX_COMPILE_TOOL_CHAIN = arm-linux-gnueabihf
        endif
    endif
endif

#config the aarch64 libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a53)
    ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
        CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-musl
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
            CEDARX_LIBS_PATH = external/lib32/openwrt-arm-glibc
        endif
    else
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARX_LIBS_PATH = external/lib32/openwrt-aarch64-musl
            ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
                CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-musl-gcc-v830
            endif
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
             CEDARX_LIBS_PATH = external/lib32/openwrt-aarch64-glibc
             ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
                 CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-glibc-gcc-v830
             endif
        endif
    endif
endif

#config the arm9 libs path
ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARX_LIBS_PATH = external/lib32/openwrt-arm926-musl
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARX_LIBS_PATH = external/lib32/openwrt-arm926-glibc
    endif
endif

#config the riscv libs path
ifeq ($(TARGET_CPU_VARIANT),c910)
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
        CEDARX_LIBS_PATH = external/lib64/openwrt-riscv-glibc
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-musl
        CEDARX_LIBS_PATH = external/lib64/openwrt-riscv-musl
    endif
endif

ifeq ($(TARGET_CPU_VARIANT),c906)
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
        CEDARX_LIBS_PATH = external/lib64/openwrt-riscv-glibc
    endif
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-musl
        CEDARX_LIBS_PATH = external/lib64/openwrt-riscv-musl
    endif
endif

#config the aarch64 libs path
ifeq ($(CONFIG_ARCH),"aarch64")
	ifeq ($(CONFIG_LIBC),"glibc")
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"11.3.0")
			CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-glibc-gcc-v1130
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.2.0")
			CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-glibc-gcc-v1020
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"13.2.0")
			CEDARX_LIBS_PATH = external/lib64/openwrt-aarch64-glibc-gcc-v1320
		endif

	endif
endif

ifeq ($(CONFIG_CEDAR_FILE_STREAM),y)
	CONFIG_STREAM += --enable-file-stream=yes
else
	CONFIG_STREAM += --enable-file-stream=no
endif

ifeq ($(CONFIG_CEDAR_HTTP_STREAM),y)
	CONFIG_STREAM += --enable-http-stream=yes
else
	CONFIG_STREAM += --enable-http-stream=no
endif

ifeq ($(CONFIG_SSL_ENABLE),y)
	CONFIG_STREAM += --enable-ssl=yes
else
	CONFIG_STREAM += --enable-ssl=no
endif

ifeq ($(CONFIG_CEDAR_RTSP_STREAM),y)
	CONFIG_STREAM += --enable-rtsp-stream=yes
else
	CONFIG_STREAM += --enable-rtsp-stream=no
endif

ifeq ($(CONFIG_CEDAR_RTMP_STREAM),y)
	CONFIG_STREAM += --enable-rtmp-stream=yes
else
	CONFIG_STREAM += --enable-rtmp-stream=no
endif

ifeq ($(CONFIG_CEDAR_MP3_MUXER),y)
	CONFIG_MUXER += --enable-mp3-muxer=yes
else
	CONFIG_MUXER += --enable-mp3-muxer=no
endif

ifeq ($(CONFIG_CEDAR_AAC_MUXER),y)
	CONFIG_MUXER += --enable-aac-muxer=yes
else
	CONFIG_MUXER += --enable-aac-muxer=no
endif

ifeq ($(CONFIG_CEDAR_TS_MUXER),y)
	CONFIG_MUXER += --enable-ts-muxer=yes
else
	CONFIG_MUXER += --enable-ts-muxer=no
endif

ifeq ($(CONFIG_CEDAR_MP4_MUXER),y)
	CONFIG_MUXER += --enable-mp4-muxer=yes
else
	CONFIG_MUXER += --enable-mp4-muxer=no
endif

ifeq ($(CONFIG_CEDAR_MUXER_DEMO),y)
	CONFIG_DEMO += --enable-muxer-test=yes
else
	CONFIG_DEMO += --enable-muxer-test=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_DEMO),y)
	CONFIG_DEMO += --enable-parser-demo=yes
else
	CONFIG_DEMO += --enable-parser-demo=no
endif

ifeq ($(CONFIG_CEDAR_VDEC_DEMO),y)
	CONFIG_DEMO += --enable-vdec-demo=yes
else
	CONFIG_DEMO += --enable-vdec-demo=no
endif

ifeq ($(CONFIG_CEDAR_VENC_DEMO),y)
	CONFIG_DEMO += --enable-venc-demo=yes
else
	CONFIG_DEMO += --enable-venc-demo=no
endif

ifeq ($(CONFIG_CEDAR_XPLAYER_DEMO),y)
	CONFIG_DEMO += --enable-xplayer-demo=yes
else
	CONFIG_DEMO += --enable-xplayer-demo=no
endif

ifeq ($(CONFIG_CEDAR_XMETA_DEMO),y)
	CONFIG_DEMO += --enable-xmeta-demo=yes
else
	CONFIG_DEMO += --enable-xmeta-demo=no
endif

ifeq ($(CONFIG_CEDAR_JPEG_DEMO),y)
	CONFIG_DEMO += --enable-jpeg-demo=yes
else
	CONFIG_DEMO += --enable-jpeg-demo=no
endif

ifeq ($(CONFIG_CEDAR_RECODER_DEMO),y)
	CONFIG_DEMO += --enable-recoder-demo=yes
else
	CONFIG_DEMO += --enable-recoder-demo=no
endif

ifeq ($(CONFIG_CEDAR_OMX_DEMO),y)
	CONFIG_DEMO += --enable-omx-demo=yes
else
	CONFIG_DEMO += --enable-omx-demo=no
endif

ifeq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	CONF_ONLY_ENABLE_AUDIO += --enable-only-audio=yes
else
	CONF_ONLY_ENABLE_AUDIO += --enable-only-audio=no
endif

ifeq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
	CONF_ONLY_DISABLE_AUDIO += --enable-only-disable-audio=yes
else
	CONF_ONLY_DISABLE_AUDIO += --enable-only-disable-audio=no
endif

ifeq ($(CONFIG_VDEC_SAVE_YUV),y)
	CONFIG_SAVE_YUV_DEBUG += --enable-vdec-saveyuv=yes
else
	CONFIG_SAVE_YUV_DEBUG += --enable-vdec-saveyuv=no
endif

ifeq ($(CONFIG_CEDAR_DISABLE_CEDARC), y)
	CONFIG_ENABLE_CEDRAC += --enable-cedarc-build=no
else
	CONFIG_ENABLE_CEDRAC += --enable-cedarc-build=yes
endif

#ifeq ($(KERNEL_PATCHVER),5.15)
#	CONF_ONLY_DISABLE_AUDIO = -DONLY_DISABLE_AUDIO
#endif

ifeq ($(CONFIG_CEDAR_DISABLE_CEDARC), y)
ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
    PKG_EXNAME:=libcedarc_v2.0
    PKG_BUILD_EXDIR := $(BUILD_DIR)/$(PKG_EXNAME)
else
    PKG_EXNAME:=libcedarc
    PKG_BUILD_EXDIR := $(BUILD_DIR)/$(PKG_EXNAME)
endif
endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)

define Package/$(PKG_NAME)/config
source "$(SOURCE)/../machinfo/$(LICHEE_IC)/$(PKG_NAME)/Config.in"
endef

define Package/$(PKG_NAME)
	SUBMENU:=Multimedia
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=$(PKG_NAME)  for allwinner
	DEPENDS:=+libpthread \
	   +!CEDAR_DISABLE_CEDARC:libcedarc \
	   +zlib \
	   +libstdcpp \
	   +SSL_ENABLE:libopenssl \
	   +CEDAR_PARSER_HLS:libopenssl \
	   +CEDAR_RTSP_STREAM:live \
	   +AUDIO_SUPPORT:alsa-lib 
endef

ifeq ($(CONFIG_CEDAR_PARSER_RAW_STREAM), y)
	CEDARX_CONFIGURE_ARGS += --enable-raw-stream=yes
endif

ifeq ($(CONFIG_PACKAGE_xplayerdemo),y)
    CEDARX_CONFIGURE_ARGS += --enable-xplayerdemo=yes
endif

define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	$(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -Lvr $(SRC_CODE_DIR)/libcedarx/* $(PKG_BUILD_DIR)/
ifeq ($(CONFIG_CEDAR_DISABLE_CEDARC), y)
	mkdir -p $(PKG_BUILD_EXDIR)/include
	$(CP) -Lvr $(SRC_CODE_DIR)/$(PKG_EXNAME)/include/* $(PKG_BUILD_EXDIR)/include
endif
	$(CP) -r ../machinfo/$(LICHEE_IC)/$(PKG_NAME)/$(LICHEE_IC)_linux_cedarx.conf $(PKG_BUILD_DIR)/
endef

define Build/Configure
	@echo "********target_board_platform = $MEDIA_TARGET_BOARD_PLATFORM***********"
	@echo "********chip type = $(TINA_CHIP_TYPE)*********"
	@echo "********kernel version = $(KERNEL_PATCHVER)*********"
	@echo "********Adapting and Checking KERNEL_VERSION = $(KERNEL_VERSION)*********"
	@echo "********Adapting and Checking KERNEL_VERSION_ION = $(KERNEL_VERSION_ION)*********"
	@echo "********toolchain = $(CONFIG_ARCH)_$(CONFIG_LIBC)*********"
	@echo "********toolchain-version = $(CONFIG_TOOLCHAIN_GCC_VERSION)*******"
	@echo "********cpu arch = $(TARGET_CPU_VARIANT)*********"
	@echo "********parser: $(CONFIG_PARSER)*********"
	@echo "********stream: $(CONFIG_STREAM)*********"
	@echo "********muxer: $(CONFIG_MUXER)*********"
	@echo "********cedarx demo: $(CONFIG_DEMO)*********"
	@echo "********debug: $(CONFIG_SAVE_YUV_DEBUG)*********"
	@echo "********enable cedarc: $(CONFIG_ENABLE_CEDRAC)*********"
	@echo "********only_enable_audio: $(CONF_ONLY_ENABLE_AUDIO)*********"
	@echo "********only_disable_audio: $(CONF_ONLY_DISABLE_AUDIO)*********"
	@echo "********Adapting and Checking cedarx_libs_path: $(CEDARX_LIBS_PATH)*********"
	@echo "********Adapting and Checking STAGING_DIR: $(STAGING_DIR)*********"
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/install --host=$(CEDARX_COMPILE_TOOL_CHAIN)  $(CONFIG_PLAYREADY) $(CONFIG_SAVE_YUV_DEBUG) $(CONFIG_ENABLE_CEDRAC) $(CONFIG_PARSER) $(CONFIG_STREAM) $(CONFIG_MUXER) $(CONFIG_DEMO) $(CONF_ONLY_DISABLE_AUDIO) CFLAGS="-D__ENABLE_ZLIB__ -Wno-error=format" CPPFLAGS="-D__ENABLE_ZLIB__ -Wno-error=format" LDFLAGS="$(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib" $(CEDARX_CONFIGURE_ARGS)
else
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/install --host=$(CEDARX_COMPILE_TOOL_CHAIN)  $(CONFIG_PLAYREADY) $(CONFIG_SAVE_YUV_DEBUG) $(CONFIG_ENABLE_CEDRAC) $(CONFIG_PARSER) $(CONFIG_STREAM) $(CONFIG_MUXER) $(CONFIG_DEMO) $(CONF_ONLY_ENABLE_AUDIO) CFLAGS="-D__ENABLE_ZLIB__ -DONLY_ENABLE_AUDIO -Wno-error=format" CPPFLAGS="-D__ENABLE_ZLIB__ -Wno-error=format" LDFLAGS="$(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib" $(CEDARX_CONFIGURE_ARGS)
endif

endef

define Build/Compile

ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
ifeq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -DONLY_DISABLE_AUDIO -D$(KERNEL_VERSION_ION) -Wno-error=format  $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS) -DMPP_ENC_API=$(NEW_ENC_API) -D$(FLAG_ENC_VERSION) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API) -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION_ION) -Wno-error=format $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib" \
		BOARD_PLATFORM="$(MEDIA_TARGET_BOARD_PLATFORM)" \
		install
else
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION_ION) -Wno-error=format  $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS) -DMPP_ENC_API=$(NEW_ENC_API) -D$(FLAG_ENC_VERSION) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API) -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner"	\
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION_ION) -Wno-error=format $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib" \
		BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
		install

endif
else
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -DONLY_ENABLE_AUDIO -D$(KERNEL_VERSION_ION) -Wno-error=format  $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION_ION) -Wno-error=format $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib" \
		BOARD_PLATFORM="$(MEDIA_TARGET_BOARD_PLATFORM)" \
		install
endif

endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
ifneq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libaw*.so $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libadecoder.so $(PKG_INSTALL_DIR)/usr/lib/
endif
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libaencoder.so $(PKG_INSTALL_DIR)/usr/lib/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libsubdecoder.so $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/liblive555.so $(PKG_INSTALL_DIR)/usr/lib/
endif

	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/

	## copy header files.
	##cdx_config.h
	-cp -r $(PKG_BUILD_DIR)/cdx_config.h										$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarx adecoder/aencoder/sdecoder
	-cp -r $(PKG_BUILD_DIR)/external/include/adecoder/adecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	-cp -r $(PKG_BUILD_DIR)/external/include/aencoder/aencoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/external/include/sdecoder/sdecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	## libcedarx:libcore
	-cp -r $(PKG_BUILD_DIR)/libcore/base/include/*.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	-cp -r $(PKG_BUILD_DIR)/libcore/muxer/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	-cp -r $(PKG_BUILD_DIR)/libcore/parser/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcore/stream/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcore/playback/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcore/common/iniparser/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## xplayer
	-cp -r $(PKG_BUILD_DIR)/xplayer/include/*.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	## awrecorder
	-cp $(PKG_BUILD_DIR)/awrecorder/awencoder.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	mkdir -p $(PKG_BUILD_DIR)/all_libs
ifneq ($(CONFIG_ONLY_DISABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libaw*.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libadecoder.so $(PKG_BUILD_DIR)/all_libs
endif
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libaencoder.so $(PKG_BUILD_DIR)/all_libs
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/libsubdecoder.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/liblive555.so $(PKG_BUILD_DIR)/all_libs
endif
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so $(PKG_BUILD_DIR)/all_libs

	echo "**install path:$(1)***"

	for n in $(XPLAYER_SUPPORT_LIBRARY_ALL); \
	do \
		$(CP) $(PKG_BUILD_DIR)/all_libs/$$$$n $(1)/usr/lib; \
	done
## deal with cedarx memory config
	$(CP) $(PKG_BUILD_DIR)/$(LICHEE_IC)_linux_cedarx.conf $(PKG_BUILD_DIR)/all_libs/cedarx.conf
ifeq ($(CONFIG_CUSTOM_FBM_NUM_ENABLE),y)
	./set_cedar_conf.sh pic_4list_num $(CONFIG_FBM_DISPLAY_HOLDING) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4di_num $(CONFIG_FBM_DEINTERLEACED_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4rotate_num $(CONFIG_FBM_ROTATE_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4smooth_num $(CONFIG_FBM_SMOOTHLY_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
endif

ifeq ($(CONFIG_CUSTOM_VBV_ENABLE),y)
	./set_cedar_conf.sh vbv_buffer_size $(CONFIG_VBVBUFSIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_VBV_ENABLE)
endif

ifeq ($(CONFIG_SCALE_DOWN_ENABLE),y)
	./set_cedar_conf.sh scaledown_large_video_flag 1 $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
	./set_cedar_conf.sh scaledown_width_limit $(CONFIG_SCALE_DOWN_WIDTH_LIMIT) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
	./set_cedar_conf.sh scaledown_ratio $(CONFIG_SCALE_DOWN_RATIO) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
endif

ifeq ($(CONFIG_CUSTOM_CACHE_ENABLE),y)
	./set_cedar_conf.sh cache_buffer_size  $(CONFIG_CACHE_BUFFER_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh cache_buffer_size_live  $(CONFIG_LIVE_CACHE_BUFFER_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh start_play_cache_time  $(CONFIG_START_PLAY_CACHE_TIME) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh max_http_stream_buf_size  $(CONFIG_HTTP_STREAM_CACHE_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
endif

ifeq ($(CONFIG_USE_ION_BUF_ENABLE),y)
	./set_cedar_conf.sh use_ion_buf_flag 1 $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_USE_ION_BUF_ENABLE)
endif

	$(CP)          $(PKG_BUILD_DIR)/all_libs/cedarx.conf						 $(1)/etc/cedarx.conf
	cd $(1)/usr/lib/; rm -rf $(NOT_SUPPORT_VIDEO_LIBS)

endef

endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
