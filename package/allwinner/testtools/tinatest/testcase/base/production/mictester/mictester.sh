#!/bin/sh

# Record via the microphone and then play the record through the headphone.

target=`get_target`
path_in_config_tree="/base/production/mictester"
record_duration_sec=`mjson_fetch $path_in_config_tree/record_duration_sec`
record_audio_path=`mjson_fetch $path_in_config_tree/record_audio_path`
headphone_volume=`mjson_fetch $path_in_config_tree/headphone_volume`

case $target in
    r40-*)
        # record
        amixer cset iface=MIXER,name='Left Input Mixer MIC1 Boost Switch' 1
        amixer cset iface=MIXER,name='Right Input Mixer MIC1 Boost Switch' 1
        # play
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='Headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
        amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
        amixer cset iface=MIXER,name='HPL Mux' 1
        amixer cset iface=MIXER,name='HPR Mux' 1
    ;;
    r16-*)
        # record
        amixer cset iface=MIXER,name='ADC input gain' 3
        amixer cset iface=MIXER,name='ADC volume' 180
        amixer cset iface=MIXER,name='MIC1 boost amplifier gain' 5
        amixer cset iface=MIXER,name='AIF1 AD0L Mixer ADCL Switch' 1
        amixer cset iface=MIXER,name='AIF1 AD0R Mixer ADCR Switch' 1
        amixer cset iface=MIXER,name='LEFT ADC input Mixer MIC1 boost Switch' 1
        amixer cset iface=MIXER,name='RIGHT ADC input Mixer MIC1 boost Switch' 1
        # play
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
        amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
        amixer cset iface=MIXER,name='HP_L Mux' 0
        amixer cset iface=MIXER,name='HP_R Mux' 0
    ;;
    r6-*)
        #record
        amixer cset iface=MIXER,name='ADC MIC Boost AMP en' on
        amixer cset iface=MIXER,name='ADC mixer mute for mic' on
        #play
        amixer cset iface=MIXER,name='head phone volume' $headphone_volume
    ;;
    v821-*)
        amixer cset iface=MIXER,name='LINEOUT Switch' 1
        amixer cset iface=MIXER,name='MIC Switch' 1
        amixer cset iface=MIXER,name='SPK Switch' 1
        amixer cset iface=MIXER,name='MIC Gain' $headphone_volume
        amixer cset iface=MIXER,name='LINEOUT Gain' $headphone_volume
    ;;
    v861-*)
        amixer cset iface=MIXER,name='LINEOUTL Switch' 1
        amixer cset iface=MIXER,name='MIC1 Switch' 1
        amixer cset iface=MIXER,name='SPK Switch' 1
        amixer cset iface=MIXER,name='ADC1 Gain' 14 #14 is maxnum
        amixer cset iface=MIXER,name='LINEOUT Volume' 7 #7 is maxnum
    ;;
    *)
        echo "This test case does not support current platform."
        exit 1
    ;;
esac

mkdir -p `dirname $record_audio_path`
arecord -v -d $record_duration_sec -f S16_LE ${record_audio_path}
if [ $? -ne "0" ]; then
    echo "Record failed!"
    exit 1
fi
aplay $record_audio_path -v
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
