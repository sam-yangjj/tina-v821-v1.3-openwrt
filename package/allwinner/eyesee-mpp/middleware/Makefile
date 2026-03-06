##############################################
# OpenWrt Makefile for eyesee-mpp-middleware
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
PKG_NAME:=eyesee-mpp-middleware
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/eyesee-mpp/middleware
ifeq ($(LICHEE_CHIP), sun8iw12p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/v5
else ifeq ($(LICHEE_CHIP), sun8iw16p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/v316
else ifeq ($(LICHEE_CHIP), sun8iw19p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw19p1
else ifeq ($(LICHEE_CHIP), sun8iw21p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw21
else ifeq ($(LICHEE_CHIP), sun300iw1p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun300iw1
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun252iw1
else ifeq ($(LICHEE_CHIP), sun20iw1p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun20iw1
else
    $(error fatal error! $(LICHEE_CHIP) is not support by eyesee-mpp!)
endif

CONFIG_IN_DIR := $(LICHEE_IC)
ifneq ($(filter v851% v853%, $(LICHEE_IC)),)
    CONFIG_IN_DIR := v853
else ifneq ($(filter v861%, $(LICHEE_IC)),)
    CONFIG_IN_DIR := v861
else ifneq ($(filter f133%, $(LICHEE_IC)),)
    CONFIG_IN_DIR := f133
endif

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)
PKG_BUILD_PARALLEL:=1

ifeq ($(filter $(LICHEE_CHIP),sun20iw1p1),)
PKG_BUILD_DEPENDS := libAWIspApi
endif

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
    ifeq ($(CONFIG_mpp_for_aw1931), y)
        MPPCFG_CHIP := AW1931
    else
        MPPCFG_CHIP := AW1882
    endif
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
    MPPCFG_CHIP := AW1918
else ifeq ($(LICHEE_CHIP), sun20iw1p1)
    MPPCFG_CHIP := AW1859
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

#config if enable mpp_vdecsoft
ifeq ($(CONFIG_mpp_vdecsoft),y)
    MPPCFG_VDECSOFT := Y
else
    MPPCFG_VDECSOFT := N
endif
export MPPCFG_VDECSOFT
#config if enable mpp_vdecsoft_h264
ifeq ($(CONFIG_mpp_vdecsoft_h264),y)
    MPPCFG_VDECSOFT_H264 := Y
else
    MPPCFG_VDECSOFT_H264 := N
endif
export MPPCFG_VDECSOFT_H264

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
ifeq ($(CONFIG_mpp_aenc_opus),y)
    MPPCFG_AENC_OPUS := Y
else
    MPPCFG_AENC_OPUS := N
endif
export MPPCFG_AENC_OPUS
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
ifeq ($(CONFIG_mpp_parser_hls),y)
    MPPCFG_PARSER_HLS := Y
else
    MPPCFG_PARSER_HLS := N
endif
export MPPCFG_PARSER_HLS
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

#select mpp sample
ifeq ($(CONFIG_mpp_sample),y)
    MPPCFG_SAMPLE := Y
else
    MPPCFG_SAMPLE := N
endif
export MPPCFG_SAMPLE
ifeq ($(CONFIG_mpp_sample_takePicture),y)
    MPPCFG_SAMPLE_TAKE_PICTURE := Y
else
    MPPCFG_SAMPLE_TAKE_PICTURE := N
endif
export MPPCFG_SAMPLE_TAKE_PICTURE

ifeq ($(CONFIG_mpp_sample_multi_takePicture),y)
    MPPCFG_SAMPLE_MULTI_TAKE_PICTURE := Y
else
    MPPCFG_SAMPLE_MULTI_TAKE_PICTURE := N
endif
export MPPCFG_SAMPLE_MULTI_TAKE_PICTURE


ifeq ($(CONFIG_mpp_sample_identifyQRCode),y)
    MPPCFG_SAMPLE_IDENTIFY_QRCODE := Y
else
    MPPCFG_SAMPLE_IDENTIFY_QRCODE := N
endif
export MPPCFG_SAMPLE_IDENTIFY_QRCODE
ifeq ($(CONFIG_mpp_sample_vencGdcZoom),y)
    MPPCFG_SAMPLE_VENC_GDCZOOM := Y
else
    MPPCFG_SAMPLE_VENC_GDCZOOM := N
endif
export MPPCFG_SAMPLE_VENC_GDCZOOM
ifeq ($(CONFIG_mpp_sample_vencRecreate),y)
    MPPCFG_SAMPLE_VENC_RECREATE := Y
else
    MPPCFG_SAMPLE_VENC_RECREATE := N
endif
export MPPCFG_SAMPLE_VENC_RECREATE
ifeq ($(CONFIG_mpp_sample_OnlineVenc),y)
    MPPCFG_SAMPLE_ONLINEVENC := Y
else
    MPPCFG_SAMPLE_ONLINEVENC := N
endif
export MPPCFG_SAMPLE_ONLINEVENC
ifeq ($(CONFIG_mpp_sample_vencQpMap),y)
    MPPCFG_SAMPLE_VENC_QPMAP := Y
else
    MPPCFG_SAMPLE_VENC_QPMAP := N
endif
export MPPCFG_SAMPLE_VENC_QPMAP
ifeq ($(CONFIG_mpp_sample_EncppGdcOffline),y)
    MPPCFG_SAMPLE_ENCPPGDCOFFLINE := Y
else
    MPPCFG_SAMPLE_ENCPPGDCOFFLINE := N
endif
export MPPCFG_SAMPLE_ENCPPGDCOFFLINE
ifeq ($(CONFIG_mpp_sample_virvi2vo_zoom),y)
    MPPCFG_SAMPLE_VIRVI2VO_ZOOM := Y
else
    MPPCFG_SAMPLE_VIRVI2VO_ZOOM := N
endif
export MPPCFG_SAMPLE_VIRVI2VO_ZOOM
ifeq ($(CONFIG_mpp_sample_CodecParallel),y)
    MPPCFG_SAMPLE_CODEC_PARALLEL := Y
else
    MPPCFG_SAMPLE_CODEC_PARALLEL := N
endif
export MPPCFG_SAMPLE_CODEC_PARALLEL
ifeq ($(CONFIG_mpp_sample_adec),y)
    MPPCFG_SAMPLE_ADEC := Y
else
    MPPCFG_SAMPLE_ADEC := N
endif
export MPPCFG_SAMPLE_ADEC
ifeq ($(CONFIG_mpp_sample_adec2ao),y)
    MPPCFG_SAMPLE_ADEC2AO := Y
else
    MPPCFG_SAMPLE_ADEC2AO := N
endif
export MPPCFG_SAMPLE_ADEC2AO
ifeq ($(CONFIG_mpp_sample_aec),y)
    MPPCFG_SAMPLE_AEC := Y
else
    MPPCFG_SAMPLE_AEC := N
endif
export MPPCFG_SAMPLE_AEC
ifeq ($(CONFIG_mpp_sample_aenc),y)
    MPPCFG_SAMPLE_AENC := Y
else
    MPPCFG_SAMPLE_AENC := N
endif
export MPPCFG_SAMPLE_AENC
ifeq ($(CONFIG_mpp_sample_ai),y)
    MPPCFG_SAMPLE_AI := Y
else
    MPPCFG_SAMPLE_AI := N
endif
export MPPCFG_SAMPLE_AI
ifeq ($(CONFIG_mpp_sample_ai2aenc),y)
    MPPCFG_SAMPLE_AI2AENC := Y
else
    MPPCFG_SAMPLE_AI2AENC := N
endif
export MPPCFG_SAMPLE_AI2AENC
ifeq ($(CONFIG_mpp_sample_ai2aenc2muxer),y)
    MPPCFG_SAMPLE_AI2AENC2MUXER := Y
else
    MPPCFG_SAMPLE_AI2AENC2MUXER := N
endif
export MPPCFG_SAMPLE_AI2AENC2MUXER
ifeq ($(CONFIG_mpp_sample_ao),y)
    MPPCFG_SAMPLE_AO := Y
else
    MPPCFG_SAMPLE_AO := N
endif
export MPPCFG_SAMPLE_AO
ifeq ($(CONFIG_mpp_sample_aoSync),y)
    MPPCFG_SAMPLE_AOSYNC := Y
else
    MPPCFG_SAMPLE_AOSYNC := N
endif
export MPPCFG_SAMPLE_AOSYNC
ifeq ($(CONFIG_mpp_sample_ao_resample_mixer),y)
    MPPCFG_SAMPLE_AO_RESAMPLE_MIXER := Y
else
    MPPCFG_SAMPLE_AO_RESAMPLE_MIXER := N
endif
export MPPCFG_SAMPLE_AO_RESAMPLE_MIXER
ifeq ($(CONFIG_mpp_sample_ao2ai_aec),y)
    MPPCFG_SAMPLE_AO2AI_AEC := Y
else
    MPPCFG_SAMPLE_AO2AI_AEC := N
endif
export MPPCFG_SAMPLE_AO2AI_AEC
ifeq ($(CONFIG_mpp_sample_ao2ai_aec_rate_mixer),y)
    MPPCFG_SAMPLE_AO2AI_AEC_RATE_MIXER := Y
else
    MPPCFG_SAMPLE_AO2AI_AEC_RATE_MIXER := N
endif
export MPPCFG_SAMPLE_AO2AI_AEC_RATE_MIXER
ifeq ($(CONFIG_mpp_sample_avmuxer),y)
    MPPCFG_SAMPLE_AVMUXER := Y
else
    MPPCFG_SAMPLE_AVMUXER := N
endif
export MPPCFG_SAMPLE_AVMUXER
export CONFIG_mpp_sample_avplayer
ifeq ($(CONFIG_mpp_sample_demux),y)
    MPPCFG_SAMPLE_DEMUX := Y
else
    MPPCFG_SAMPLE_DEMUX := N
endif
export MPPCFG_SAMPLE_DEMUX
ifeq ($(CONFIG_mpp_sample_demux2adec),y)
    MPPCFG_SAMPLE_DEMUX2ADEC := Y
else
    MPPCFG_SAMPLE_DEMUX2ADEC := N
endif
export MPPCFG_SAMPLE_DEMUX2ADEC
ifeq ($(CONFIG_mpp_sample_demux2adec2ao),y)
    MPPCFG_SAMPLE_DEMUX2ADEC2AO := Y
else
    MPPCFG_SAMPLE_DEMUX2ADEC2AO := N
endif
export MPPCFG_SAMPLE_DEMUX2ADEC2AO
ifeq ($(CONFIG_mpp_sample_demux2vdec),y)
    MPPCFG_SAMPLE_DEMUX2VDEC := Y
else
    MPPCFG_SAMPLE_DEMUX2VDEC := N
endif
export MPPCFG_SAMPLE_DEMUX2VDEC
ifeq ($(CONFIG_mpp_sample_demux2vdec_saveFrame),y)
    MPPCFG_SAMPLE_DEMUX2VDEC_SAVEFRAME := Y
else
    MPPCFG_SAMPLE_DEMUX2VDEC_SAVEFRAME := N
endif
export MPPCFG_SAMPLE_DEMUX2VDEC_SAVEFRAME
ifeq ($(CONFIG_mpp_sample_demux2vdecsoft_saveFrame),y)
    MPPCFG_SAMPLE_DEMUX2VDECSOFT_SAVEFRAME := Y
else
    MPPCFG_SAMPLE_DEMUX2VDECSOFT_SAVEFRAME := N
endif
export MPPCFG_SAMPLE_DEMUX2VDECSOFT_SAVEFRAME
ifeq ($(CONFIG_mpp_sample_demux2vdec2vo),y)
    MPPCFG_SAMPLE_DEMUX2VDEC2VO := Y
else
    MPPCFG_SAMPLE_DEMUX2VDEC2VO := N
endif
export MPPCFG_SAMPLE_DEMUX2VDEC2VO
ifeq ($(CONFIG_mpp_sample_demux2vdecsoft2vo),y)
    MPPCFG_SAMPLE_DEMUX2VDECSOFT2VO := Y
else
    MPPCFG_SAMPLE_DEMUX2VDECSOFT2VO := N
endif
export MPPCFG_SAMPLE_DEMUX2VDECSOFT2VO
ifeq ($(CONFIG_mpp_sample_demux2vdecDynRot2vo),y)
    MPPCFG_SAMPLE_DEMUX2VDECDYNROT2VO := Y
else
    MPPCFG_SAMPLE_DEMUX2VDECDYNROT2VO := N
endif
export MPPCFG_SAMPLE_DEMUX2VDECDYNROT2VO
ifeq ($(CONFIG_mpp_sample_directIORead),y)
    MPPCFG_SAMPLE_DIRECTIOREAD := Y
else
    MPPCFG_SAMPLE_DIRECTIOREAD := N
endif
export MPPCFG_SAMPLE_DIRECTIOREAD
ifeq ($(CONFIG_mpp_sample_driverVipp),y)
    MPPCFG_SAMPLE_DRIVERVIPP := Y
else
    MPPCFG_SAMPLE_DRIVERVIPP := N
endif
export MPPCFG_SAMPLE_DRIVERVIPP
ifeq ($(CONFIG_mpp_sample_file_repair),y)
    MPPCFG_SAMPLE_FILE_REPAIR := Y
else
    MPPCFG_SAMPLE_FILE_REPAIR := N
endif
export MPPCFG_SAMPLE_FILE_REPAIR
ifeq ($(CONFIG_mpp_sample_fish),y)
    MPPCFG_SAMPLE_FISH := Y
else
    MPPCFG_SAMPLE_FISH := N
endif
export MPPCFG_SAMPLE_FISH
ifeq ($(CONFIG_mpp_sample_g2d),y)
    MPPCFG_SAMPLE_G2D := Y
else
    MPPCFG_SAMPLE_G2D := N
endif
export MPPCFG_SAMPLE_G2D
ifeq ($(CONFIG_mpp_sample_glog),y)
    MPPCFG_SAMPLE_GLOG := Y
else
    MPPCFG_SAMPLE_GLOG := N
endif
export MPPCFG_SAMPLE_GLOG
ifeq ($(CONFIG_mpp_sample_hello),y)
    MPPCFG_SAMPLE_HELLO := Y
else
    MPPCFG_SAMPLE_HELLO := N
endif
export MPPCFG_SAMPLE_HELLO
ifeq ($(CONFIG_mpp_sample_isposd),y)
    MPPCFG_SAMPLE_ISPOSD := Y
else
    MPPCFG_SAMPLE_ISPOSD := N
endif
export MPPCFG_SAMPLE_ISPOSD
ifeq ($(CONFIG_mpp_sample_MotionDetect),y)
    MPPCFG_SAMPLE_MOTIONDETECT := Y
else
    MPPCFG_SAMPLE_MOTIONDETECT := N
endif
export MPPCFG_SAMPLE_MOTIONDETECT
ifeq ($(CONFIG_mpp_sample_motor),y)
    MPPCFG_SAMPLE_MOTOR := Y
else
    MPPCFG_SAMPLE_MOTOR := N
endif
export MPPCFG_SAMPLE_MOTOR
ifeq ($(CONFIG_mpp_sample_multi_vi2venc2muxer),y)
    MPPCFG_SAMPLE_MULTI_VI2VENC2MUXER := Y
else
    MPPCFG_SAMPLE_MULTI_VI2VENC2MUXER := N
endif
export MPPCFG_SAMPLE_MULTI_VI2VENC2MUXER
ifeq ($(CONFIG_mpp_sample_muxer_multi_stream),y)
    MPPCFG_SAMPLE_MUXER_MULTI_STREAM := Y
else
    MPPCFG_SAMPLE_MUXER_MULTI_STREAM := N
endif
export MPPCFG_SAMPLE_MUXER_MULTI_STREAM
ifeq ($(CONFIG_mpp_sample_PersonDetect),y)
    MPPCFG_SAMPLE_PERSONDETECT := Y
else
    MPPCFG_SAMPLE_PERSONDETECT := N
endif
export MPPCFG_SAMPLE_PERSONDETECT
ifeq ($(CONFIG_mpp_sample_persondetect_demo),y)
    MPPCFG_SAMPLE_PERSONDETECT_DEMO := Y
else
    MPPCFG_SAMPLE_PERSONDETECT_DEMO := N
endif
export MPPCFG_SAMPLE_PERSONDETECT_DEMO
ifeq ($(CONFIG_mpp_sample_facedetect_demo),y)
    MPPCFG_SAMPLE_FACEDETECT_DEMO := Y
else
    MPPCFG_SAMPLE_FACEDETECT_DEMO := N
endif
export MPPCFG_SAMPLE_FACEDETECT_DEMO
ifeq ($(CONFIG_mpp_sample_person_vehicle_pet_detection),y)
    MPPCFG_SAMPLE_PERSONVEHICLEPETDETECTION := Y
else
    MPPCFG_SAMPLE_PERSONVEHICLEPETDETECTION := N
endif
export MPPCFG_SAMPLE_PERSONVEHICLEPETDETECTION
ifeq ($(CONFIG_mpp_sample_pthread_cancel),y)
    MPPCFG_SAMPLE_PTHREAD_CANCEL := Y
else
    MPPCFG_SAMPLE_PTHREAD_CANCEL := N
endif
export MPPCFG_SAMPLE_PTHREAD_CANCEL
ifeq ($(CONFIG_mpp_sample_region),y)
    MPPCFG_SAMPLE_REGION := Y
else
    MPPCFG_SAMPLE_REGION := N
endif
export MPPCFG_SAMPLE_REGION

ifeq ($(CONFIG_mpp_sample_RegionDetect),y)
    MPPCFG_SAMPLE_REGION_DETECT := Y
else
    MPPCFG_SAMPLE_REGION_DETECT := N
endif
export MPPCFG_SAMPLE_REGION_DETECT

ifeq ($(CONFIG_mpp_sample_rtsp),y)
    MPPCFG_SAMPLE_RTSP := Y
else
    MPPCFG_SAMPLE_RTSP := N
endif
export MPPCFG_SAMPLE_RTSP
ifeq ($(CONFIG_mpp_sample_smartIPC_demo),y)
    MPPCFG_SAMPLE_SMARTIPC_DEMO := Y
else
    MPPCFG_SAMPLE_SMARTIPC_DEMO := N
endif
export MPPCFG_SAMPLE_SMARTIPC_DEMO
ifeq ($(CONFIG_mpp_sample_smartPreview_demo),y)
    MPPCFG_SAMPLE_SMARTPREVIEW_DEMO := Y
else
    MPPCFG_SAMPLE_SMARTPREVIEW_DEMO := N
endif
export MPPCFG_SAMPLE_SMARTPREVIEW_DEMO
ifeq ($(CONFIG_mpp_sample_select),y)
    MPPCFG_SAMPLE_SELECT := Y
else
    MPPCFG_SAMPLE_SELECT := N
endif
export MPPCFG_SAMPLE_SELECT
ifeq ($(CONFIG_mpp_sample_sound_controler),y)
    MPPCFG_SAMPLE_SOUND_CONTROLER := Y
else
    MPPCFG_SAMPLE_SOUND_CONTROLER := N
endif
export MPPCFG_SAMPLE_SOUND_CONTROLER
ifeq ($(CONFIG_mpp_sample_timelapse),y)
    MPPCFG_SAMPLE_TIMELAPSE := Y
else
    MPPCFG_SAMPLE_TIMELAPSE := N
endif
export MPPCFG_SAMPLE_TIMELAPSE
ifeq ($(CONFIG_mpp_sample_twinchn_virvi2venc2ce),y)
    MPPCFG_SAMPLE_TWINCHN_VIRVI2VENC2CE := Y
else
    MPPCFG_SAMPLE_TWINCHN_VIRVI2VENC2CE := N
endif
export MPPCFG_SAMPLE_TWINCHN_VIRVI2VENC2CE
ifeq ($(CONFIG_mpp_sample_UILayer),y)
    MPPCFG_SAMPLE_UILAYER := Y
else
    MPPCFG_SAMPLE_UILAYER := N
endif
export MPPCFG_SAMPLE_UILAYER
ifeq ($(CONFIG_mpp_sample_uvc_vo),y)
    MPPCFG_SAMPLE_UVC_VO := Y
else
    MPPCFG_SAMPLE_UVC_VO := N
endif
export MPPCFG_SAMPLE_UVC_VO
ifeq ($(CONFIG_mpp_sample_uvc2vdec_vo),y)
    MPPCFG_SAMPLE_UVC2VDEC_VO := Y
else
    MPPCFG_SAMPLE_UVC2VDEC_VO := N
endif
export MPPCFG_SAMPLE_UVC2VDEC_VO
ifeq ($(CONFIG_mpp_sample_uvc2vdenc2vo),y)
    MPPCFG_SAMPLE_UVC2VDENC2VO := Y
else
    MPPCFG_SAMPLE_UVC2VDENC2VO := N
endif
export MPPCFG_SAMPLE_UVC2VDENC2VO
ifeq ($(CONFIG_mpp_sample_uvc2vo),y)
    MPPCFG_SAMPLE_UVC2VO := Y
else
    MPPCFG_SAMPLE_UVC2VO := N
endif
export MPPCFG_SAMPLE_UVC2VO
ifeq ($(CONFIG_mpp_sample_uvc_vcodec_vo_uac),y)
    MPPCFG_SAMPLE_UVC_VCODEC_VO_UAC := Y
else
    MPPCFG_SAMPLE_UVC_VCODEC_VO_UAC := N
endif
export MPPCFG_SAMPLE_UVC_VCODEC_VO_UAC
ifeq ($(CONFIG_mpp_sample_uvc_vi_codec),y)
    MPPCFG_SAMPLE_UVC_VI_CODEC := Y
else
    MPPCFG_SAMPLE_UVC_VI_CODEC := N
endif
export MPPCFG_SAMPLE_UVC_VI_CODEC
ifeq ($(CONFIG_mpp_sample_uac),y)
    MPPCFG_SAMPLE_UAC := Y
else
    MPPCFG_SAMPLE_UAC := N
endif
export MPPCFG_SAMPLE_UAC
ifeq ($(CONFIG_mpp_sample_uvcout),y)
    MPPCFG_SAMPLE_UVCOUT := Y
else
    MPPCFG_SAMPLE_UVCOUT := N
endif
export MPPCFG_SAMPLE_UVCOUT
ifeq ($(CONFIG_mpp_sample_usbcamera),y)
    MPPCFG_SAMPLE_USBCAMERA := Y
else
    MPPCFG_SAMPLE_USBCAMERA := N
endif
export MPPCFG_SAMPLE_USBCAMERA
ifeq ($(CONFIG_mpp_sample_vdec),y)
    MPPCFG_SAMPLE_VDEC := Y
else
    MPPCFG_SAMPLE_VDEC := N
endif
export MPPCFG_SAMPLE_VDEC
ifeq ($(CONFIG_mpp_sample_vdecsoft),y)
    MPPCFG_SAMPLE_VDECSOFT := Y
else
    MPPCFG_SAMPLE_VDECSOFT := N
endif
export MPPCFG_SAMPLE_VDECSOFT
ifeq ($(CONFIG_mpp_sample_venc),y)
    MPPCFG_SAMPLE_VENC := Y
else
    MPPCFG_SAMPLE_VENC := N
endif
export MPPCFG_SAMPLE_VENC
ifeq ($(CONFIG_mpp_sample_venc2muxer),y)
    MPPCFG_SAMPLE_VENC2MUXER := Y
else
    MPPCFG_SAMPLE_VENC2MUXER := N
endif
export MPPCFG_SAMPLE_VENC2MUXER
ifeq ($(CONFIG_mpp_sample_vi_g2d),y)
    MPPCFG_SAMPLE_VI_G2D := Y
else
    MPPCFG_SAMPLE_VI_G2D := N
endif
export MPPCFG_SAMPLE_VI_G2D
ifeq ($(CONFIG_mpp_sample_vi_reset),y)
    MPPCFG_SAMPLE_VI_RESET := Y
else
    MPPCFG_SAMPLE_VI_RESET := N
endif
export MPPCFG_SAMPLE_VI_RESET
ifeq ($(CONFIG_mpp_sample_vin_isp_test),y)
    MPPCFG_SAMPLE_VIN_ISP_TEST := Y
else
    MPPCFG_SAMPLE_VIN_ISP_TEST := N
endif
export MPPCFG_SAMPLE_VIN_ISP_TEST
ifeq ($(CONFIG_mpp_sample_virvi),y)
    MPPCFG_SAMPLE_VIRVI := Y
else
    MPPCFG_SAMPLE_VIRVI := N
endif
export MPPCFG_SAMPLE_VIRVI
ifeq ($(CONFIG_mpp_sample_virvi2eis2venc),y)
    MPPCFG_SAMPLE_VIRVI2EIS2VENC := Y
else
    MPPCFG_SAMPLE_VIRVI2EIS2VENC := N
endif
export MPPCFG_SAMPLE_VIRVI2EIS2VENC
ifeq ($(CONFIG_mpp_sample_virvi2fish2venc),y)
    MPPCFG_SAMPLE_VIRVI2FISH2VENC := Y
else
    MPPCFG_SAMPLE_VIRVI2FISH2VENC := N
endif
export MPPCFG_SAMPLE_VIRVI2FISH2VENC
ifeq ($(CONFIG_mpp_sample_virvi2fish2vo),y)
    MPPCFG_SAMPLE_VIRVI2FISH2VO := Y
else
    MPPCFG_SAMPLE_VIRVI2FISH2VO := N
endif
export MPPCFG_SAMPLE_VIRVI2FISH2VO
ifeq ($(CONFIG_mpp_sample_virvi2venc),y)
    MPPCFG_SAMPLE_VIRVI2VENC := Y
else
    MPPCFG_SAMPLE_VIRVI2VENC := N
endif
export MPPCFG_SAMPLE_VIRVI2VENC
ifeq ($(CONFIG_mpp_sample_virvi2vencSync),y)
    MPPCFG_SAMPLE_VIRVI2VENCSYNC := Y
else
    MPPCFG_SAMPLE_VIRVI2VENCSYNC := N
endif
export MPPCFG_SAMPLE_VIRVI2VENCSYNC
ifeq ($(CONFIG_mpp_sample_virvi2venc2ce),y)
    MPPCFG_SAMPLE_VIRVI2VENC2CE := Y
else
    MPPCFG_SAMPLE_VIRVI2VENC2CE := N
endif
export MPPCFG_SAMPLE_VIRVI2VENC2CE
ifeq ($(CONFIG_mpp_sample_virvi2venc2muxer),y)
    MPPCFG_SAMPLE_VIRVI2VENC2MUXER := Y
else
    MPPCFG_SAMPLE_VIRVI2VENC2MUXER := N
endif
export MPPCFG_SAMPLE_VIRVI2VENC2MUXER
ifeq ($(CONFIG_mpp_sample_virvi2vo),y)
    MPPCFG_SAMPLE_VIRVI2VO := Y
else
    MPPCFG_SAMPLE_VIRVI2VO := N
endif
export MPPCFG_SAMPLE_VIRVI2VO
ifeq ($(CONFIG_mpp_sample_vo),y)
    MPPCFG_SAMPLE_VO := Y
else
    MPPCFG_SAMPLE_VO := N
endif
export MPPCFG_SAMPLE_VO

ifeq ($(CONFIG_mpp_sample_pdet),y)
    MPPCFG_SAMPLE_PDET := Y
else
    MPPCFG_SAMPLE_PDET := N
endif
export MPPCFG_SAMPLE_PDET

ifeq ($(CONFIG_mpp_sample_odet_demo),y)
    MPPCFG_SAMPLE_ODET_DEMO := Y
else
    MPPCFG_SAMPLE_ODET_DEMO := N
endif
export MPPCFG_SAMPLE_ODET_DEMO

ifeq ($(CONFIG_mpp_sample_facekit_demo),y)
    MPPCFG_SAMPLE_FACEKIT_DEMO := Y
else
    MPPCFG_SAMPLE_FACEKIT_DEMO := N
endif
export MPPCFG_SAMPLE_FACEKIT_DEMO

ifeq ($(CONFIG_mpp_sample_recorder),y)
    MPPCFG_SAMPLE_RECORDER := Y
else
    MPPCFG_SAMPLE_RECORDER := N
endif
export MPPCFG_SAMPLE_RECORDER

ifeq ($(CONFIG_mpp_sample_CDR_demo),y)
    MPPCFG_SAMPLE_CDR_DEMO := Y
else
    MPPCFG_SAMPLE_CDR_DEMO := N
endif
export MPPCFG_SAMPLE_CDR_DEMO

ifeq ($(CONFIG_mpp_sample_multi_CDR_demo),y)
    MPPCFG_SAMPLE_MULTI_CDR_DEMO := Y
else
    MPPCFG_SAMPLE_MULTI_CDR_DEMO := N
endif
export MPPCFG_SAMPLE_MULTI_CDR_DEMO

ifeq ($(CONFIG_mpp_sample_ai2ao),y)
    MPPCFG_SAMPLE_AI2AO := Y
else
    MPPCFG_SAMPLE_AI2AO := N
endif
export MPPCFG_SAMPLE_AI2AO

ifeq ($(CONFIG_mpp_sample_WebRtcAec),y)
    MPPCFG_SAMPLE_WEBRTCAEC := Y
else
    MPPCFG_SAMPLE_WEBRTCAEC := N
endif
export MPPCFG_SAMPLE_WEBRTCAEC

ifeq ($(CONFIG_mpp_sample_UvoiceAec),y)
    MPPCFG_SAMPLE_UVOICEAEC := Y
else
    MPPCFG_SAMPLE_UVOICEAEC := N
endif
export MPPCFG_SAMPLE_UVOICEAEC

ifeq ($(CONFIG_mpp_sample_VideoResizer),y)
    MPPCFG_SAMPLE_VIDEORESIZER := Y
else
    MPPCFG_SAMPLE_VIDEORESIZER := N
endif
export MPPCFG_SAMPLE_VIDEORESIZER

ifeq ($(CONFIG_mpp_software_tdm_for_isp),y)
    MPPCFG_SOFTWARE_TDM_FOR_ISP := Y
else
    MPPCFG_SOFTWARE_TDM_FOR_ISP := N
endif
export MPPCFG_SOFTWARE_TDM_FOR_ISP

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below

# sun20iw1p1 don't have isp, we don't needs libAWIspApi depends
define Package/$(PKG_NAME)
  SECTION:=eyesee-mpp-middleware
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=eyesee-mpp-middleware for Tina Linux
  DEPENDS:= @PACKAGE_eyesee-mpp-middleware_config +eyesee-mpp-system +libcedarc +libawion \
    +(AUDIO_SUPPORT&&mpp_aio_alsa):alsa-lib +mpp_aio_audiosystem:AudioSystem +mpp_demuxer:zlib +mpp_systrace:awsystrace \
    +mpp_parser_hls:libopenssl
ifeq ($(filter $(LICHEE_CHIP),sun20iw1p1),)
  DEPENDS += +mpp_vi:libAWIspApi
endif
ifneq ($(filter $(LICHEE_CHIP),sun252iw1p1),)
  DEPENDS += +mpp_vo:libuapi
endif
endef

define Package/$(PKG_NAME)-demo
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=samples for eyesee-mpp-middleware
  DEPENDS:= @PACKAGE_eyesee-mpp-middleware-demo_config $(PKG_NAME) \
	  +(mpp_sample_rtsp||mpp_sample_smartIPC_demo||mpp_sample_person_vehicle_pet_detection):eyesee-mpp-private-rtsp \
	  +(mpp_sample_identifyQRCode):libzbar \
	  +(mpp_sample_virvi2venc2ce||mpp_sample_twinchn_virvi2venc2ce):libopenssl
ifeq ($(LICHEE_CHIP), sun300iw1p1)
  DEPENDS += +(mpp_sample_pdet||mpp_sample_smartIPC_demo):person_detection \
	     +(mpp_sample_smartIPC_demo):motion_detection
else ifeq ($(LICHEE_CHIP), sun252iw1p1)
  DEPENDS += +(mpp_sample_smartIPC_demo||mpp_sample_person_vehicle_pet_detection):motion_detection \
	     +(mpp_sample_smartIPC_demo):person_detection_rlion \
	     +(mpp_sample_rtsp||mpp_sample_smartIPC_demo||mpp_sample_person_vehicle_pet_detection):awnn_runtime \
	     +(mpp_sample_rtsp||mpp_sample_smartIPC_demo||mpp_sample_person_vehicle_pet_detection):libawaiisp \
	     +(mpp_sample_persondetect_demo||mpp_sample_smartIPC_demo):person_detection_rlion \
	     +(mpp_sample_person_vehicle_pet_detection):person_vehicle_pet_detection_rlion \
	     +(mpp_sample_facedetect_demo):face_detection
endif
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/machinfo/$(CONFIG_IN_DIR)/Config.in"
endef

define Package/$(PKG_NAME)-demo/config
source "$(SOURCE)/machinfo/$(CONFIG_IN_DIR)/Config-demo.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-mpp-middleware contains eyesee-mpp basic libraries.
endef

define Package/$(PKG_NAME)-demo/description
  eyesee-mpp-middleware-demo contains all samples designed to test mpp-middleware and to show how to use mpp-middleware.
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
	# copy custom mpp_config.mk to source code directory.
	#if [ -f $(TOPDIR)/target/allwinner/$(BOARD)/custom-config/mpp_config.mk ]; then \
	#	echo "copy happen!"; \
	#	$(CP) $(TOPDIR)/target/allwinner/$(BOARD)/custom-config/mpp_config.mk $(SRC_CODE_DIR)/config/mpp_config.mk; \
	#fi
	ln -s $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)
endef

define Build/Configure
endef

define Build/Compile
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
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		MPPCFG_COMPILE_DYNAMIC_LIB=Y \
		MPPCFG_COMPILE_STATIC_LIB=Y \
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

define Build/InstallDev
	if [ -d $(SRC_COMPILE_DIR)/InstallDev ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/InstallDev/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/InstallDev not exist?"; \
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
	if [ -d $(SRC_COMPILE_DIR)/install ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/install/* $(1); \
	else \
		echo "fatal error! why $(SRC_COMPILE_DIR)/install not exist?"; \
	fi
endef

define Package/$(PKG_NAME)-demo/install
	@echo "package $(PKG_NAME)-demo has no libs or exeFiles to install.";
endef

#define Package/$(PKG_NAME)/InstallDev
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
