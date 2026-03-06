##############################################
# OpenWrt Makefile for eyesee-mpp-dragonboard
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
PKG_NAME:=eyesee-mpp-dragonboard
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/eyesee-mpp/dragonboard
#ifeq ($(LICHEE_CHIP), sun8iw12p1)
#    SRC_CODE_DIR := $(SRC_CODE_DIR)/v5
#else ifeq ($(LICHEE_CHIP), sun8iw16p1)
#    SRC_CODE_DIR := $(SRC_CODE_DIR)/v316
#else ifeq ($(LICHEE_CHIP), sun8iw19p1)
#    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw19p1
#else ifeq ($(LICHEE_CHIP), sun8iw21p1)
#    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw21
#else ifeq ($(LICHEE_CHIP), sun300iw1p1)
#    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun300iw1
#else
#    $(error fatal error! $(LICHEE_CHIP) is not support by eyesee-mpp!)
#endif

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

export TARGET_BOARD_PLATFORM
export LICHEE_PLATFORM_OS:=tina

ifeq ($(CONFIG_mpp_support_fastboot),y)
    MPPCFG_SUPPORT_FASTBOOT := Y
else
    MPPCFG_SUPPORT_FASTBOOT := N
endif
export MPPCFG_SUPPORT_FASTBOOT

ifeq ($(CONFIG_mpp_enable_aiisp),y)
    MPPCFG_ENABLE_AIISP := Y
else
    MPPCFG_ENABLE_AIISP := N
endif
export MPPCFG_ENABLE_AIISP

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
#config if enable libcedarc_venc_support_ext_param
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
#config if include libfilerepair
ifeq ($(CONFIG_mpp_filerepair),y)
    MPPCFG_FILEREPAIR := Y
else
    MPPCFG_FILEREPAIR := N
endif
export MPPCFG_FILEREPAIR

#select mpp dragonboard sample
ifeq ($(CONFIG_mpp_fw_sample_Camera),y)
    MPPCFG_MPP_FW_SAMPLE_CAMERA := Y
else
    MPPCFG_MPP_FW_SAMPLE_CAMERA := N
endif
export MPPCFG_MPP_FW_SAMPLE_CAMERA

ifeq ($(CONFIG_mpp_fw_sample_CDREmergency),y)
    MPPCFG_MPP_FW_SAMPLE_CDREMERGENCY := Y
else
    MPPCFG_MPP_FW_SAMPLE_CDREMERGENCY := N
endif
export MPPCFG_MPP_FW_SAMPLE_CDREMERGENCY

ifeq ($(CONFIG_mpp_fw_sample_EncodeResolutionChange),y)
    MPPCFG_MPP_FW_SAMPLE_ENCODERESOLUTIONCHANGE := Y
else
    MPPCFG_MPP_FW_SAMPLE_ENCODERESOLUTIONCHANGE := N
endif
export MPPCFG_MPP_FW_SAMPLE_ENCODERESOLUTIONCHANGE

ifeq ($(CONFIG_mpp_fw_sample_RecorderCallbackOut),y)
    MPPCFG_MPP_FW_SAMPLE_RECORDERCALLBACKOUT := Y
else
    MPPCFG_MPP_FW_SAMPLE_RECORDERCALLBACKOUT := N
endif
export MPPCFG_MPP_FW_SAMPLE_RECORDERCALLBACKOUT

ifeq ($(CONFIG_mpp_fw_sample_RecorderSegment),y)
    MPPCFG_MPP_FW_SAMPLE_RECORDERSEGMENT := Y
else
    MPPCFG_MPP_FW_SAMPLE_RECORDERSEGMENT := N
endif
export MPPCFG_MPP_FW_SAMPLE_RECORDERSEGMENT

ifeq ($(CONFIG_mpp_fw_sample_RecordLowDelay),y)
    MPPCFG_MPP_FW_SAMPLE_RECORDLOWDELAY := Y
else
    MPPCFG_MPP_FW_SAMPLE_RECORDLOWDELAY := N
endif
export MPPCFG_MPP_FW_SAMPLE_RECORDLOWDELAY

ifeq ($(CONFIG_mpp_fw_sample_RecordMultiStream),y)
    MPPCFG_MPP_FW_SAMPLE_RECORDMULTISTREAM := Y
else
    MPPCFG_MPP_FW_SAMPLE_RECORDMULTISTREAM := N
