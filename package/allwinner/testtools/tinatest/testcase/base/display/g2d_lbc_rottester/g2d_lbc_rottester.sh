#!/bin/ash

md5_rotate_90=0b513a3ede1095a6a87e865cad4f6780
md5_rotate_180=b548169c60cbbefbf9fcee59350f58a9
md5_rotate_270=ebdc11a97d86d2efb9a2ba86cbcd2587
result=1

g2d_lbc_rot -flag 256 -in_fb 0 800 480 -src_rect 0 0 800 480 -out_fb 0 480 800 -dst_rect 0 0 480 800 -src_file /usr/lib/tt-data/src_800x480_rgb.bin -dst_file /tmp/en_src_800x480_rgb_90.bin
md5_tmp=`md5sum /tmp/en_src_800x480_rgb_90.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_rotate_90 == $md5_tmp ]
then
	echo -e "\n90 degree rotation Successful $md5_rotate_90和$md5_tmp相等\n"
else
	echo -e "\n90 degree rotation Failed $md5_rotate_90和$md5_tmp不相等\n"
	result=0
fi

g2d_lbc_rot -flag 512 -in_fb 0 800 480 -src_rect 0 0 800 480 -out_fb 0 800 480 -dst_rect 0 0 800 480 -src_file /usr/lib/tt-data/src_800x480_rgb.bin -dst_file /tmp/en_src_800x480_rgb_180.bin
md5_tmp=`md5sum /tmp/en_src_800x480_rgb_180.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_rotate_180 == $md5_tmp ]
then
	echo -e "\n180 degree rotation Successful $md5_rotate_180和$md5_tmp相等\n"
else
	echo -e "\n180 degree rotation Failed $md5_rotate_180和$md5_tmp不相等\n"
	result=0
fi

g2d_lbc_rot -flag 768 -in_fb 0 800 480 -src_rect 0 0 800 480 -out_fb 0 480 800 -dst_rect 0 0 480 800 -src_file /usr/lib/tt-data/src_800x480_rgb.bin -dst_file /tmp/en_src_800x480_rgb_270.bin
md5_tmp=`md5sum /tmp/en_src_800x480_rgb_270.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_rotate_270 == $md5_tmp ]
then
	echo -e "\n270 degree rotation Successful $md5_rotate_270和$md5_tmp相等\n"
else
	echo -e "\n270 degree rotation Failed $md5_rotate_270和$md5_tmp不相等\n"
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
