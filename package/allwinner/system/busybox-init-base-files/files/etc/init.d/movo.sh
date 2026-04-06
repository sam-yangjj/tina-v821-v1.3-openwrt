#!/bin/sh

df -h
sleep 2
ln -s /mnt/extsd /userdata
/etc/init.d/dropbear start
sleep 2
/usr/sbin/dropbear -p 22
/etc/init.d/sync.sh &
# /userdata/sample_smartIPC_demo -path sample_smartIPC_demo.conf &
wifi -c Xiaomi_E93C Sam123456789