endif
export MPPCFG_MPP_FW_SAMPLE_RECORDMULTISTREAM


ifeq ($(CONFIG_mpp_fw_sample_RecordSwitchFileNormal),y)
    MPPCFG_MPP_FW_SAMPLE_RECORDSWITCHFILENORMAL := Y
else
    MPPCFG_MPP_FW_SAMPLE_RECORDSWITCHFILENORMAL := N
endif
export MPPCFG_MPP_FW_SAMPLE_RECORDSWITCHFILENORMAL

ifeq ($(CONFIG_mpp_fw_sample_ThumbRetriever),y)
    MPPCFG_MPP_FW_SAMPLE_THUMBRETRIEVER := Y
else
    MPPCFG_MPP_FW_SAMPLE_THUMBRETRIEVER := N
endif
export MPPCFG_MPP_FW_SAMPLE_THUMBRETRIEVER

ifeq ($(CONFIG_mpp_fw_sample_OSD),y)
    MPPCFG_MPP_FW_SAMPLE_OSD := Y
else
    MPPCFG_MPP_FW_SAMPLE_OSD := N
endif
export MPPCFG_MPP_FW_SAMPLE_OSD

ifeq ($(CONFIG_mpp_fw_sample_Player),y)
    MPPCFG_MPP_FW_SAMPLE_PLAYER := Y
else
    MPPCFG_MPP_FW_SAMPLE_PLAYER := N
endif
export MPPCFG_MPP_FW_SAMPLE_PLAYER

ifeq ($(CONFIG_dragonboard_feature_startup),y)
    DRAGONBOARD_FEATURE_STARTUP := Y
else
    DRAGONBOARD_FEATURE_STARTUP := N
endif
export DRAGONBOARD_FEATURE_STARTUP

############################################################################
#below items are used for mpp dragonboard configuration

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=eyesee-mpp-dragonboard
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=eyesee-mpp-dragonboard for Tina Linux
  DEPENDS:= +eyesee-mpp-middleware
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-mpp-dragonboard wraps uses of eyesee-mpp-middleware, be used for test board hardware function.
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
	$(CP) $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)
	$(CP) $(TINA_TOPDIR)/platform/thirdparty/gui/lvgl-8/lvgl $(SRC_COMPILE_DIR)/apps/DragonBoard/core/gui/lvgl
	$(CP) $(TINA_TOPDIR)/platform/thirdparty/gui/lvgl-8/lv_drivers $(SRC_COMPILE_DIR)/apps/DragonBoard/core/gui/lvgl
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(SRC_COMPILE_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		LINUX_BSP_HEADERS="$(LICHEE_BSP_DIR)" \
		all
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		$(MAKE) -C $(SRC_COMPILE_DIR) \
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
	$(INSTALL_DIR) $(1)/usr/lib/eyesee-mpp
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/dragonboard
	if [ -d $(SRC_COMPILE_DIR)/include ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/include $(1)/usr/include/eyesee-mpp/dragonboard; \
	fi
	#$(CP) $(SRC_COMPILE_DIR)/apps/cdr/webserver/libwebserver.a $(1)/usr/lib/eyesee-mpp
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
	$(INSTALL_DIR) $(1)/etc/init.d
	if [ $(CONFIG_dragonboard_feature_startup) == "y" ]; then \
		$(INSTALL_BIN) $(SRC_COMPILE_DIR)/apps/DragonBoard/config/out/etc/init.d/S01app $(1)/etc/init.d/S01app; \
	fi
	$(INSTALL_DIR) $(1)/usr/share/res/jpeg
	if [ -f $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/colorbar.jpg ]; then \
		$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/colorbar.jpg $(1)/usr/share/res/jpeg; \
	fi
	$(INSTALL_DIR) $(1)/usr/bin
	if [ -f $(SRC_COMPILE_DIR)/apps/DragonBoard/core/dragonboard ]; then \
		$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/core/dragonboard $(1)/usr/bin; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/apps/DragonBoard/core/test_config.fex ]; then \
		$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/core/test_config.fex $(1)/usr/bin; \
	fi
	$(INSTALL_DIR) $(1)/usr/bin/testcase
	if [ -d $(SRC_COMPILE_DIR)/apps/DragonBoard/bin ]; then \
		$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/* $(1)/usr/bin/testcase; \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/apps/DragonBoard/bin not exist?"; \
	fi
endef

#define Package/$(PKG_NAME)/InstallDev
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
