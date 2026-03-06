#ifndef __TUYA_YUV_UTILS_H__
#define __TUYA_YUV_UTILS_H__
#include <stdint.h>

//目前支持的格式
enum YUV_CONVERT_TYPE{
    YUV_CONVERT_TYPE_I420,
    YUV_CONVERT_TYPE_NV21,
    YUV_CONVERT_TYPE_NV12,
};

enum RGB_CONVERT_TYPE{
    RGB_CONVERT_TYPE_RGBA8888,
    RGB_CONVERT_TYPE_ARGB8888,    
    RGB_CONVERT_TYPE_RGB888,
};

int yuv_convert_frame_to_rgb(uint8_t* const ch_org,uint8_t* ch_out,
                                    int org_width,int org_height,int dst_width,int dst_height,
                                    enum RGB_CONVERT_TYPE dst_type) ;
//原始图像为I420
int yuv_scale_frame(uint8_t* const ch_org,uint8_t* ch_out,
                    int org_width,int org_height,int org_stride,
                    int dst_width,int dst_height,
                    enum YUV_CONVERT_TYPE dst_type);

int yuv_scale_frame_dis_continuous_mem(uint8_t* const ch_org_y,uint8_t* const ch_org_u,uint8_t* const ch_org_v,
                    uint8_t* ch_out,
                    int org_width,int org_height,int org_stride,
                    int dst_width,int dst_height,
                    enum YUV_CONVERT_TYPE dst_type);
//原始图像为NV21
int yuv_scale_frame_semi(uint8_t* const ch_org,uint8_t* ch_out,
                        int org_width,int org_height,int org_stride,
                        int dst_width,int dst_height,
                        enum YUV_CONVERT_TYPE dst_type);


#endif /* __TUYA_YUV_UTILS_H__ */
