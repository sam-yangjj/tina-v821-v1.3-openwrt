##############################################
# OpenWrt Makefile for rt_media-mpp_demo
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
PKG_NAME:=ai-glass
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD2
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
    RTC_INIT_PATCH := files/sun300iw1/rtc_init.sh
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
    MPPCFG_CHIP := AW1918
    RTC_INIT_PATCH := files/sun252iw1/rtc_init.sh
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
#config if use hw display method
ifeq ($(CONFIG_mpp_hw_display),y)
    MPPCFG_HW_DISPLAY := Y
else
    MPPCFG_HW_DISPLAY := N
endif
export MPPCFG_HW_DISPLAY
#config if enable mpp_vi
ifeq ($(CONFIG_mpp_vi),y)
    MPPCFG_VI := Y
else
    MPPCFG_VI := N
endif
export MPPCFG_VI
#config if enable mpp_isp_awTuningApp_use_libcedarc
ifeq ($(CONFIG_mpp_isp_awTuningApp_use_libcedarc),y)
    MPPCFG_ISP_AWTUNINGAPP_USE_LIBCEDARC := Y
else
    MPPCFG_ISP_AWTUNINGAPP_USE_LIBCEDARC := N
endif
export MPPCFG_ISP_AWTUNINGAPP_USE_LIBCEDARC
#config if enable mpp_vo
ifeq ($(CONFIG_mpp_vo),y)
    MPPCFG_VO := Y
else
    MPPCFG_VO := N
endif
export MPPCFG_VO
#config if enable mpp_textEnc
ifeq ($(CONFIG_mpp_textenc),y)
    MPPCFG_TEXTENC := Y
else
    MPPCFG_TEXTENC := N
endif
export MPPCFG_TEXTENC
#config if enable mpp_venc
ifeq ($(CONFIG_mpp_venc),y)
    MPPCFG_VENC := Y
else
    MPPCFG_VENC := N
endif
export MPPCFG_VENC
#config if enable mpp_venc_h264
ifeq ($(CONFIG_mpp_venc_h264),y)
    MPPCFG_VENC_H264 := Y
else
    MPPCFG_VENC_H264 := N
endif
export MPPCFG_VENC_H264
#config if enable mpp_venc_h265
ifeq ($(CONFIG_mpp_venc_h265),y)
    MPPCFG_VENC_H265 := Y
else
    MPPCFG_VENC_H265 := N
endif
export MPPCFG_VENC_H265
#config if enable mpp_venc_jpeg
ifeq ($(CONFIG_mpp_venc_jpeg),y)
    MPPCFG_VENC_JPEG := Y
else
    MPPCFG_VENC_JPEG := N
endif
export MPPCFG_VENC_JPEG
#config if enable mpp_venc_support_venc_param_debug
ifeq ($(CONFIG_libcedarc_venc_support_ext_param),y)
    MPP_VENC_SUPPORT_VENC_PARAM_DEBUG := Y
else
    MPP_VENC_SUPPORT_VENC_PARAM_DEBUG := N
endif
export MPP_VENC_SUPPORT_VENC_PARAM_DEBUG
#config if enable mpp_vdec
ifeq ($(CONFIG_mpp_vdec),y)
    MPPCFG_VDEC := Y
else
    MPPCFG_VDEC := N
endif
export MPPCFG_VDEC
#config if enable mpp_vdec_h264
ifeq ($(CONFIG_mpp_vdec_h264),y)
    MPPCFG_VDEC_H264 := Y
else
    MPPCFG_VDEC_H264 := N
endif
export MPPCFG_VDEC_H264
#config if enable mpp_vdec_h265
ifeq ($(CONFIG_mpp_vdec_h265),y)
    MPPCFG_VDEC_H265 := Y
else
    MPPCFG_VDEC_H265 := N
endif
export MPPCFG_VDEC_H265
#config if enable mpp_vdec_jpeg
ifeq ($(CONFIG_mpp_vdec_jpeg),y)
    MPPCFG_VDEC_JPEG := Y
else
    MPPCFG_VDEC_JPEG := N
endif
export MPPCFG_VDEC_JPEG
#config if enable mpp_aio
ifeq ($(CONFIG_mpp_aio),y)
    MPPCFG_AIO := Y
