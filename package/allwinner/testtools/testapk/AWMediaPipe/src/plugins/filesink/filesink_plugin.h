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
#ifndef FILESINK_PLUGIN_H
#define FILESINK_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a file sink plugin instance
 *
 * Creates a new file sink plugin that can write H.264 encoded frames to a file.
 * The plugin follows the standard MediaPipeX plugin interface and acts as a sink.
 *
 * @param name Optional name for the plugin instance (can be NULL)
 * @param filename Path to the output file where H.264 data will be written
 * @return Pointer to the created plugin, or NULL on failure
 */
Plugin* create_filesink_plugin(const char* name, const char* filename);

/**
 * @brief Set output filename for the file sink
 *
 * Changes the output file for the file sink plugin. The plugin must be stopped
 * before calling this function.
 *
 * @param self Pointer to the plugin instance
 * @param filename Path to the new output file
 * @return 0 on success, -1 on failure
 */
int filesink_set_filename(Plugin* self, const char* filename);

/**
 * @brief Destroy file sink plugin
 *
 * Properly destroys the file sink plugin and frees all resources.
 * This function should be called externally since the Plugin interface
 * doesn't include a destroy function pointer.
 *
 * @param plugin Pointer to the plugin to destroy
 */
void destroy_filesink_plugin(Plugin* plugin);

#ifdef __cplusplus
}
#endif

#endif // FILESINK_PLUGIN_H
