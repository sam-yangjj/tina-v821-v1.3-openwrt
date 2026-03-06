#!/bin/sh

erase_sdnand_boot0()
{
	#the first time start up after efex, erase sdnand boot0. So that start up from nor next time
	echo "erase sdnand boot0" > /dev/console
	dd if=/dev/zero of=/dev/mmcblk0 seek=16 bs=512 count=1
	dd if=/dev/zero of=/dev/mmcblk0 seek=256 bs=512 count=1
	fsync /dev/mmcblk0
}

erase_sdnand_rootfsdate()
{
	#the first time start up after efex, erase sdnand boot0. So that start up from nor next time
	echo "erase sdnand rootfsdate" > /dev/console
	dd if=/dev/zero of=/dev/mtdblock7 seek=0 bs=512 count=640
	fsync /dev/mtdblock7
}

erase_sdnand_boot0
erase_sdnand_rootfsdate
return 0
