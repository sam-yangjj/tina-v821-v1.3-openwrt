#!/bin/sh

# The device use either usb1 or usb2 or...
get_usb()
{
    readlink /sys/class/block/${dev} | awk -F'\/' '{print $6}'
}

record()
{
    checked="${checked} ${usbn}"
    [ "$(echo ${checked} | awk '{print NF}')" -ge ${usb_count} ] \
        && exit 0
}

get_capacity()
{
    fsize="/sys/block/${dev}/size"
    ! [ -f "${fsize}" ] \
        && echo "/dev/${dev}: get capacity failed" \
        && return 1

    capacity="$(cat ${fsize} | awk '{sum+=$0}END{printf "%.2f\n", sum/2097152}')"
    RED='\033[0;31m'
    NC='\033[0m'
    echo -e "${RED}${usbn} : /dev/${dev} : ${capacity}G${NC}"
    return 0
}

usb_count=`mjson_fetch /base/production/udisktester/usb_count`
[ -z "${usb_count}" ] \
    && echo "No set usb count, use default 1" \
    && usb_count=1
echo "usb count to check : ${usb_count}"
echo "-----------------------------"

run_once_time=`mjson_fetch /base/production/udisktester/testcase_run_once_time` #second minute hour day
run_once_time=$(echo ${run_once_time} | awk '{total = $4 * 86400 + $3 * 3600 + $2 * 60 + $1;print total}')
echo $0 run times limit ${run_once_time} seconds
check_times=0
while true
do
    [ ${check_times} -le ${run_once_time} ] || {
        echo udisktester timeout
        exit 1
    }
    for dev in $(cd /dev/ && ls sd[a-z] 2>/dev/null)
    do
        [ -b "/dev/${dev}" ] || continue
        usbn=$(get_usb)
        # if checked before, continue loop.
        echo ${checked} | grep "${usbn}" &>/dev/null && continue
        get_capacity && record ${usbn}
    done
    check_times=$((check_times + 1))
    sleep 1
done
