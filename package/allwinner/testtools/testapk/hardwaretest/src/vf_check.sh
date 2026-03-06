#!/bin/ash

#encodertest 0 NV12 1920 1080 1920 1080 1 /usr/lib/tt-data/bike_1920x1080_220.bin /tmp

echo "dmesg | grep dvfs"
dmesg | grep dvfs

echo "cat /sys/kernel/debug/opp/cpu0/opp*/supply-0/u_volt_target"
cat /sys/kernel/debug/opp/cpu0/opp*/supply-0/u_volt_target

echo "cat /sys/kernel/debug/opp/cpu0/opp*/rate_hz"
cat /sys/kernel/debug/opp/cpu0/opp*/rate_hz

echo "cat /sys/class/dsufreq/scaling_available_frequencies"
cat /sys/class/dsufreq/scaling_available_frequencies

echo "cat /sys/kernel/debug/clk/clk_summary | grep "rv\|e907\|de\|g2d\|isp\|mbus\|nsi\|apb\|ahb\|ddr\|dram""
cat /sys/kernel/debug/clk/clk_summary | grep "rv\|e907\|de\|g2d\|isp\|mbus\|nsi\|apb\|ahb\|ddr\|dram"

echo "cd /sys/devices/system/cpu/cpufreq/policy0"
cd /sys/devices/system/cpu/cpufreq/policy0
grep -rn "" *

echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

available_frequencies=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies`
if [ -z "$available_frequencies" ]; then
	echo "no available frequencies"
	exit
fi

get_element() {
    local index=$1
    local array=$2
    local count=0
    
    # 使用 for 循环和 awk 来分割字符串，并计数
    for element in $array; do
        if [ $count -eq $index ]; then
            echo $element
            return
        fi
        count=$((count + 1))
    done
    
    # 如果索引超出范围，则打印错误信息
    echo "Index out of range"
}

sleep 1
npu_available_frequencies=`cat /sys/kernel/debug/viplite/clk_freq | grep Hz`

# 将输入传递给循环处理
echo "$npu_available_frequencies" | while read -r line; do
    # 提取频率值并打印
    freq=$(echo "$line" | cut -d' ' -f1)
    echo "echo $freq > /sys/kernel/debug/viplite/clk_freq"
    sleep 1
    echo $freq > /sys/kernel/debug/viplite/clk_freq
    sleep 1
    echo "cat /sys/kernel/debug/viplite/vip_freq"
    sleep 1
    cat /sys/kernel/debug/viplite/vip_freq
    sleep 1
    freq_vol=`i2cdump -f -y 7 0x36 | grep "10:"`
    cur_vol=$(get_element 6 "$freq_vol")
    cur_vol_tmp=`printf "%d\n" 0x$cur_vol`
    cur_vol_tmp=`expr $cur_vol_tmp + 50`
    cur_vol_tmp=`expr $cur_vol_tmp \* 10000`
    echo NPUVolt[$cur_vol_tmp]
done

CPUFREQ_ARRAY=$available_frequencies
freq_count=0
IFS=" "

set -- $CPUFREQ_ARRAY
for freq in "$@"; do
    #echo "freq=$freq"
    freq_count=$((freq_count + 1))
done

size=$freq_count
echo cpufreq_array_size=$size
echo ""

dvfs_cur_count=0

while true
do
	if [ $dvfs_cur_count -ge $freq_count ]; then
		break
	fi

	echo No.$dvfs_cur_count

	freq_target=$(get_element $dvfs_cur_count "$CPUFREQ_ARRAY")
	echo $freq_target > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

	cur_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`

	set_vol=`cat /sys/kernel/debug/opp/cpu0/opp:$freq_target\000/supply-0/u_volt_target`

	#echo "i2cdump -f -y 7 0x36 | grep "0123456789abcdef\|10""
	#i2cdump -f -y 7 0x36 | grep "0123456789abcdef\|10"
	freq_vol=`i2cdump -f -y 7 0x36 | grep "10:"`
	cur_vol=$(get_element 4 "$freq_vol")
	cur_vol_tmp=`printf "%d\n" 0x$cur_vol`
	cur_vol_tmp=`expr $cur_vol_tmp + 50`
	cur_vol_tmp=`expr $cur_vol_tmp \* 10000`

	if [ $cur_freq = $freq_target ]; then
		if [ $cur_vol_tmp = $set_vol ]; then
			echo CpuCurFreq[$cur_freq] CurVolt[$cur_vol_tmp] OK
		else
			echo CpuCurFreq[$cur_freq] CurVolt[$cur_vol_tmp] SetVolt[$set_vol] FAILURE
		fi
	else
		echo CpuCurFreq[$cur_freq] TargetFreq=[$freq_target] FAILURE
	fi

	echo "cat /sys/kernel/debug/clk/clk_summary | grep dsu"
	cat /sys/kernel/debug/clk/clk_summary | grep dsu

	echo ""
	let dvfs_cur_count++

	usleep 100000
done

freq_vol=`i2cdump -f -y 7 0x35 | grep "80:"`
cur_vol=$(get_element 4 "$freq_vol")
cur_vol_tmp=`printf "%d\n" 0x$cur_vol`
cur_vol_tmp=`expr $cur_vol_tmp + 50 - 128`
cur_vol_tmp=`expr $cur_vol_tmp \* 10000`
echo VddSysVolt[$cur_vol_tmp]

freq_vol=`i2cdump -f -y 7 0x35 | grep "80:"`
cur_vol=$(get_element 6 "$freq_vol")
cur_vol_tmp=`printf "%d\n" 0x$cur_vol`
cur_vol_tmp=`expr $cur_vol_tmp + 50 - 128`
cur_vol_tmp=`expr $cur_vol_tmp \* 10000`
echo VccDramVolt[$cur_vol_tmp]

freq_vol=`i2cdump -f -y 7 0x35 | grep "90:"`
cur_vol=$(get_element 16 "$freq_vol")
cur_vol_tmp=`printf "%d\n" 0x$cur_vol`
cur_vol_tmp=`expr $cur_vol_tmp \* 5 + 50`
cur_vol_tmp=`expr $cur_vol_tmp \* 10000`
echo VddCpusVolt[$cur_vol_tmp]