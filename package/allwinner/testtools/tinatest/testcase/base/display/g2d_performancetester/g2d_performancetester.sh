#!/bin/ash

result=1

g2d_performance -flag 256 -in_fb 42 4096 2160 -src_rect 0 0 4096 2160 -out_fb 42 2160 4096 -dst_rect 0 0 2160 4096 -compare 0 -src_file /usr/lib/tt-data/I420_4096x2160_coast.bin -dst_file /tmp/I420_4096x2160_coast_90.bin -vs_file /usr/lib/tt-data/I420_4096x2160_coast_90.bin

#g2d_performance -flag 256 -in_fb 40 1920 1080 -src_rect 0 0 1920 1080 -out_fb 40 1080 1920 -dst_rect 0 0 1080 1920 -compare 0 -src_file /usr/lib/tt-data/bike_1920x1080_220.bin -dst_file /tmp/bike_1920x1080_220_90.bin -vs_file /usr/lib/tt-data/bike_1920x1080_220_90.bin

if [ $result == 1 ]
then
	echo -e "test passed"
	return 0
else
	echo -e "test failed"
	return 1
fi