else
    MPPCFG_AIO := N
endif
export MPPCFG_AIO

#config aio library of alsa if enable mpp_aio_alsa
ifeq ($(CONFIG_mpp_aio_alsa),y)
    MPPCFG_AIO_LIB := alsa
endif
ifeq ($(CONFIG_mpp_aio_audiosystem),y)
    MPPCFG_AIO_LIB := audiosystem
endif
export MPPCFG_AIO_LIB

#config if enable mpp_aenc
ifeq ($(CONFIG_mpp_aenc),y)
    MPPCFG_AENC := Y
else
    MPPCFG_AENC := N
endif
export MPPCFG_AENC
ifeq ($(CONFIG_mpp_aenc_pcm),y)
    MPPCFG_AENC_PCM := Y
else
    MPPCFG_AENC_PCM := N
endif
export MPPCFG_AENC_PCM
ifeq ($(CONFIG_mpp_aenc_g711),y)
    MPPCFG_AENC_G711 := Y
else
    MPPCFG_AENC_G711 := N
endif
export MPPCFG_AENC_G711
ifeq ($(CONFIG_mpp_aenc_g726),y)
    MPPCFG_AENC_G726 := Y
else
    MPPCFG_AENC_G726 := N
endif
export MPPCFG_AENC_G726
ifeq ($(CONFIG_mpp_aenc_aac),y)
    MPPCFG_AENC_AAC := Y
else
    MPPCFG_AENC_AAC := N
endif
export MPPCFG_AENC_AAC
ifeq ($(CONFIG_mpp_aenc_mp3),y)
    MPPCFG_AENC_MP3 := Y
else
    MPPCFG_AENC_MP3 := N
endif
export MPPCFG_AENC_MP3
#config if enable mpp_adec
ifeq ($(CONFIG_mpp_adec),y)
    MPPCFG_ADEC := Y
else
    MPPCFG_ADEC := N
endif
export MPPCFG_ADEC
ifeq ($(CONFIG_mpp_adec_wav),y)
    MPPCFG_ADEC_WAV := Y
else
    MPPCFG_ADEC_WAV := N
endif
export MPPCFG_ADEC_WAV
ifeq ($(CONFIG_mpp_adec_g726),y)
    MPPCFG_ADEC_G726 := Y
else
    MPPCFG_ADEC_G726 := N
endif
export MPPCFG_ADEC_G726
ifeq ($(CONFIG_mpp_adec_aac),y)
    MPPCFG_ADEC_AAC := Y
else
    MPPCFG_ADEC_AAC := N
endif
export MPPCFG_ADEC_AAC
ifeq ($(CONFIG_mpp_adec_mp3),y)
    MPPCFG_ADEC_MP3 := Y
else
    MPPCFG_ADEC_MP3 := N
endif
export MPPCFG_ADEC_MP3
ifeq ($(CONFIG_mpp_adec_amr),y)
    MPPCFG_ADEC_AMR := Y
else
    MPPCFG_ADEC_AMR := N
endif
export MPPCFG_ADEC_AMR
ifeq ($(CONFIG_mpp_adec_ape),y)
    MPPCFG_ADEC_APE := Y
else
    MPPCFG_ADEC_APE := N
endif
export MPPCFG_ADEC_APE
ifeq ($(CONFIG_mpp_adec_flac),y)
    MPPCFG_ADEC_FLAC := Y
else
    MPPCFG_ADEC_FLAC := N
endif
export MPPCFG_ADEC_FLAC
ifeq ($(CONFIG_mpp_adec_ogg),y)
    MPPCFG_ADEC_OGG := Y
else
    MPPCFG_ADEC_OGG := N
endif
export MPPCFG_ADEC_OGG
ifeq ($(CONFIG_mpp_adec_opus),y)
    MPPCFG_ADEC_OPUS := Y
else
    MPPCFG_ADEC_OPUS := N
endif
export MPPCFG_ADEC_OPUS
#config if enable mpp_muxer
ifeq ($(CONFIG_mpp_muxer),y)
    MPPCFG_MUXER := Y
else
    MPPCFG_MUXER := N
