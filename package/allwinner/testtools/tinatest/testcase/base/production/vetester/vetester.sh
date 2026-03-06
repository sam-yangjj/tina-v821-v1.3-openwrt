#!/bin/sh

tt_base="/base/production/vetester"

mkdir -p /tmp/bmp
/usr/bin/vetester
result=$?
need_to_confirm_the_result=`mjson_fetch ${tt_base}/need_to_confirm_the_result`
if [ x${need_to_confirm_the_result} == xtrue ];then
        RED='\033[0;31m'
        NC='\033[0m'
        echo -e "${RED}Please check the vetester.raw under /tmp. You can use adb pull to get it. [Y|n]${NC}"
        read -r answer
        case "$answer" in
        Y|y)
                exit 0  # 正常返回
        ;;
        N|n)
                exit 1  # 异常返回
        ;;
        *)
                exit 2  # 输入错误
        ;;
        esac
else
	exit ${result}
fi
