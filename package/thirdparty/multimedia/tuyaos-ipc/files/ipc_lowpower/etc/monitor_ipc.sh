#!/bin/sh

PROCESS_NAME="tuyaos_demo_ipc"
CHECK_INTERVAL=20
REBOOT_DELAY=15
MONITOR_PID_FILE="/var/run/tuyaipc_monitor.pid"

is_process_running() {
    ps | grep "$PROCESS_NAME" | grep -v grep > /dev/null
}

monitor() {
    sleep 3
    while true; do
        if is_process_running; then
            :  # No action
        else
            echo "$PROCESS_NAME not running"

            sleep $REBOOT_DELAY

            if ! is_process_running; then
                echo "Confirmed $PROCESS_NAME not running. Rebooting now"
                reboot
                exit 1
            fi
        fi

        sleep $CHECK_INTERVAL
    done
}

start() {
    if [ -f "$MONITOR_PID_FILE" ]; then
        local pid=$(cat "$MONITOR_PID_FILE")
        if ps | grep "$pid" | grep -v grep > /dev/null; then
            echo "Monitor is already running (PID: $pid)"
            return 0
        fi
        echo "Cleaning up stale PID file"
        rm "$MONITOR_PID_FILE"
    fi

    echo "Starting monitor..."
    monitor &
    echo $! > "$MONITOR_PID_FILE"
    echo "Monitor started with PID: $(cat "$MONITOR_PID_FILE")"
}

stop() {
    if [ ! -f "$MONITOR_PID_FILE" ]; then
        echo "Monitor is not running"
        return 0
    fi

    local pid=$(cat "$MONITOR_PID_FILE")
    if ! ps | grep "$pid" | grep -v grep > /dev/null; then
        echo "Monitor PID file exists but process not found. Cleaning up."
        rm "$MONITOR_PID_FILE"
        return 0
    fi

    echo "Stopping monitor (PID: $pid)..."
    kill "$pid"
    rm "$MONITOR_PID_FILE"
    echo "Monitor stopped"
}

usage() {
    echo "Usage: $0 {start|stop}"
    exit 1
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    *)
        usage
        ;;
esac
