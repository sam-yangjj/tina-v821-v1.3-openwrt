#!/bin/bash

rootfsdir=$1
hookdir=${LICHEE_TOP_DIR}/openwrt/target/${LICHEE_IC}/${LICHEE_IC}-common/build_hooks
platdir=${LICHEE_TOP_DIR}/openwrt/target/${LICHEE_IC}/${LICHEE_IC}-${LICHEE_BOARD}
configdir=${LICHEE_BOARD_CONFIG_DIR}

if [ -e "${LICHEE_BOARD_CONFIG_DIR}/usr_list.json" ]; then
		echo "--------- process update usr rootfs ---------"
		${LICHEE_TOP_DIR}/prebuilt/hostbuilt/python3.8/bin/python3 ${LICHEE_TOP_DIR}/tools/quick/auto_update_usr ${LICHEE_TOP_DIR}/.buildconfig \
			--config ${LICHEE_BOARD_CONFIG_DIR}/usr_list.json
fi
# process kernel ko
if [ -e "${platdir}/kernel_modules.json" ]; then
	if [ ! -e "${LICHEE_BUILD_DIR}/bin/aw_modupdate" ]; then
		echo "aw_modupdate execute file not exist"
	fi
	echo "--------- process kernel modules ---------"
	${LICHEE_BUILD_DIR}/bin/aw_modupdate \
			-l -i ${platdir}/kernel_modules.json \
			-k ${TINA_TOPDIR}/out/kernel/build \
			-s ${rootfsdir}/lib/modules
fi
