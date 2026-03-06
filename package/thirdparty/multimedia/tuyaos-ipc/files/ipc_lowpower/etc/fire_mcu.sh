if [ ! -e /tmp/rtos.bin ]; then
    echo "not find /tmp/rtos.bin!!"
    exit 1
fi
partition_size=$(cat /proc/partitions | grep "mtdblock6" | tr -s ' ' | cut -d' ' -f4)
echo "partition_size: $partition_size"
firmware_size=$(du /tmp/rtos.bin | cut -f1)
echo "firmware_size: $firmware_size"
if [ "$firmware_size" -gt "$partition_size" ]; then
    echo "error, exit! rtos.bin bigger than /dev/mtd6 "
    exit 1  # 停止升级过程
fi

cd /tmp/
killall monitor_ipc.sh
killall monitor_low_bat.sh
killall tuyaos_demo_ipc
flash_eraseall /dev/mtd6
flashcp -v /tmp/rtos.bin /dev/mtd6
echo "delay 2s reboot system..."
sleep 2
reboot
