#!/bin/sh

tt_base="/base/production/cameraisptester"

mkdir -p /mnt/extsd/bmp
/usr/bin/cameraisptester 0 1280 720 /mnt/extsd/bmp 3 5 1 30 0 0
result=$?
need_to_confirm_the_result=`mjson_fetch ${tt_base}/need_to_confirm_the_result`
if [ x${need_to_confirm_the_result} == xtrue ];then
        RED='\033[0;31m'
        NC='\033[0m'
        echo -e "${RED}Please check the bmp images under /mnt/extsd/bmp. You can use adb pull to get /mnt/extsd/bmp. [Y|n]${NC}"
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
