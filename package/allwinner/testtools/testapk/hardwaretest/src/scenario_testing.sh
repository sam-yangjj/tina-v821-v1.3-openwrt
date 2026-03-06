#!/bin/ash
#需要连接wifi后再执行该脚本
#执行脚本后，脚本中会执行cpu_monitor和mtop，注意看log是否正常
#cpu_monitor可以看到开关核的状态和cpu的频率，默认为schedutil调频策略，注意测试时cpu频率是否正确
#mtop可以看到各个模块的带宽占用，如果占用为0则有异常
#cpulimit可以限制单个进程的cpu占用率，cpulimit -p pid -l 50 &

killall mtop
killall cpu_monitor
killall stress-ng

#随机设置CPU调频模式函数
function set_cpu_scaling_governor() {
	random_hex=$(hexdump -n 1 -e '1/1 "%02x"' /dev/urandom | head -c2)
	echo "$random_hex"
	decimal_number=$(printf "%d" 0x$random_hex)

	if [ $decimal_number -lt 85 ]; then
		echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	elif [ $decimal_number -ge 85 ] && [ $decimal_number -lt 170 ]; then
		echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	else
		echo schedutil > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	fi
}

echo 0 > /proc/sys/kernel/printk
#开关ledc
#while true; do echo 10 > /sys/class/leds/sunxi_led0g/brightness; sleep 2; echo 0 > /sys/class/leds/sunxi_led0g/brightness; sleep 2; done &
#sleep 1
echo 8 > /proc/sys/kernel/printk

#关闭温控
#echo 100000 > /sys/devices/virtual/thermal/thermal_zone0/trip_point_0_temp
#echo 110000 > /sys/devices/virtual/thermal/thermal_zone0/trip_point_1_temp
#echo 115000 > /sys/devices/virtual/thermal/thermal_zone0/trip_point_2_temp
#echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
#echo 1800000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed
#fdt set /cluster-opp-table/opp@1800000000 status "disabled"
#fdt set /cluster-opp-table/opp@1800000000 opp-microvolt-vf0000 <1180000>
#echo 0 > /sys/devices/system/cpu/cpu1/online
#echo 0 > /sys/devices/system/cpu/cpu2/online
#echo 0 > /sys/devices/system/cpu/cpu3/online

#PID 是stress-ng的进程号，ps可以看到最后两个stress-ng进程，限制90%占用率
#cpulimit -p PID -l 90 &

#wifi进低功耗
#wifi -e "linux: ioctl: wlan0 setsuspendmode 1"
echo disabled > /sys/class/thermal/thermal_zone0/mode
echo disabled > /sys/class/thermal/thermal_zone1/mode

#设置uart回环
echo enable > /sys/devices/platform/soc@3000000/2605000.uart/loopback
echo enable > /sys/devices/platform/soc@3000000/2606000.uart/loopback

#设置内存水位线，flash读写占用太多缓存
echo 51200 > /proc/sys/vm/min_free_kbytes

#关闭崩溃后3秒重启机制
echo 0 > /sys/module/kernel/parameters/panic

#限制g2d 最大800M带宽
#echo 8 1 > /sys/class/nsi-pmu/hwmon0/port_abs_bwlen
#echo 8 800 > /sys/class/nsi-pmu/hwmon0/port_abs_bwl

#cpu电压
cat /sys/kernel/debug/opp/cpu0/opp*/supply-0/u_volt_target
#cpu频点
cat /sys/kernel/debug/opp/cpu0/opp*/rate_hz

#各模块时钟，ve需要运行ve应用后才变成实际的时钟
cat /sys/kernel/debug/clk/clk_summary

#设置ddr调频模式为performance
#echo performance > /sys/class/devfreq/3120000.dmcfreq/governor
#echo userspace > /sys/class/devfreq/3120000.dmcfreq/governor
#echo sunxi_actmon > /sys/class/devfreq/3120000.dmcfreq/governor
#echo 1392000000 > /sys/class/devfreq/3120000.dmcfreq/userspace/set_freq

#rpmsg_test异构通信测试，需要rv端运行rpmsg_test -N test -L 496 -c -t 1 -M 10
rpmsg_test -r e907_rproc@1a00000 -c test -L 496 -t 5 &

#rpbuf_test异构通信测试，需要rv端运行rpbuf_test -c -I 0 -N test -L 0x1000
#rpbuf_test -s -c 100 -I 0 -N test -L 0x1000
#设置喇叭音量
amixer cset numid=3 40

