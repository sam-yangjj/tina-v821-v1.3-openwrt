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
#ifndef G2D_PLUGIN_H
#define G2D_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new G2D plugin instance
 *
 * Creates and initializes a G2D plugin with the specified name.
 * The plugin provides hardware-accelerated 2D graphics operations.
 *
 * @param name Plugin instance name (can be NULL for default name)
 * @return Pointer to the created Plugin instance, or NULL on failure
 */
Plugin* create_g2d_plugin(const char* name);

/**
 * @brief Set output pixel format for G2D operations
 *
 * Configures the pixel format for the G2D output frames.
 * Supported formats include NV12, RGB888, ARGB8888, YUV420P, etc.
 *
 * @param self Pointer to the G2D plugin instance
 * @param fmt Target pixel format from PixelFormat enum
 * @return 0 on success, negative error code on failure
 */
int g2d_set_output_format(Plugin* self, PixelFormat fmt);

/**
 * @brief Set output frame dimensions for G2D operations
 *
 * Configures the width and height for the G2D output frames.
 * The G2D hardware will scale input frames to match these dimensions.
 *
 * @param self Pointer to the G2D plugin instance
 * @param width Target output width in pixels
 * @param height Target output height in pixels
 * @return 0 on success, negative error code on failure
 */
int g2d_set_output_size(Plugin* self, int width, int height);

/**
 * @brief Set crop region for G2D operations
 *
 * Defines a rectangular region in the input frame to be cropped
 * before applying other G2D operations like scaling or rotation.
 * Coordinates are in the input frame's coordinate system.
 *
 * @param self Pointer to the G2D plugin instance
 * @param crop_x X coordinate of the crop region's top-left corner
 * @param crop_y Y coordinate of the crop region's top-left corner
 * @param crop_width Width of the crop region in pixels
 * @param crop_height Height of the crop region in pixels
 * @return 0 on success, negative error code on failure
 */
int g2d_set_crop_region(Plugin* self, int crop_x, int crop_y, int crop_width, int crop_height);

/**
 * @brief Set rotation angle for G2D operations
 *
 * Configures clockwise rotation angle for the G2D output frames.
 * Only specific angles are supported by the hardware.
 *
 * @param self Pointer to the G2D plugin instance
 * @param angle Rotation angle in degrees (supported: 0, 90, 180, 270)
 * @return 0 on success, negative error code on failure
 */
int g2d_set_rotation(Plugin* self, int angle);

void destroy_g2d_plugin(Plugin* self);

#ifdef __cplusplus
}
#endif

#endif // G2D_PLUGIN_H
