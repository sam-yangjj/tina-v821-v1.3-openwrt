#!/bin/sh

# Play an audio file through the headphone.

target=`get_target`
path_in_config_tree="/base/production/headphonetester"
headphone_volume=`mjson_fetch $path_in_config_tree/headphone_volume`
audio_file=`mjson_fetch $path_in_config_tree/audio_file`

case "$target" in
    r40-*)
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='Headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
        amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
        amixer cset iface=MIXER,name='HPL Mux' 1
        amixer cset iface=MIXER,name='HPR Mux' 1
    ;;
    r16-*)
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
        amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
        amixer cset iface=MIXER,name='HP_L Mux' 0
        amixer cset iface=MIXER,name='HP_R Mux' 0
    ;;
    r6-*)
        echo "platform: r6"
        amixer cset iface=MIXER,name='head phone volume' $headphone_volume
    ;;
    v821-*)
        amixer cset iface=MIXER,name='LINEOUT Switch' 1
        amixer cset iface=MIXER,name='MIC Switch' 1
        amixer cset iface=MIXER,name='SPK Switch' 1
        amixer cset iface=MIXER,name='LINEOUT Volume' $headphone_volume
    ;;
    v861-*)
        amixer cset iface=MIXER,name='LINEOUTL Switch' 1
        amixer cset iface=MIXER,name='MIC1 Switch' 1
        amixer cset iface=MIXER,name='SPK Switch' 1
        amixer cset iface=MIXER,name='LINEOUT Volume' 7 #7 is maxnum
    ;;
    *)
        echo "This test case does not support current platform."
        exit 1
    ;;
esac
echo "start playing audio..."
aplay $audio_file -v

need_to_confirm_the_result=`mjson_fetch ${path_in_config_tree}/need_to_confirm_the_result`
if [ x${need_to_confirm_the_result} == xtrue ];then
	RED='\033[0;31m'
	NC='\033[0m'
	echo -e "${RED}Please confirm whether the speaker is playing normally? [Y|n]${NC}"
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
fi
