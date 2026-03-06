#!/bin/bash

dot_buildconfig=$1
data_type=$2

if [ $# -ne 2 ]; then
	echo "usage: $0 /path/to/.buildconfig type"
	exit 1
fi

if [ ! -f "$dot_buildconfig" ]; then
	echo "$dot_buildconfig not exist!!!"
	exit 1
fi

source $dot_buildconfig

# prepare variant
CC=${LICHEE_TOOLCHAIN_PATH}/bin/${LICHEE_CROSS_COMPILER}-
kernel_src=${LICHEE_KERN_DIR}
kernel_out=${LICHEE_TOP_DIR}/out/kernel/build
kernel_arch=${LICHEE_KERNEL_ARCH}

mkdir .tmp_dir
echo "#include <linux/module.h>" > .tmp_dir/test.c
echo "#include <linux/of.h>" >> .tmp_dir/test.c
echo "char test_array[${data_type}];" >> .tmp_dir/test.c
echo 'MODULE_LICENSE("GPL");' >> .tmp_dir/test.c

echo "obj-m += test.o" > .tmp_dir/Makefile
echo "KDIR := ${kernel_out}" >> .tmp_dir/Makefile
echo "PWD := ${PWD}/.tmp_dir" >> .tmp_dir/Makefile
echo "ARCH := ${kernel_arch}" >> .tmp_dir/Makefile
echo "export CROSS_COMPILE = ${CC}" >> .tmp_dir/Makefile
echo "all:" >> .tmp_dir/Makefile
echo -e '\tmake -C $(KDIR) ARCH=$(ARCH) M=$(PWD) modules' >> .tmp_dir/Makefile
echo "clean:" >> .tmp_dir/Makefile
echo -e '\tmake -C $(KDIR) ARCH=$(ARCH) M=$(PWD) clean' >> .tmp_dir/Makefile
make -C .tmp_dir 2>&1 > /dev/null
readelf -s .tmp_dir/test.o|grep test_array|awk '{print $3}'
make -C .tmp_dir clean 2>&1 > /dev/null
rm -rf .tmp_dir
