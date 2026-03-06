#ifndef __TUYA_RGB_UTILS_H__
#define __TUYA_RGB_UTILS_H__
#include <stdint.h>


typedef enum {   
    TUYA_PIXEL_FORMAT_TYPE_I420 = 0,
	TUYA_PIXEL_FORMAT_TYPE_NV21,
	TUYA_PIXEL_FORMAT_TYPE_NV12,
	
	TUYA_PIXEL_FORMAT_TYPE_RGB888,
	TUYA_PIXEL_FORMAT_TYPE_RGBA8888,
	TUYA_PIXEL_FORMAT_TYPE_ARGB8888,

	
    TUYA_PIXEL_FORMAT_TYPE_BUTT   
} TUYA_PIXEL_FORMAT_TYPE_E;
	
typedef struct {
	int x;
	int y;
	int width;
	int height;
} TUYA_RECT_UTILS_T;

typedef struct {
	TUYA_PIXEL_FORMAT_TYPE_E src_pixel;	
	TUYA_PIXEL_FORMAT_TYPE_E dst_pixel;
	int src_width;
	int src_height;
	int src_stride;
	int dst_width;
	int dst_height;
	int dst_stride;
	TUYA_RECT_UTILS_T crop;
	TUYA_RECT_UTILS_T clip;
	int src_len;
	void *src_image;
	int dst_len;
	void *dst_image;
} TUYA_CONVERT_IMAGE_INFO_T;


int tuya_rgb_convert(TUYA_CONVERT_IMAGE_INFO_T *image);


#endif /* __TUYA_RGB_UTILS_H__ */
