#!/bin/sh

# echo "create firewall..."
# firewall

# echo "run monitor low battery..."
# /etc/monitor_low_bat.sh

echo "run devctrl..."
devctrl &

# echo "run monitor process..."
# /etc/monitor_ipc.sh start

echo "run tuya demo..."
tuyaos_demo_ipc -d1 &

