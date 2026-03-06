#!/bin/ash

md5_fill_b=cea2a8ae98abcc638813f0fe74c7f9b4
md5_fill_g=7b512d41dce9eb3bfa039b0816fc123c
md5_fill_r=6de319f12bb67b49ec9523590bd9b0d9
result=1

g2d_fill_dmabuf -out_fb 0 800 480 -dst_rect 0 0 800 480 -out_file /tmp/fill_dmabuf_b.bin -color 4278190335 -alpha_mode 1 -alpha 255
md5_tmp=`md5sum /tmp/fill_dmabuf_b.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_fill_b == $md5_tmp ]
then
	echo -e "\nfill blue Successful $md5_fill_b和$md5_tmp相等\n"
else
	echo -e "\nfill blue Failed $md5_fill_b和$md5_tmp不相等\n"
	result=0
fi

g2d_fill_dmabuf -out_fb 0 800 480 -dst_rect 0 0 800 480 -out_file /tmp/fill_dmabuf_g.bin -color 4278255360‬ -alpha_mode 1 -alpha 255
md5_tmp=`md5sum /tmp/fill_dmabuf_g.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_fill_g == $md5_tmp ]
then
	echo -e "\nfill green Successful $md5_fill_g和$md5_tmp相等\n"
else
	echo -e "\nfill green Failed $md5_fill_g和$md5_tmp不相等\n"
	result=0
fi

g2d_fill_dmabuf -out_fb 0 800 480 -dst_rect 0 0 800 480 -out_file /tmp/fill_dmabuf_r.bin -color 4294901760 -alpha_mode 1 -alpha 255
md5_tmp=`md5sum /tmp/fill_dmabuf_r.bin`
md5_tmp=${md5_tmp%% *}

if [ $md5_fill_r == $md5_tmp ]
then
	echo -e "\nfill read Successful $md5_fill_r和$md5_tmp相等\n"
else
	echo -e "\nfill read Failed $md5_fill_r和$md5_tmp不相等\n"
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