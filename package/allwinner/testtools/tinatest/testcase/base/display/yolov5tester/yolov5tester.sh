#!/bin/ash

md5_yolov5=e00cc3d04aafc9500fdb68e5eb93b0ea
result=1

cd /tmp
yolov5 /etc/npu/yolov5/model/yolov5.nb /etc/npu/yolov5/input_data/dog_640_640.jpg
md5_tmp=`md5sum /tmp/result.png`
md5_tmp=${md5_tmp%% *}

if [ $md5_yolov5 == $md5_tmp ]
then
	echo -e "\nfill blue Successful $md5_yolov5和$md5_tmp相等\n"
else
	echo -e "\nfill blue Failed $md5_yolov5和$md5_tmp不相等\n"
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
