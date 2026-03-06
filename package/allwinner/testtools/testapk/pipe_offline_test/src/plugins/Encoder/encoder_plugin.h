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
#ifndef ENCODER_PLUGIN_H
#define ENCODER_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int input_width;
    int input_height;
    int input_fps;
    PixelFormat input_format;
} encConfig;

/**
 * @brief Create an H.264 encoder plugin instance
 *
 * Creates and initializes an H.264 video encoder plugin with the specified name.
 * The plugin provides hardware-accelerated video encoding capabilities and
 * integrates with the MediaPipeX plugin framework.
 *
 * The encoder supports multiple input formats and can be configured for
 * different bitrates, frame rates, and quality levels. It uses the underlying
 * H264Encoder implementation for actual encoding operations.
 *
 * @param name Plugin instance name (can be NULL for default "EncoderPlugin")
 * @return Pointer to the created Plugin instance, or NULL on failure
 *
 * @note The plugin must be initialized and started before it can process frames.
 *       Default encoding parameters are set during initialization but can be
 *       modified through configuration interfaces.
 */
Plugin* create_encoder_plugin(const char* name);

/**
 * @brief Set encoder bitrate (future extension)
 *
 * This function is reserved for future implementation to allow dynamic
 * bitrate configuration of the encoder plugin.
 *
 * @param self Pointer to the encoder plugin instance
 * @param bitrate Target bitrate in bits per second
 * @return 0 on success, negative error code on failure
 */
// int encoder_set_bitrate(Plugin* self, int bitrate);

/**
 * @brief Set encoder frame rate (future extension)
 *
 * This function is reserved for future implementation to allow dynamic
 * frame rate configuration of the encoder plugin.
 *
 * @param self Pointer to the encoder plugin instance
 * @param fps Target frame rate in frames per second
 * @return 0 on success, negative error code on failure
 */
// int encoder_set_framerate(Plugin* self, int fps);

int get_encoder_parameter(Plugin* self, unsigned char** data);

void destroy_encoder_plugin(Plugin* self);

#ifdef __cplusplus
}
#endif

#endif // ENCODER_PLUGIN_H
