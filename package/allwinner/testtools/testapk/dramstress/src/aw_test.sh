#!/bin/sh

USE_STRESSAPPTEST=1
AWTEST_DIR=/mnt/UDISK/stressapptest/.awtest
AWTEST_LOG=$AWTEST_DIR/log
RUNIN_DIR=/mnt/UDISK/stressapptest/.runin
REBOOT_TIME_FILE=$RUNIN_DIR/reboot_test_time
# STRESS_TIME=48
# STANDBY_TIME=24
VERSION="tina_1.0"

function __awtest_log()
{
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $@" | tee -a $AWTEST_LOG
}

function __get_test_time()
{
	local testTime=0
	echo -e "\n\n-------------------------------------"
	echo "Set Test Time (Unit Hours) :"
	echo "-------------------------------------"
    echo -e "\r<input> >>>>>>>>>> \c"
	read testTime
	# __runin_log "reboot test $testTime hours"
	if [ $testTime -le 0 -o $testTime -gt 255 ]; then
		echo "<error> testTime must belongs to (0,255] !"
		exit 0
	else
		return $testTime
	fi
}

function stressTest(){
	# memtester x 4
	memtester 10M > /dev/null &
	memtester 10M > /dev/null &
	memtester 10M > /dev/null &
	memtester 10M > /dev/null &
	__awtest_log "<info> running memtester"

	# display runtime bandwidth
	mtop -m &
	echo "<info> Stress Test Running."

	local i=0
	local val=0
	local startTime=$(date "+%Y-%m-%d %H:%M:%S")
	local currentTime=$(date "+%Y-%m-%d %H:%M:%S")
	local runTime=$(($(date +%s -d "${currentTime}")-$(date +%s -d "${startTime}")))
	local runTimeHour=0
	local runTimeMin=0
	local runTimeSec=0
	# local testTime=`echo "$1 * 60 * 60" | bc`
	local testTime=$(($1*60*60))
	local remainingTime=$(( ${testTime}-${runTime} ))
	local remainingTimeHour=0
	local remainingTimeMin=0
	local remainingTimeSec=0

	startTime=$(date "+%Y-%m-%d %H:%M:%S")
	currentTime=$(date "+%Y-%m-%d %H:%M:%S")

	__awtest_log "<info> stress test $1 hours."

	# Android module test pattern
	if [ $USE_STRESSAPPTEST -ge 1 ]; then
		stressapptest -s $testTime -C 4 -W -M 256 &
		__awtest_log "<info> running stressapptest"
	else
		# cmd-1
		glscale -c /vendor/etc/display/glscale-test-data/default.json -t 1280x720-8bit-rgba -m 1 -o 1 -p 300000 -f 600 -r /data/profile.txt &
		# cmd-2
		g2d-stress-test -delaytime 100 &
		# cmd-3
		hwcdebug --strategy 3 &
		DEBenchmark -c 4 &
		__awtest_log "<info> running patterns"
    fi

	while [ 1 -eq 1 ]; do
		i=$(busybox expr $i + 1)
		currentTime=$(date "+%Y-%m-%d %H:%M:%S")
		runTime=$(($(date +%s -d "${currentTime}")-$(date +%s -d "${startTime}")))
		runTimeHour=$(( $runTime/3600 ))
		runTimeMin=$(( ($runTime-${runTimeHour}*3600)/60 ))
		runTimeSec=$(( ($runTime-${runTimeHour}*3600-${runTimeMin}*60) ))

		remainingTime=$(( ${testTime}-${runTime} ))
		if [ $remainingTime -lt 0 ]; then
			remainingTime=0
		fi
		remainingTimeHour=$(( $remainingTime/3600 ))
		remainingTimeMin=$(( ($remainingTime-${remainingTimeHour}*3600)/60 ))
		remainingTimeSec=$(( ($remainingTime-${remainingTimeHour}*3600-${remainingTimeMin}*60) ))
		if [ $runTime -gt $testTime ]; then
			echo -e "\n\n"
			__awtest_log "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec"
			__awtest_log "<info> stress test done."
			echo -e "\n\n"
			# Android module test pattern
			if [ $USE_STRESSAPPTEST -ge 1 ]; then
				echo  "kill stressapptest"
				killall stressapptest
			else
				killall glscale
				killall g2d-stress-test
				killall hwcdebug
				killall DEBenchmark
			fi
			killall memtester
			killall mtop

			return
		fi

		# val=`echo "$i % 2" | bc`
		val=$(($i%2))
		if [ $val -eq 0 ]; then
			killall mtop
			echo "====================================="
			echo "<info> stress test running."
			echo "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec"
			echo "<info> remaining time : $remainingTimeHour:$remainingTimeMin:$remainingTimeSec"
			echo "====================================="
			cpu_monitor &
			sleep 10
			killall cpu_monitor
			mtop -m &
		else
			sleep 10
		fi

	done
}

function rebootTest(){
	echo $1 > $REBOOT_TIME_FILE
	# setprop debug.runin.restart 1
	runin.sh restart
	echo "<info> Reboot Test Running."
}

