#!/bin/sh

echo "create /mnt/UDISK/ota_info"
echo "boot_type=3" > /mnt/UDISK/ota_info

echo "set gprcm to reboot from nor"
echo 0x4a000258 0x30000000 > /sys/class/sunxi_dump/write

#echo "switch to sdnand rootfsA"
#mkdir /tmp/tmp_root
#mount /dev/mmcblk0p9 /tmp/tmp_root
#switch_root /tmp/tmp_root
#
#echo "destroy nor boot0"
#dd if=/dev/zero of=/dev/mtdblock0 bs=8K count=2

return 0
