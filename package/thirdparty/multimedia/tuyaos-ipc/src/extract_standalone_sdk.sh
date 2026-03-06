#! /bin/bash

# DEMO_ADAPTER_CODE_URL=https://images.tuyacn.com/rms-static/bcbc85e0-13fd-11ef-9eac-b120705c4c0c-1715916598591.tar.gz?tyName=tuyaos_adapter_3.9.0.tar.gz

# 使用grep和sed命令查找并提取字符串
sdk_version=$(grep -o "tuyaos-ipc_[^_]*_" ../../SDKInformation.json | sed -e 's/tuyaos-ipc_//' -e 's/_//')
echo $sdk_version
STANDALONE_SDK_DIR=../../sdk_$sdk_version

rm -rf $STANDALONE_SDK_DIR
mkdir -p $STANDALONE_SDK_DIR/sdk/libs
mkdir $STANDALONE_SDK_DIR/application_components
# mkdir $STANDALONE_SDK_DIR/build
#copy 环境变量
# cp -a ../build/build_param $STANDALONE_SDK_DIR/build
#copy sdk库
cp -a ../libs/libtuyaos.a $STANDALONE_SDK_DIR/sdk/libs
#copy sdk头文件
cp -a ../include/ $STANDALONE_SDK_DIR/sdk/
#copy demo
cp -a ../apps/tuyaos_demo_ipc/application_components/* $STANDALONE_SDK_DIR/application_components
#copy tuya_device_cfg.json
# cp -a ../apps/tuyaos_demo_ipc/tuya_device_cfg.json $STANDALONE_SDK_DIR/demo_tuyaos_app
#copy makefile
# cp -a Makefile_sdk $STANDALONE_SDK_DIR/Makefile

# cd $STANDALONE_SDK_DIR
#下载tuyaos_adapter
# wget $DEMO_ADAPTER_CODE_URL -O adapter_demo.tar.gz
# tar -xvf adapter_demo.tar.gz
# rm -f adapter_demo.tar.gz
# mv tuyaos_adapter/ demo_tuyaos_adapter
# rm -rf demo_tuyaos_adapter/local.mk

#从vendor获取tuyaos_adapter
cp -a ../vendor/v821_musl/tuyaos/tuyaos_adapter/ $STANDALONE_SDK_DIR/
find $STANDALONE_SDK_DIR -name local.mk -type f -delete
find $STANDALONE_SDK_DIR -name IoTOSconfig -type f -delete
cp -a ../../SDKInformation.json $STANDALONE_SDK_DIR/
cp -a extract_standalone_sdk.sh $STANDALONE_SDK_DIR/

exit 0