endif
export MPPCFG_MUXER
#config gps data pack method
ifeq ($(CONFIG_mpp_gps_pack_in_track),y)
    MPPCFG_GPS_PACK_IN_TRACK := Y
else
    MPPCFG_GPS_PACK_IN_TRACK := N
endif
export MPPCFG_GPS_PACK_IN_TRACK
ifeq ($(CONFIG_mpp_gps_pack_in_mdat),y)
    MPPCFG_GPS_PACK_IN_MDAT := Y
else
    MPPCFG_GPS_PACK_IN_MDAT := N
endif
export MPPCFG_GPS_PACK_IN_MDAT
#config if enable mpp_demuxer
ifeq ($(CONFIG_mpp_demuxer),y)
    MPPCFG_DEMUXER := Y
else
    MPPCFG_DEMUXER := N
endif
export MPPCFG_DEMUXER
ifeq ($(CONFIG_mpp_parser_aac),y)
    MPPCFG_PARSER_AAC := Y
else
    MPPCFG_PARSER_AAC := N
endif
export MPPCFG_PARSER_AAC
ifeq ($(CONFIG_mpp_parser_amr),y)
    MPPCFG_PARSER_AMR := Y
else
    MPPCFG_PARSER_AMR := N
endif
export MPPCFG_PARSER_AMR
ifeq ($(CONFIG_mpp_parser_ape),y)
    MPPCFG_PARSER_APE := Y
else
    MPPCFG_PARSER_APE := N
endif
export MPPCFG_PARSER_APE
ifeq ($(CONFIG_mpp_parser_flac),y)
    MPPCFG_PARSER_FLAC := Y
else
    MPPCFG_PARSER_FLAC := N
endif
export MPPCFG_PARSER_FLAC
ifeq ($(CONFIG_mpp_parser_id3v2),y)
    MPPCFG_PARSER_ID3V2 := Y
else
    MPPCFG_PARSER_ID3V2 := N
endif
export MPPCFG_PARSER_ID3V2
ifeq ($(CONFIG_mpp_parser_mov),y)
    MPPCFG_PARSER_MOV := Y
else
    MPPCFG_PARSER_MOV := N
endif
export MPPCFG_PARSER_MOV
ifeq ($(CONFIG_mpp_parser_mp3),y)
    MPPCFG_PARSER_MP3 := Y
else
    MPPCFG_PARSER_MP3 := N
endif
export MPPCFG_PARSER_MP3
ifeq ($(CONFIG_mpp_parser_mpg),y)
    MPPCFG_PARSER_MPG := Y
else
    MPPCFG_PARSER_MPG := N
endif
export MPPCFG_PARSER_MPG
ifeq ($(CONFIG_mpp_parser_ogg),y)
    MPPCFG_PARSER_OGG := Y
else
    MPPCFG_PARSER_OGG := N
endif
export MPPCFG_PARSER_OGG
ifeq ($(CONFIG_mpp_parser_ts),y)
    MPPCFG_PARSER_TS := Y
else
    MPPCFG_PARSER_TS := N
endif
export MPPCFG_PARSER_TS
ifeq ($(CONFIG_mpp_parser_wav),y)
    MPPCFG_PARSER_WAV := Y
else
    MPPCFG_PARSER_WAV := N
endif
export MPPCFG_PARSER_WAV
ifeq ($(CONFIG_mpp_stream_http),y)
    MPPCFG_STREAM_HTTP := Y
else
    MPPCFG_STREAM_HTTP := N
endif
export MPPCFG_STREAM_HTTP
ifeq ($(CONFIG_mpp_stream_tcp),y)
    MPPCFG_STREAM_TCP := Y
else
    MPPCFG_STREAM_TCP := N
endif
export MPPCFG_STREAM_TCP
#config if include libmpp_ise.so and enable ise function.
ifeq ($(CONFIG_mpp_ise),y)
    MPPCFG_ISE := Y
else
    MPPCFG_ISE := N
endif
export MPPCFG_ISE
#config if include libmpp_ise.so and enable ise mo module.
ifeq ($(CONFIG_mpp_ise_mo),y)
    MPPCFG_ISE_MO := Y
else
    MPPCFG_ISE_MO := N
