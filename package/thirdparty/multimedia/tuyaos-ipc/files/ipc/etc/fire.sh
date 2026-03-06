if [ ! -e /tmp/rootfs.squashfs ]; then
    echo "not find /tmp/rootfs.squashfs!!"
    exit 1
fi
partition_size=$(cat /proc/partitions | grep "mtdblock4" | tr -s ' ' | cut -d' ' -f4)
echo "partition_size: $partition_size"
firmware_size=$(du /tmp/rootfs.squashfs | cut -f1)
echo "firmware_size: $firmware_size"
if [ "$firmware_size" -gt "$partition_size" ]; then
    echo "error, exit! rootfs.squashfs bigger than /dev/mtd4 "
    exit 1  # 停止升级过程
fi

cd /tmp/
killall monitor_ipc.sh
killall monitor_low_bat.sh
killall tuyaos_demo_ipc
flash_eraseall /dev/mtd4
flashcp -v /tmp/rootfs.squashfs /dev/mtd4
echo "delay 2s reboot system..."
sleep 2
reboot
