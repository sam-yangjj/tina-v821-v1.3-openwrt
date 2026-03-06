#!/bin/bash

hook_dir=${TINA_TOPDIR}/openwrt/target/$LICHEE_IC/$LICHEE_IC-common/build_hooks
update_tools=${LICHEE_TOOLS_DIR}/pack/pctools/linux/mod_update/update_unflatten_fdt_size
dtb=${outpath}/${outname}

echo "tina hook dtb: ${dtb}"
echo "hook dir:${TINA_TOPDIR}/openwrt/target/$LICHEE_IC/$LICHEE_IC-common/build_hooks"

if [ ! -f $update_tools ]; then
	echo "update_unflatten_fdt_size tool not found, skip update_unflatten_fdt_size"
else
	node_size=`${hook_dir}/get_kernel_type_size.sh ${LICHEE_TOP_DIR}/.buildconfig "sizeof(struct device_node)"`
	node_align=`${hook_dir}/get_kernel_type_size.sh ${LICHEE_TOP_DIR}/.buildconfig "__alignof__(struct device_node)"`
	prop_size=`${hook_dir}/get_kernel_type_size.sh ${LICHEE_TOP_DIR}/.buildconfig "sizeof(struct property)"`
	prop_align=`${hook_dir}/get_kernel_type_size.sh ${LICHEE_TOP_DIR}/.buildconfig "__alignof__(struct property)"`
	$update_tools $dtb $node_size $node_align $prop_size $prop_align
fi
