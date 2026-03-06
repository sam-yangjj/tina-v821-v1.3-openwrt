#!/bin/bash
#
# This script is a hook and it is be run by SUPPORT_PACK_OUT_OF_TINA function
#
# You can add commands here for copy some files or resource to aw_pack_src
# directory
#
# Show aw_pack_src directory name, you should copy file to correct directory
#
#./aw_pack_src
#|--aw_pack.sh  #执行此脚本即可在aw_pack_src/out/目录生成固件
#|--rootfs      #存放rootfs的tar.gz打包,给二次修改使用
#|--lib_aw      #拷贝全志方案的库文件，如多媒体组件eyesempp等,
#		给应用app编译链接使用(没有选择这些库，则可能是空文件).
#|--README      #关于板级方案的一些说明，例如分区布局等等(无说明则没有这个文件)

#
#NOTE: input parameter ${output_path} is a path of aw_pack_src
#

#author: wuguanling@allwinnertech.com

aw_top=${TINA_TOPDIR}
source ${aw_top}/.buildconfig
output_path=$1


mkdir -p ${output_path}/lib_aw/

#This script of this project need to copy eyesempp lib to aw_pack_src/lib_aw
if [ -d ${LICHEE_PLAT_OUT}/build_dir/target/libcedarc ]; then
	mkdir -p ${output_path}/lib_aw/lib/libcedarc
	mkdir -p ${output_path}/lib_aw/include/libcedarc
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/libcedarc/libcedarc/InstallDev/usr/lib/* ${output_path}/lib_aw/lib/libcedarc
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/libcedarc/libcedarc/InstallDev/usr/include/libcedarc/* ${output_path}/lib_aw/include/libcedarc
fi

if [ -d ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/eyesee-mpp/ ]; then
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/eyesee-mpp/ ${output_path}/lib_aw/lib
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/include/eyesee-mpp/ ${output_path}/lib_aw/include
fi

# mpp need this lib
if [ -f ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libawion.a ]; then
	mkdir -p ${output_path}/lib_aw/lib/awion
	mkdir -p ${output_path}/lib_aw/include/awion
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libawion.a ${output_path}/lib_aw/lib/awion
	cp -rf ${LICHEE_PLAT_OUT}/build_dir/target/libawion/ipkg-install/usr/include/* ${output_path}/lib_aw/include/awion
fi

#mpp need this lib
if [ -f ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libexpat.a  ]; then
	mkdir -p ${output_path}/lib_aw/lib/expat
	mkdir -p ${output_path}/lib_aw/include/expat
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libexpat.a ${output_path}/lib_aw/lib/expat
	cp -rf ${LICHEE_PLAT_OUT}/build_dir/target/expat-*/ipkg-install/usr/include/* ${output_path}/lib_aw/include/expat
fi

#pdet
if [ -d ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/pdet  ]; then
        mkdir -p ${output_path}/lib_aw/lib/pdet
        mkdir -p ${output_path}/lib_aw/include/pdet
        cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/pdet/*.a ${output_path}/lib_aw/lib/pdet
        cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/include/pdet/* ${output_path}/lib_aw/include/pdet
fi


#cp v821 wifi lib , lib.so also in rootfs
if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libnl-tiny.so ]; then
	mkdir -p ${output_path}/lib_aw/lib/wifi
	mkdir -p ${output_path}/lib_aw/include/wifi
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libnl-tiny.so ${output_path}/lib_aw/lib/wifi/
	cp -rf ${LICHEE_PLAT_OUT}/build_dir/target/libnl-tiny-*/ipkg-install/usr/include/libnl-tiny ${output_path}/lib_aw/include/wifi/

fi

#mpp need
if [ -d  ${LICHEE_PLAT_OUT}/build_dir/target/libbaselist ]; then
	mkdir -p ${output_path}/lib_aw/include/libbaselist/
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/libbaselist/src/include/base_list_type.h ${output_path}/lib_aw/include/libbaselist/