endif
export MPPCFG_ISE_MO
#config if include libmpp_ise.so and enable ise gdc module.
ifeq ($(CONFIG_mpp_ise_gdc),y)
    MPPCFG_ISE_GDC := Y
else
    MPPCFG_ISE_GDC := N
endif
export MPPCFG_ISE_GDC
#config if include libmpp_ise.so and enable ise bi module.
ifeq ($(CONFIG_mpp_ise_bi),y)
    MPPCFG_ISE_BI := Y
else
    MPPCFG_ISE_BI := N
endif
export MPPCFG_ISE_BI
#config if include libmpp_ise.so and enable ise bi soft module.
ifeq ($(CONFIG_mpp_ise_bi_soft),y)
    MPPCFG_ISE_BI_SOFT := Y
else
    MPPCFG_ISE_BI_SOFT := N
endif
export MPPCFG_ISE_BI_SOFT
#config if include libmpp_ise.so and enable ise sti module.
ifeq ($(CONFIG_mpp_ise_two_ise),y)
    MPPCFG_ISE_STI := Y
else
    MPPCFG_ISE_STI := N
endif
export MPPCFG_ISE_STI
#config if include libmpp_eis.so and enable eis function.(Electronic Image Stabilization)
ifeq ($(CONFIG_mpp_eis),y)
    MPPCFG_EIS := Y
else
    MPPCFG_EIS := N
endif
export MPPCFG_EIS
#config if enable mpp_aec
ifeq ($(CONFIG_mpp_aec),y)
    MPPCFG_AEC := Y
else
    MPPCFG_AEC := N
endif
export MPPCFG_AEC

#config aec library of webrtc if enable mpp_aec_libwebrtc
ifeq ($(CONFIG_mpp_aec_libwebrtc),y)
    MPPCFG_AEC_LIB := webrtc
endif
ifeq ($(CONFIG_mpp_aec_libuvoice),y)
    MPPCFG_AEC_LIB := uvoice
endif
export MPPCFG_AEC_LIB

#config if enable mpp_softdrc
ifeq ($(CONFIG_mpp_softdrc),y)
    MPPCFG_SOFTDRC := Y
else
    MPPCFG_SOFTDRC := N
endif
export MPPCFG_SOFTDRC
#config if enable mpp_agc
ifeq ($(CONFIG_mpp_agc),y)
    MPPCFG_AGC := Y
else
    MPPCFG_AGC := N
endif
export MPPCFG_AGC
#config agc library of webrtc if enable mpp_agc_libwebrtc
ifeq ($(CONFIG_mpp_agc_libwebrtc),y)
    MPPCFG_AGC_LIB := webrtc
else ifeq ($(CONFIG_mpp_agc_libagcfloat),y)
    MPPCFG_AGC_LIB := agcfloat
endif
export MPPCFG_AGC_LIB
#ifeq ($(CONFIG_mpp_ai_agc),y)
#    MPPCFG_AI_AGC := Y
#else
#    MPPCFG_AI_AGC := N
#endif
#export MPPCFG_AI_AGC

#config if enable mpp_ans
ifeq ($(CONFIG_mpp_ans),y)
    MPPCFG_ANS := Y
else
    MPPCFG_ANS := N
endif
export MPPCFG_ANS

#config ans library of webrtc, lstm, nosc.
ifeq ($(CONFIG_mpp_ans_libwebrtc),y)
    MPPCFG_ANS_LIB := libwebrtc
endif
ifeq ($(CONFIG_mpp_ans_liblstm),y)
    MPPCFG_ANS_LIB := liblstm
endif
ifeq ($(CONFIG_mpp_ans_libnosc),y)
    MPPCFG_ANS_LIB := libnosc
endif
export MPPCFG_ANS_LIB

#config if include libmpp_uvc.so
ifeq ($(CONFIG_mpp_uvc),y)
    MPPCFG_UVC := Y
else
    MPPCFG_UVC := N
endif
export MPPCFG_UVC

#config if enable iommu
ifeq ($(CONFIG_mpp_iommu),y)
    MPPCFG_USE_IOMMU := Y
else
    MPPCFG_USE_IOMMU := N
