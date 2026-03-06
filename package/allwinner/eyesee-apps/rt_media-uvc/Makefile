##############################################
# OpenWrt Makefile for rt_media-uvc program
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
PKG_NAME:=rt_media-uvc
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
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

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/eyesee-apps/rt_media-uvc


# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/rt_media-uvc
	TITLE:=rt_media-uvc for Tina Linux
	SUBMENU:=eyesee-apps
	CATEGORY:=Allwinner
	DEPENDS:= +alsa-lib +rt_media +eyesee-mpp-middleware +libexpat
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/rt_media-uvc/description
	rt_media-uvc is based on rt_media and eyesee-mpp-middleware
endef

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

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
define Package/rt_media-uvc/install
	$(INSTALL_DIR) $(1)/bin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rt_media-uvc $(1)/bin/
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,rt_media-uvc))