fi

#mpp need
if [ -d  ${LICHEE_PLAT_OUT}/build_dir/target/motion_detection ]; then
	mkdir -p ${output_path}/lib_aw/lib/motion_detection
	mkdir -p ${output_path}/lib_aw/include/motion_detection
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/motion_detection/src/InstallDev/usr/include/* ${output_path}/lib_aw/include/motion_detection/
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/motion_detection/src/InstallDev/usr/lib/*.a ${output_path}/lib_aw/lib/motion_detection/

fi

#mpp need
if [ -d  ${LICHEE_PLAT_OUT}/build_dir/target/libAWIspApi ]; then
	mkdir -p ${output_path}/lib_aw/lib/libAWIspApi
	mkdir -p ${output_path}/lib_aw/include/libAWIspApi
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/libAWIspApi/ipkg-install/usr/include/* ${output_path}/lib_aw/include/libAWIspApi/
	cp -rf  ${LICHEE_PLAT_OUT}/build_dir/target/libAWIspApi/ipkg-install/usr/lib/*.a ${output_path}/lib_aw/lib/libAWIspApi/
fi

#kernel headers
if [ -d  ${LICHEE_PLAT_OUT}/../../kernel/build/user_headers ]; then
	mkdir -p ${output_path}/lib_aw/include/kernel
	cp -rf ${LICHEE_PLAT_OUT}/../../kernel/build/user_headers/include/* ${output_path}/lib_aw/include/kernel/
fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/lib/libwifimg-v2.0.so ]; then
	mkdir -p ${output_path}/lib_aw/lib/wifi
	mkdir -p ${output_path}/lib_aw/include/wifi/wifimanager
	cp -rf ${LICHEE_PLAT_OUT}/staging_dir/target/lib/libwifimg-v2.0.so  ${output_path}/lib_aw/lib/wifi
	cp -rf ${LICHEE_PLAT_OUT}/build_dir/target/wifimanager-*/ipkg-install/usr/include/* ${output_path}/lib_aw/include/wifi/wifimanager
fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libwirelesscom.so ]; then
	mkdir -p ${output_path}/lib_aw/lib/wifi
	mkdir -p ${output_path}/lib_aw/include/wifi/wirelesscom
	cp -rf ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libwirelesscom.so  ${output_path}/lib_aw/lib/wifi
	cp -rf ${LICHEE_PLAT_OUT}/build_dir/target/wirelesscommon-*/ipkg-install/usr/include/* ${output_path}/lib_aw/include/wifi/wirelesscom
fi

#cp dynamic library
if [ -f ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/liblog.so ]; then
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/liblog.so ${output_path}/lib_aw/lib/
fi

if [ -d ${LICHEE_PLAT_OUT}/staging_dir/target/usr/include/glogCWrapper ]; then #glog 依赖liblog.so
	mkdir -p ${output_path}/lib_aw/include/glog
	cp -rf  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/include/glogCWrapper ${output_path}/lib_aw/include/glog
fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libasound.so ]; then
	cp  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libasound.so  ${output_path}/lib_aw/lib/
fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libglog.so ]; then
	cp  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libglog.so  ${output_path}/lib_aw/lib/
fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libz.so ]; then
	cp  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libz.so  ${output_path}/lib_aw/lib/

fi

if [ -f  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libjson-c.so ]; then
	cp  ${LICHEE_PLAT_OUT}/staging_dir/target/usr/lib/libjson-c.so  ${output_path}/lib_aw/lib/
fi

#cp rootfs
rootfs_dir=${LICHEE_PLAT_OUT}/build_dir/target/root-${LICHEE_IC}-${LICHEE_BOARD}
if [ -d ${rootfs_dir} ]; then
	cd ${rootfs_dir}
	mkdir -p ${output_path}/rootfs
	tar -czf ${output_path}/rootfs/rootfs.tar.gz ./
fi