endif
export MPPCFG_USE_IOMMU
#prebuild all AI libs and component by static
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
ifeq ($(CONFIG_mpp_systrace),y)
    MPPCFG_SYSTRACE := Y
else
    MPPCFG_SYSTRACE := N
endif
export MPPCFG_SYSTRACE
ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
    MPPCFG_TOOLCHAIN_LIBC := musl
else ifeq ($(CONFIG_TOOLCHAIN_LIBC), "glibc")
    MPPCFG_TOOLCHAIN_LIBC := glibc
endif
export MPPCFG_TOOLCHAIN_LIBC
#config if include libfilerepair
ifeq ($(CONFIG_mpp_filerepair),y)
    MPPCFG_FILEREPAIR := Y
else
    MPPCFG_FILEREPAIR := N
endif
export MPPCFG_FILEREPAIR
#config if include libsample_confparser
ifeq ($(CONFIG_mpp_sample_configfileparser),y)
    MPPCFG_SAMPLE_CONFIGFILEPARSER := Y
else
    MPPCFG_SAMPLE_CONFIGFILEPARSER := N
endif
export MPPCFG_SAMPLE_CONFIGFILEPARSER

#-include $(INCLUDE_DIR)/version.mk
SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/eyesee-apps/demo_ai_glass
TARGET_CFLAGS += -DTINA_5_0

define Package/$(PKG_NAME)/Default
  SUBMENU:=eyesee-apps
  CATEGORY:=Allwinner
  TITLE:=ai glass for tina linux
endef

define Package/$(PKG_NAME)
  MENU:=1
  DEFAULT:=
  $(call Package/$(PKG_NAME)/Default)
endef

define Package/$(PKG_NAME)/description
	ai_glass is based on rt_media, mpp, wifimanager.
endef

#define Package/$(PKG_NAME)/config
#source "$(SOURCE)/Config.in"
#endef

define Package/$(PKG_NAME)-lib
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass lib
  DEPENDS:= ai-glass +libstdcpp
endef

define Package/$(PKG_NAME)-lib/description
	ai_glass lib
endef

define Package/$(PKG_NAME)-photo
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass take photo app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +rt_media +eyesee-mpp-middleware +wirelesscommon
endef

define Package/$(PKG_NAME)-photo/description
	ai glass take photo app
endef

define Package/$(PKG_NAME)-video
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass take video app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +rt_media +eyesee-mpp-middleware +wirelesscommon
endef

define Package/$(PKG_NAME)-video/description
	ai glass take video app
endef

define Package/$(PKG_NAME)-video-preview
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass take video preview app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +rt_media +eyesee-mpp-middleware +wirelesscommon
endef

define Package/$(PKG_NAME)-video-preview/description
	ai glass take video preview app
endef

define Package/$(PKG_NAME)-aov
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass aov app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +rt_media +eyesee-mpp-middleware +wirelesscommon +@mpp_system_rgb_ctrl
endef

define Package/$(PKG_NAME)-aov/description
	ai glass aov app
endef

define Package/$(PKG_NAME)-download
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass media download app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +wirelesscommon +wifimanager-v2.0-lib +cJSON
endef

define Package/$(PKG_NAME)-download/description
	ai glass media download app
endef

define Package/$(PKG_NAME)-ota
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass system upgrade app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +wirelesscommon +wifimanager-v2.0-lib
endef

define Package/$(PKG_NAME)-ota/description
	ai glass system upgrade app
endef

define Package/$(PKG_NAME)-livestream
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass media livestream app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +wirelesscommon +wifimanager-v2.0-lib +libstdcpp +libawion +rt_media +eyesee-mpp-middleware +libexpat +eyesee-mpp-private-rtsp +rt_media-demo-common
endef

define Package/$(PKG_NAME)-livestream/description
	ai glass media livestream app
endef

define Package/$(PKG_NAME)-normal
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass system upgrade app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +wirelesscommon
endef

define Package/$(PKG_NAME)-normal/description
	ai glass normal mode app
endef

define Package/$(PKG_NAME)-audio
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=ai glass take audio app
  DEPENDS:= ai-glass +$(PKG_NAME)-lib +wirelesscommon
endef

