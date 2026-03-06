#!/bin/bash
#this shell included by Makefile to set some default params
#for example: ./set_cedar_conf.sh vbv_buffer_size $(CONFIG_VBVBUFSIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_VBV_ENABLE)
line=`sed -n "/\<$1\>/p" $3`
if [ -n "$line" ]
then
if [ $4 == "y" ]
then
sed -i "s/$line/$1 = $2/g" $3
fi
else
if [ $4 == "y" ]
then
sed -i '3a '$1' = '$2'' $3
fi
fi
