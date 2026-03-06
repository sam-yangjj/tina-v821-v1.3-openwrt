#!/bin/sh

# 功能：开机检查电量，运行中定期监控电量，低电量时进入休眠，并设置定时系统唤醒

BATTERY_CAPACITY_FILE="/sys/class/power_supply/axp2602-battery/capacity"
BATTERY_VOLTAGE_FILE="/sys/class/power_supply/axp2602-battery/voltage_now"
APP_NAME="tuyaos_demo_ipc"
LOW_BATTERY_THRESHOLD=15   # 低电量阈值（百分比）
RESUME_SYSTEM_INTERVAL=$((15 * 60 * 1000))  # 唤醒系统间隔（分钟）
CHECK_INTERVAL=60  # 电量检查间隔（秒）

is_process_running() {
    ps | grep "$APP_NAME" | grep -v grep > /dev/null
}

check_battery_level() {
    if [ ! -f "$BATTERY_CAPACITY_FILE" ]; then
        echo "错误：无法找到电池电量文件 $BATTERY_CAPACITY_FILE"
        exit 1
    fi

    battery_level=$(cat "$BATTERY_CAPACITY_FILE")

    if [ -z "$battery_level" ] || ! echo "$battery_level" | grep -q '^[0-9]\+$'; then
        echo "错误：无法读取有效的电池电量"
        exit 1
    fi

    echo "$battery_level"
}

handle_low_battery() {
    echo "当前电池电量低：$1%"
    if is_process_running; then
        echo "正在关闭 $APP_NAME 应用..."
        killall "$APP_NAME"
        sleep 5
        if is_process_running; then
            echo "警告：无法正常关闭 $APP_NAME，尝试强制终止..."
            killall -9 "$APP_NAME"
        fi
        reboot #Fix invalid timing
        exit 0
    fi

    echo "系统将进入休眠，定时唤醒 $RESUME_SYSTEM_INTERVAL ms"
    sleep 10
    echo $RESUME_SYSTEM_INTERVAL > /sys/class/ae350_standby/time_to_wakeup_ms
    echo 1 > /sys/class/ae350_standby/use_ultra_standby
    poweroff
}

monitor() {
    while true; do
        sleep "$CHECK_INTERVAL"
        current_battery=$(check_battery_level)

        if [ "$current_battery" -lt "$LOW_BATTERY_THRESHOLD" ]; then
            handle_low_battery "$current_battery"
            exit 0
        fi
    done
}

main() {
    initial_battery=$(check_battery_level)
    current_voltage=$(cat "$BATTERY_VOLTAGE_FILE" 2>/dev/null || echo "N/A")

    echo "当前电池电量：$initial_battery%，阈值电量：$LOW_BATTERY_THRESHOLD%, 当前电压：${current_voltage}μV"

    if [ "$initial_battery" -lt "$LOW_BATTERY_THRESHOLD" ]; then
        handle_low_battery "$initial_battery"
        sleep 3
        exit 0
    fi

    monitor &
}

main