define Package/$(PKG_NAME)-audio/description
	ai glass take audio app
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_demo_ai_glass_autostart

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/app $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/files $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

TARGET_CFLAGS += -DOS_NET_LINUX_OS

ifeq ($(CONFIG_PACKAGE_ai-glass-lib),y)
define Build/Compile/$(PKG_NAME)-lib
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-photo),y)
define Build/Compile/$(PKG_NAME)-photo
	$(MAKE) -C $(PKG_BUILD_DIR)/app/photo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-video),y)
define Build/Compile/$(PKG_NAME)-video
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/video \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-video-preview),y)
define Build/Compile/$(PKG_NAME)-video-preview
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/video_preview \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-aov),y)
define Build/Compile/$(PKG_NAME)-aov
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/aov \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-download),y)
define Build/Compile/$(PKG_NAME)-download
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/download \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-ota),y)
define Build/Compile/$(PKG_NAME)-ota
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/ota \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-livestream),y)
define Build/Compile/$(PKG_NAME)-livestream
	#libaglink.so
	$(MAKE) -C $(PKG_BUILD_DIR)/app/livestream \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-normal),y)
define Build/Compile/$(PKG_NAME)-normal
	$(MAKE) -C $(PKG_BUILD_DIR)/app/normal/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_ai-glass-audio),y)
define Build/Compile/$(PKG_NAME)-audio
	$(MAKE) -C $(PKG_BUILD_DIR)/app/audio/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef
endif
define Build/Compile
	$(Build/Compile/$(PKG_NAME)-lib)
	$(Build/Compile/$(PKG_NAME)-photo)
	$(Build/Compile/$(PKG_NAME)-video)
	$(Build/Compile/$(PKG_NAME)-video-preview)
	$(Build/Compile/$(PKG_NAME)-aov)
	$(Build/Compile/$(PKG_NAME)-download)
	$(Build/Compile/$(PKG_NAME)-ota)
	$(Build/Compile/$(PKG_NAME)-livestream)
	$(Build/Compile/$(PKG_NAME)-normal)
	$(Build/Compile/$(PKG_NAME)-audio)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include/aglink
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libaglink.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/aglink/*.h $(1)/usr/include/aglink
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/media
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/log_handle.sh $(1)/etc/media
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(RTC_INIT_PATCH) $(1)/etc/media
endef

define Package/$(PKG_NAME)-lib/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaglink.so $(1)/lib
	if [ -d $(STAGING_DIR)/usr/lib ]; then \
		find $(STAGING_DIR)/usr/lib -name "*crypto*.so*" -exec $(CP) {} $(1)/lib/ \;; \
	fi
endef

define Package/$(PKG_NAME)-photo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/photo/ai_glass_photo $(1)/bin
endef

define Package/$(PKG_NAME)-video/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/video/ai_glass_video $(1)/bin
endef

define Package/$(PKG_NAME)-video-preview/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/video_preview/ai_glass_video_preview $(1)/bin
endef

define Package/$(PKG_NAME)-aov/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/aov/ai_glass_aov $(1)/bin
endef

define Package/$(PKG_NAME)-download/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/download/ai_glass_download $(1)/bin
endef

define Package/$(PKG_NAME)-ota/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/ota/ai_glass_ota $(1)/bin
endef

define Package/$(PKG_NAME)-livestream/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/livestream/ai_glass_livestream $(1)/bin
endef

define Package/$(PKG_NAME)-normal/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/normal/ai_glass_normal $(1)/bin
endef

define Package/$(PKG_NAME)-audio/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/audio/ai_glass_audio $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-photo))
$(eval $(call BuildPackage,$(PKG_NAME)-video))
$(eval $(call BuildPackage,$(PKG_NAME)-video-preview))
$(eval $(call BuildPackage,$(PKG_NAME)-aov))
$(eval $(call BuildPackage,$(PKG_NAME)-ota))
$(eval $(call BuildPackage,$(PKG_NAME)-download))
$(eval $(call BuildPackage,$(PKG_NAME)-livestream))
$(eval $(call BuildPackage,$(PKG_NAME)-normal))
$(eval $(call BuildPackage,$(PKG_NAME)-audio))
