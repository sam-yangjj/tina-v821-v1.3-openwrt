#ifndef __TUYA_JPEG_UTILS_H__
#define __TUYA_JPEG_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct __TUYA_JPEG_HEADER
{
    int width;
    int height;
    int num_channels;
} TUYA_JPEG_HEADER_T;

/*********************************************************************************
* nv21图像编码成jpeg
* src_yuv           yuv内存
* width             图像宽
* width             图像高
* quality           编码质量 0-100
* dst_jpeg          输出jpeg内存
* dst_len           输出jpeg内存长度
**********************************************************************************/
int tuya_jpeg_encode_from_nv21(unsigned char *src_yuv, int width, int height, int quality, unsigned char **dst_jpeg, int *dst_len);

/*********************************************************************************
* nv12图像编码成jpeg
* src_yuv           yuv内存
* width             图像宽
* width             图像高
* quality           编码质量 0-100
* dst_jpeg          输出jpeg内存
* dst_len           输出jpeg内存长度
**********************************************************************************/
int tuya_jpeg_encode_from_nv12(unsigned char *src_yuv, int width, int height, int quality, unsigned char **dst_jpeg, int *dst_len);

/*********************************************************************************
* bgr图像编码成jpeg
* src_bgr           bgr内存
* width             图像宽
* width             图像高
* quality           编码质量 0-100
* dst_jpeg          输出jpeg内存
* dst_len           输出jpeg内存长度
**********************************************************************************/
int tuya_jpeg_encode_from_bgr(unsigned char *src_bgr, int width, int height, int quality, unsigned char **dst_jpeg, int *dst_len);

/*********************************************************************************
* rgb图像编码成jpeg
* src_rgb           rgb内存
* width             图像宽
* width             图像高
* quality           编码质量 0-100
* dst_jpeg          输出jpeg内存
* dst_len           输出jpeg内存长度
**********************************************************************************/
int tuya_jpeg_encode_from_rgb(unsigned char *src_rgb, int width, int height, int quality, unsigned char **dst_jpeg, int *dst_len);


/*********************************************************************************
* jpeg解码到nv21
* src_jpeg          输入jpeg内存图像
* src_len           jpeg大小
* width             输出图像高
* height            输出图像宽
* dst_yuv           输出nv21内存
**********************************************************************************/
int tuya_jpeg_decode_to_nv21(unsigned char *src_jpeg, int src_len ,int *width, int *height,  unsigned char **dst_yuv);


/*********************************************************************************
* jpeg解码到bgr

* src_jpeg          输入jpeg内存图像
* src_len           jpeg大小
* width             输出图像高
* height            输出图像宽
* dst_bgr           输出bgr内存，1)如果*dst_bgr为NULL，内存在接口内部申请，用户使用完释放，否则存在内存泄漏;2)如果*dst_bgr不为NULL，用户需要保证*dst_len足够大，否则会由于空间不足导致失败;
* dst_len           bgr内存长度,配合*dst_bgr使用，返回实际的bgr长度
**********************************************************************************/
int tuya_jpeg_decode_to_bgr(unsigned char *src_jpeg, int src_len, int *width, int *height, unsigned char **dst_bgr, int *dst_len);


/*********************************************************************************
* jpeg解码到rgb

* src_jpeg          输入jpeg内存图像
* src_len           jpeg大小
* width             输出图像高
* height            输出图像宽
* dst_bgr           输出rgb内存，1)如果*dst_bgr为NULL，内存在接口内部申请，用户使用完释放，否则存在内存泄漏;2)如果*dst_rgb不为NULL，用户需要保证*dst_len足够大，否则会由于空间不足导致失败;
* dst_len           rgb内存长度,配合*dst_rgb使用，返回实际的bgr长度
**********************************************************************************/
int tuya_jpeg_decode_to_rgb(unsigned char *src_jpeg, int src_len, int *width, int *height, unsigned char **dst_rgb, int *dst_len);


/*********************************************************************************
* jpeg解码，tuya_jpeg_decode_to_rgb 拆解为3个函数，可以逐行读取rgb数据，节省内存
* 调用顺序：tuya_jpeg_decode_open ->tuya_jpeg_decode_read->tuya_jpeg_decode_close
* src_jpeg          输入jpeg内存图像
* src_len           jpeg大小
* width             输出图像高
* height            输出图像宽
* num_channels      输出图像位宽，通常为3，代表24位 
**********************************************************************************/
int tuya_jpeg_decode_open(unsigned char *src_jpeg, int src_len, int *width, int *height, int *num_channels);

/*********************************************************************************
* 读取jpeg解码数据 
* [in]dst_bgr           输出rgb内存，*dst_rgb不可为NULL，用户需要保证分配的内存足够大，否则会由于空间不足导致失败;
* [in]buf_len           dst_bgr 分配的长度
* [out]out_scanline     当前解码的行号
* [out]dst_len          输出的解码长度
**********************************************************************************/
int tuya_jpeg_decode_read(unsigned char **dst_rgb, int buf_len, int *out_scanline, int *dst_len);

/*********************************************************************************
* 关闭jpeg解码
**********************************************************************************/
int tuya_jpeg_decode_close();

/*********************************************************************************
* 仅解jpeg头信息, 独立调用
**********************************************************************************/
int tuya_jpeg_decode_head_info(unsigned char *src_jpeg, int src_len, TUYA_JPEG_HEADER_T *header);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

