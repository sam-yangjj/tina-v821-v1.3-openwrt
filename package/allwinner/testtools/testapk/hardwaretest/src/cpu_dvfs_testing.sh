#!/bin/ash

killall mtop
killall cpu_monitor
killall stress-ng

# 读取可用频率并存入位置参数
available_freqs=$(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies)
set -- $available_freqs
count=$#

# 设置调频策略为用户模式
echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

cpu_monitor &
mtop -m &

#cpu压力测试
while true; do stress-ng --cpu 2 --io 1 --vm 1 --vm-bytes 8M; sleep 1; done &

#ddr压力测试
stressapptest -s 43200 -C 1 -W -M 16 &

# 主循环
while true; do
    # 使用hexdump生成随机数（0-65535）
    rand_num=$(hexdump -n 2 -e '/2 "%u"' /dev/urandom 2>/dev/null)
    
    # 计算随机索引（0到count-1）
    index=$(( rand_num % count ))
    
    # 通过eval获取对应位置的频率值
    selected_freq=$(eval echo \$$((index + 1)))
    
    # 设置新频率
	echo "setting cpu freq $selected_freq"
    echo "$selected_freq" > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed
    
    # 休眠10秒
    sleep 10
done
