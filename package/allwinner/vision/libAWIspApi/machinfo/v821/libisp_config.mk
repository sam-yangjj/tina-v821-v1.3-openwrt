SENSOR_NAME:=
ifeq ($(CONFIG_imx386),y)
    SENSOR_NAME+=imx386
endif
ifeq ($(CONFIG_gc4663),y)
    SENSOR_NAME+=gc4663
endif
ifeq ($(CONFIG_gc1084),y)
    SENSOR_NAME+=gc1084
    ifeq ($(CONFIG_gc1084_8bit),y)
        SENSOR_NAME+=gc1084_8bit
    endif
endif
ifeq ($(CONFIG_gc2053),y)
    SENSOR_NAME+=gc2053
    ifeq ($(CONFIG_gc2053_8bit),y)
        SENSOR_NAME+=gc2053_8bit
    endif
endif
ifeq ($(CONFIG_gc0406),y)
    SENSOR_NAME+=gc0406
endif
ifeq ($(CONFIG_gc2083),y)
    SENSOR_NAME+=gc2083
endif
ifeq ($(CONFIG_f37p),y)
    SENSOR_NAME+=f37p
endif
ifeq ($(CONFIG_f355p),y)
    SENSOR_NAME+=f355p
endif
ifeq ($(CONFIG_mis2008),y)
    SENSOR_NAME+=mis2008
endif
ifeq ($(CONFIG_sc1346),y)
    SENSOR_NAME+=sc1346
endif
ifeq ($(CONFIG_sc2336),y)
    SENSOR_NAME+=sc2336
endif
ifeq ($(CONFIG_sc2355),y)
    SENSOR_NAME+=sc2355
endif
ifeq ($(CONFIG_sp1405),y)
    SENSOR_NAME+=sp1405
endif
ifeq ($(CONFIG_sc3336),y)
    SENSOR_NAME+=sc3336
endif
ifeq ($(CONFIG_gc4663),y)
    SENSOR_NAME+=gc4663
endif
ifeq ($(CONFIG_imx258),y)
    SENSOR_NAME+=imx258
endif
ifeq ($(CONFIG_sc4336),y)
    SENSOR_NAME+=sc4336
endif
ifeq ($(CONFIG_sc4336p),y)
    SENSOR_NAME+=sc4336p
endif
ifeq ($(CONFIG_sc5336),y)
    SENSOR_NAME+=sc5336
endif
ifeq ($(CONFIG_sc200ai),y)
    SENSOR_NAME+=sc200ai
    ifeq ($(CONFIG_sc200ai_8bit),y)
        SENSOR_NAME+=sc200ai_8bit
    endif
endif
ifeq ($(CONFIG_os02g10),y)
    SENSOR_NAME+=os02g10
endif
ifeq ($(CONFIG_bf2257cs),y)
    SENSOR_NAME+=bf2257cs
endif
ifeq ($(CONFIG_nvp6158c),y)
    SENSOR_NAME+=nvp6158c
endif
export SENSOR_NAME

ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
    LIBISP_CFG_TOOLCHAIN_LIBC := musl
else ifeq ($(CONFIG_TOOLCHAIN_LIBC), "glibc")
    LIBISP_CFG_TOOLCHAIN_LIBC := glibc
endif
export LIBISP_CFG_TOOLCHAIN_LIBC

#compile static libs
ifeq ($(CONFIG_libisp_compile_static_lib),y)
    LIBISP_CFG_COMPILE_STATIC_LIB := Y
else
    LIBISP_CFG_COMPILE_STATIC_LIB := N
endif
export LIBISP_CFG_COMPILE_STATIC_LIB

#compile dynamic libs
ifeq ($(CONFIG_libisp_compile_dynamic_lib),y)
    LIBISP_CFG_COMPILE_DYNAMIC_LIB := Y
else
    LIBISP_CFG_COMPILE_DYNAMIC_LIB := N
endif
export LIBISP_CFG_COMPILE_DYNAMIC_LIB

ifeq ($(CONFIG_libisp_enable_systrace),y)
    LIBISP_CFG_SYSTRACE := Y
else
    LIBISP_CFG_SYSTRACE := N
endif
export LIBISP_CFG_SYSTRACE

ifeq ($(CONFIG_libisp_support_fastboot),y)
    LIBISP_CFG_FASTBOOT := Y
else
    LIBISP_CFG_FASTBOOT := N
endif
export LIBISP_CFG_FASTBOOT

ifeq ($(CONFIG_llibisp_build_libispapi),y)
    LIBISP_CFG_LIBISPAPI := Y
else
    LIBISP_CFG_LIBISPAPI := N
endif
export LIBISP_CFG_LIBISPAPI

ifeq ($(CONFIG_libisp_build_tunning_app),y)
    LIBISP_CFG_AWTUNNING_APP := Y
else
    LIBISP_CFG_AWTUNNING_APP := N
endif
export LIBISP_CFG_AWTUNNING_APP

ifeq ($(CONFIG_libisp_software_tdm),y)
    LIBISP_SOFTWARE_TDM_FOR_ISP := Y
else
    LIBISP_SOFTWARE_TDM_FOR_ISP := N
endif
export LIBISP_SOFTWARE_TDM_FOR_ISP
