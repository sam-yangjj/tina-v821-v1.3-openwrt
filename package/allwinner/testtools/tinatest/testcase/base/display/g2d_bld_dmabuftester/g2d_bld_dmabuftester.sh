#!/bin/ash

md5_bld=93f1a74caade2d72337a4eefbecd725d
result=1

g2d_bld_dmabuf -flag 4 -in_fb 0 800 480 -src_rect 0 0 800 480 -src_coor 0 0 -in_fb2 0 800 480 -src_rect2 0 0 800 480 -src_coor2 0 0 -out_fb 0 800 480 -dst_rect 0 0 800 480 -src_file /usr/lib/tt-data/src_800x480_rgb.bin -src_file2 /usr/lib/tt-data/src2_800x480_rgb.bin -dst_file /tmp/bld_test.bin -alpha_mode 1 -alpha 208

md5_tmp=`md5sum /tmp/bld_test.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_bld == $md5_tmp ]
then
	echo -e "\nblend Successful $md5_bld和$md5_tmp相等\n"
else
	echo -e "\nblend Failed $md5_bld和$md5_tmp不相等\n"
	result=0
fi

if [ $result == 1 ]
then
	echo -e "test passed"
	return 0
else
	echo -e "test failed"
	return 1
fi