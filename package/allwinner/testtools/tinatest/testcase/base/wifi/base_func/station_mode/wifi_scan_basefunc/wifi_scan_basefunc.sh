#!/bin/sh

target=`get_target`

wifi_scan()
{
    echo "====== start wifi_scan_results_test ======"
    wifi_scan_results_test
    if [ $? -eq 0 ];then
	echo "====== wifi_scan_results_test successed!!! ======"
	exit 0
    else
	echo "====== wifi_scan_results_test failed!!! ======"
	exit 1
    fi
}

wifi_scan_wifimanager_v2_0()
{
    echo "====== start wifi_scan_results_test ======"
    wifi -s
    if [ $? -eq 0 ];then
	echo "====== wifi_scan_results_test successed!!! ======"
	exit 0
    else
	echo "====== wifi_scan_results_test failed!!! ======"
	exit 1
    fi

}

case "$target" in
    v861-*)
        wifi_scan_wifimanager_v2_0
    ;;
    *)
        wifi_scan
    ;;
esac