#跑ddr带宽
#df_dok --dfb:system=dummy --dfb:mode=1920x1080 --size 800x480 --system --fill-traps --duration 864000000 &

#ddr压力测试
stressapptest -s 43200 -C 1 -W -M 16 &

#mbw64 -n 1000000 100M &
#g2d性能测试
#tt /base/display/g2d_performancetester &

#npu性能测试
#vpm_run -s /etc/npu/sample_viplite/VGG19/sample.txt -l 1000000000000 > /dev/null &

#循环播放音乐
while true;do aplay /etc/s16le_44100_stereo.wav; sleep 1;free; done &

#录音
#arecord -Dhw:sndi2s0 -f S16_LE -r 16000 -c 1 -t wav  /tmp/test_16000.wav

#uart+dma回环测试
while true;do uart_test -d 5 -b 1500000 -s 256 -n 2000000000; done &
while true;do uart_test -d 6 -b 115200 -s 256 -n 200; done &

#cpu压力测试
while true; do stress-ng --cpu 2 --io 1 --vm 1 --vm-bytes 8M; sleep 1; done &

#spi3读取陀螺仪数据
#echo 1 > /sys/devices/virtual/input/input0/sensor_init
#echo 7 > /sys/devices/virtual/input/input0/acc_mode
#while true; do cat /sys/devices/virtual/input/input0/acc_val; sleep 2; done &

#生产h264编码文件
#encodertest 0 NV12 1920 1080 1920 1080 1500 /usr/lib/tt-data/bike_1920x1080_220.bin /tmp
#encodertest 0 YUV420 3840 2160 3840 2160 150 /etc/npu/sample_viplite/3840x2160_yv21.yuv /tmp
#压力测试h264编码文件，会对比每一帧是否一致
#encoderStressTest 0 NV12 1920 1080 1920 1080 1500 /usr/lib/tt-data/bike_1920x1080_220.bin /mnt/UDISK /etc/npu/sample_viplite/1920_1080_vs.264 10000000 432 &
#encoderStressTest 0 YUV420 3840 2160 3840 2160 150 /etc/npu/sample_viplite/3840x2160_yv21.yuv /mnt/UDISK /etc/npu/sample_viplite/3840_2160_vs.264 10000000 432 &
#JPEG解码
#jpegdecodedemo /mnt/UDISK/timg.jpeg 0 nv21
#MJPEG解码
#decodertest /etc/npu/sample_viplite/JPEG.mp4 /mnt/UDISK 0 NV21 2 10

#联网pin，需要先连接网络
#wifi -c AWTest 1qaz@WSX
ping www.baidu.com &

#两台板子对测性能，-s是服务端，-c是客户端，-c后的ip地址是服务端的ip地址，-t是时间，秒为单位
#iperf -s -i 1 &
#iperf -c 10.10.10.124 -i 1 -t 259200 &
#下载文件并校验
#/etc/npu/sample_viplite/wifi_test.sh &

#前摄单独出图
#camerademo NV21 2592 1944 30 yuv /tmp 10
#后摄单独出图
#camerademo NV21 640 480 30 yuv /tmp 10 1
#读取裸数据出图，不用接物理上的摄像头
#camerademo  NV21 1920 1080 30 bmp /tmp 10 4 process_raw /etc/npu/sample_viplite/source_RGGB10.raw
#/etc/recorder.cfg camera_id 的video_width video_height，可以修改输入分辨率
#csi+isp+ve
#前摄5M30fps，ve 1080p30s
#trecorderdemo 3 &
#后摄0.3M30fps，ve 1080p30s
#trecorderdemo 4 &
#双摄5M30fps, 0.3M30fps，ve 1080p30s
#trecorderdemo 5 &
#双摄5M30fps, 0.3M30fps，ve 1080p30s，录制10分钟，然后重新录制
#while true; do echo 1 >  /proc/sys/vm/drop_caches; trecorderdemo 6; sleep 2; done &

#双摄出图保存文件
camera_double_test &

cpu_monitor -s 5000 &
mtop -m -d 5 &
cat /dev/kmsg > /mnt/UDISK/kernel.log &

sleep 10
#读写压力测试
rwcheck -d /mnt/UDISK -b 128k -t 100000000 -p 80 &

#随机设置cpu调频策略
while true; do
    set_cpu_scaling_governor
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    sleep 10
done