function standbyTest(){
	echo "<info> Standby Test Running."
	local i=0
	local startTime=$(date "+%Y-%m-%d %H:%M:%S")
	local currentTime=$(date "+%Y-%m-%d %H:%M:%S")
	local runTime=$(($(date +%s -d "${currentTime}")-$(date +%s -d "${startTime}")))
	local runTimeHour=0
	local runTimeMin=0
	local runTimeSec=0
	# local testTime=`echo "$1 * 60 * 60" | bc`
	local testTime=$(($1*60*60))
	# testTime=`echo "180" | bc`	# for test
	local remainingTime=$(( ${testTime}-${runTime} ))
	local remainingTimeHour=0
	local remainingTimeMin=0
	local remainingTimeSec=0

	startTime=$(date "+%Y-%m-%d %H:%M:%S")
	currentTime=$(date "+%Y-%m-%d %H:%M:%S")

	__awtest_log "<info> standby test $1 hours."


	while [ 1 -eq 1 ]; do
		i=$(busybox expr $i + 1)
		echo "====================================="
		echo "Running Standby Test"
		echo "Current Test: $i times"
		echo "====================================="
		sleep 1

		echo "+10" > /sys/class/rtc/rtc0/wakealarm
		echo mem > /sys/power/state
		sleep 10

		__awtest_log "Past total cycle test: $i times done."
		sync

		currentTime=$(date "+%Y-%m-%d %H:%M:%S")
		runTime=$(($(date +%s -d "${currentTime}")-$(date +%s -d "${startTime}")))
		runTimeHour=$(( $runTime/3600 ))
		runTimeMin=$(( ($runTime-${runTimeHour}*3600)/60 ))
		runTimeSec=$(( ($runTime-${runTimeHour}*3600-${runTimeMin}*60) ))

		remainingTime=$(( ${testTime}-${runTime} ))
		if [ $remainingTime -lt 0 ]; then
			remainingTime=0
		fi
		remainingTimeHour=$(( $remainingTime/3600 ))
		remainingTimeMin=$(( ($remainingTime-${remainingTimeHour}*3600)/60 ))
		remainingTimeSec=$(( ($remainingTime-${remainingTimeHour}*3600-${remainingTimeMin}*60) ))

		echo "====================================="
		echo "<info> standby test success: $i times."
		echo "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec"
		echo "<info> remaining time : $remainingTimeHour:$remainingTimeMin:$remainingTimeSec"
		echo "====================================="

		if [ $runTime -gt $testTime ]; then
			echo -e "\n\n"
			__awtest_log "<info> running time : $runTimeHour:$runTimeMin:$runTimeSec"
			__awtest_log "<info> standby test done."
			echo -e "\n\n"
			return
		fi

		# val=`echo "$i % 3" | bc`
		val=$(($i%3))
		if [ $val -eq 0 ]; then
			cpu_monitor &
			sleep 10
			killall cpu_monitor
		else
			sleep 10
		fi

	done
}

function main(){
	if [ ! -e ${AWTEST_DIR} ]; then
		mkdir -p ${AWTEST_DIR}
		mkdir -p ${RUNIN_DIR}
	fi
	if [ ! -e ${AWTEST_LOG} ]; then
		touch ${AWTEST_LOG}
	fi

	local testTime=0
	local test_item="null"
	echo -e "\n-----version $VERSION-----"
	echo -E '     __    __        __        __  '
	echo -E '    /\ \   \ \      /\ \      / /  '
	echo -E '   / /\ \   \ \    / /\ \    / /   '
	echo -E '  / /__\ \   \ \  / /  \ \  / /    '
	echo -E ' / /____\ \   \ \/ /    \ \/ /     '
	echo -E '/ /      \ \   \ \/      \ \/      '
	echo "-------------------------------------"
	echo "  AW DRAM Verification Tool          "
	echo "  awa2055 20240616                   "
	echo "-------------------------------------"

	echo -e "\nSelect Test Item :"
	echo "----------------------"
	echo "--- 0 : Stress Test"
	echo "--- 1 : Reboot Test"
	echo "--- 2 : Standby Test"
	echo "----------------------"
	echo "--- Q : exit"
	echo -e "\r<input> >>>>>>>>>> \c"
	read -t 10 test_item
	# echo "[debug] test_item : $test_item"

	# Run test function
	case "$test_item" in
		"null") echo "<info> input time out ..."
		return
		;;
		"0") __awtest_log "<info> Stress Test Starting."
		;;
		"1") __awtest_log "<info> Reboot Test Starting."
		;;
		"2") __awtest_log "<info> Standby Test Starting."
		;;
		"Q"|"q") echo "<info> test exit ..."
		return
		;;
		*) echo "<error> illegal input! Press any key to continue ..."
		read
		main
		;;
	esac

	__get_test_time
	testTime=$?

	# Get boot_complete state
	# local n=0
	# # ret_val=0
	# while [ $n -lt 60 ]; do
		# # echo "[debug] n : $n"
		# local ret_val=$(getprop sys.boot_completed)
		# # local reg_val=`echo ${ret_val}`
		# # local reg_val="1"
		# # echo "[debug] ret_val : $ret_val"
		# # echo "[debug] reg_val : $reg_val"
		# if [ "$ret_val" = "1" ]; then
			# # su
			# echo test > /sys/power/wake_lock
			# echo "<info> check sys.boot_completed state done."
			# break
		# else
			# echo -e "\r<info> waiting sys.boot_completed."
		# fi
		# let "n++"
		# if [ $n -ge 60 ]; then
			# echo "<error> dev.bootcomplete state error !"
			# return
		# fi
		# sleep 3
	# done

	# Run test function
	case "$test_item" in
		"0") stressTest $testTime
		;;
		"1") rebootTest $testTime
		;;
		"2") standbyTest $testTime
		;;
		*) echo "<error> some exception occur ..."
		read
		main
		;;
	esac

}

# Main function entry
main


