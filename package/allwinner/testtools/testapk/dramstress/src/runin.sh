#!/bin/sh

RUNIN_DIR=/mnt/UDISK/stressapptest/.runin
RUNIN_LOG=$RUNIN_DIR/log
CURRENT_LOOP_FILE=$RUNIN_DIR/loop_index
CURRENT_REBOOT_FILE=$RUNIN_DIR/reboot_time
CURRENT_TIME_FILE=$RUNIN_DIR/start_time
REBOOT_TIME_FILE=$RUNIN_DIR/reboot_test_time

function __runin_log()
{
    echo "$(date "+%Y-%m-%d %H:%M:%S") $@" > /dev/console | tee -a $RUNIN_LOG
}

# Test based on time, not times
function __runin_reboot_test()
{
	local rebootTime=0
	rebootTime=`cat $CURRENT_REBOOT_FILE`
	local startTime=0
	startTime=`cat $CURRENT_TIME_FILE`

	if [ -z $rebootTime ]; then
		rebootTime=0
		echo $rebootTime > $CURRENT_REBOOT_FILE
		sync
    fi

    if [ -z "$startTime" ]; then
        __runin_log "reboot test $@ hours"
        startTime=$(date "+%Y-%m-%d %H:%M:%S")
		echo $startTime > $CURRENT_TIME_FILE
		sync
    fi
    local currentTime=$(date "+%Y-%m-%d %H:%M:%S")
	currentTime=$(date "+%Y-%m-%d %H:%M:%S")
    local runTime=0
	runTime=$(($(date +%s -d "${currentTime}")-$(date +%s -d "${startTime}")))
	local testTime=0
	local testTime1=0
	# testTime=`echo "$1 * 60 * 60" | bc`
	testTime=$(($1*60*60))
	# local testTime=`echo "5 * 60" | bc`

	# __runin_log "<info> start time : $startTime"
	# __runin_log "<info> current time : $currentTime"
	# __runin_log "<info> running time : $runTime"
	# __runin_log "<info> test time : $testTime"

	local runTimeHour=$(( $runTime/3600 ))
	local runTimeMin=$(( ($runTime-${runTimeHour}*3600)/60 ))
	local runTimeSec=$(( ($runTime-${runTimeHour}*3600-${runTimeMin}*60) ))

	local remainingTime=$(( ${testTime}-${runTime} ))
	# if [ $remainingTime -lt 0 ]; then
        # remainingTime=0
    # fi
	local remainingTimeHour=$(( $remainingTime/3600 ))
	local remainingTimeMin=$(( ($remainingTime-${remainingTimeHour}*3600)/60 ))
	local remainingTimeSec=$(( ($remainingTime-${remainingTimeHour}*3600-${remainingTimeMin}*60) ))

	echo "=====================================" > /dev/console
	echo "<info> reboot test success: $rebootTime times." > /dev/console
	echo "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec" > /dev/console
	echo "<info> remaining time : $remainingTimeHour:$remainingTimeMin:$remainingTimeSec" > /dev/console
	echo "=====================================" > /dev/console
	__runin_log "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec"

	if [ $runTimeSec -lt 0 ]; then
		echo -e "\n\n" > /dev/console
        __runin_log "<info> reboot test done for low power."
		rm -rf $CURRENT_LOOP_FILE
		rm -rf $CURRENT_REBOOT_FILE
		rm -rf $CURRENT_TIME_FILE
		sync
		echo -e "\n\n" > /dev/console
        return
	fi

	echo "xbtest rutime:$runTime,testtime:$testTime" > /dev/console
	if [ -z $rebootTime ] || [ $runTime -gt $testTime ]; then
        # rm $CURRENT_TIME_FILE
		echo -e "\n\n" > /dev/console
        __runin_log "<info> reboot test done."
		rm -rf $CURRENT_LOOP_FILE
		rm -rf $CURRENT_REBOOT_FILE
		rm -rf $CURRENT_TIME_FILE
		sync
		echo -e "\n\n" > /dev/console
        return
    fi

	let rebootTime=rebootTime+1
    echo $rebootTime > $CURRENT_REBOOT_FILE
	sync

	cpu_monitor  > /dev/console &
	sleep 15
    reboot
    exit 0
}

# initialize
mkdir -p $RUNIN_DIR
testTime=0

if [ x"$1" == x"restart" ]; then
	rm -rf $CURRENT_LOOP_FILE
	rm -rf $CURRENT_REBOOT_FILE
	rm -rf $CURRENT_TIME_FILE
    echo 0 > $CURRENT_LOOP_FILE
	sync

	testTime=`cat $REBOOT_TIME_FILE`
	__runin_log "reboot test $testTime hours"
fi

testTime=`cat $REBOOT_TIME_FILE`

if [ ! -f "$CURRENT_LOOP_FILE" ]; then
    exit 0
fi

__runin_reboot_test $testTime

rm -rf $CURRENT_LOOP_FILE
rm -rf $CURRENT_REBOOT_FILE
rm -rf $CURRENT_TIME_FILE
sync

