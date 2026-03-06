/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GDC_PLUGIN_H
#define GDC_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    USER_WARP_DEFAULT = 0,
    USER_WARP_WIDE,
    USER_WARP_ULTRA_WIDE,
    USER_WARP_PANORAMA_180,
    USER_WARP_PANORAMA_360,
    USER_WARP_TOP_DOWN,
    USER_WARP_PERSPECTIVE,
    USER_WARP_BIRDSEYE,
    USER_WARP_CUSTOM,
    USER_WARP_UNKNOWN
} UserWarpMode;


#define LUT_X_PATH  "/mnt/UDISK/test_x.txt"
#define LUT_Y_PATH  "/mnt/UDISK/test_y.txt"


#define ALIGN_XXB(y, x) (((x) + ((y)-1)) & ~((y)-1))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#include <stdint.h>

typedef struct {
    UserWarpMode warp_mode;      ///< 用户选择的矫正模式（映射到内部 eWarpType）

    int input_width;             ///< 输入图像宽度(默认和上游的输出图像一致)
    int input_height;            ///< 输入图像高度(默认和上游的输出图像一致)

    int output_width;            ///< 输出图像宽度（可以等于或不同于输入）
    int output_height;           ///< 输出图像高度

} GDCConfig;



/**
 * @brief Create a GDC plugin instance
 *
 * Creates and initializes a GDC (Geometry Distortion Correction) plugin
 * with the specified name. The plugin provides hardware-accelerated
 * geometric transformation capabilities for image correction.
 *
 * The plugin can correct various types of distortions commonly found in
 * camera systems, including lens distortion, perspective distortion,
 * and other geometric aberrations.
 *
 * @param name Plugin instance name (can be NULL for default "GdcPlugin")
 * @return Pointer to the created Plugin instance, or NULL on failure
 *
 * @note The plugin must be initialized and started before it can process frames.
 *       Default parameters are suitable for basic distortion correction, but
 *       can be customized through the configuration functions.
 */
Plugin* create_gdc_plugin(const char* name);


void destroy_gdc_plugin(Plugin* plugin);

#ifdef __cplusplus
}
#endif

#endif // GDC_PLUGIN_H
