#!/bin/ash

count=20

killall mtop
killall cpu_monitor
killall stress-ng

cpu_monitor -s 20000 &

while true; do
    # 生成随机数（1-20）
    rand_num=$(hexdump -n 2 -e '/2 "%u"' /dev/urandom 2>/dev/null)
    index=$(( (rand_num % count) + 1 ))  # 修正索引范围为1-20
    
    # 单次压力测试（带超时）
    stress-ng --cpu 2 --io 1 --vm 1 --vm-bytes 48M --timeout ${index}
    
    # 休眠（单位自动为秒）
    sleep ${index}
done
