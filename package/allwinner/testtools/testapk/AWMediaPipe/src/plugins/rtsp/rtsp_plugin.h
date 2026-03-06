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
#ifndef RTSP_PLUGIN_H
#define RTSP_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create an RTSP streaming server plugin instance
 *
 * Creates and initializes an RTSP plugin that provides video streaming
 * capabilities using the RTSP/RTP protocol. The plugin acts as a sink
 * in the processing pipeline, consuming H.264 encoded frames and serving
 * them to network clients.
 *
 * The plugin creates a multi-threaded RTSP server that can handle multiple
 * concurrent client connections. It automatically manages the H.264 frame
 * buffer and RTP packet transmission for optimal streaming performance.
 *
 * @param name Plugin instance name (can be NULL for default "RtspPlugin")
 * @param ip Server IP address to bind to ("0.0.0.0" for all interfaces,
 *           can be NULL for default "0.0.0.0")
 * @param port Server port number (standard RTSP port is 554,
 *             use 0 or negative for default 554)
 * @return Pointer to the created Plugin instance, or NULL on failure
 *
 * @note The plugin must be initialized and started before it can accept
 *       client connections. Ensure the specified port is not already in use.
 *
 * @example
 * ```c
 * Plugin* rtsp = create_rtsp_plugin("RTSP", "0.0.0.0", 554);
 * rtsp->upstream = encoder_plugin;  // Connect to H.264 encoder
 * rtsp->init(rtsp, NULL);
 * rtsp->start(rtsp);
 * // Clients can now connect to: rtsp://server_ip:554/live
 * ```
 */
Plugin* create_rtsp_plugin(const char* name, const char* ip, int port, int fps);

/**
 * @brief Set RTSP server network address configuration
 *
 * Updates the IP address and port number for the RTSP server.
 * This function can be called before plugin initialization to change
 * the default server binding configuration.
 *
 * @param self Pointer to the RTSP plugin instance
 * @param ip Server IP address to bind to ("0.0.0.0" for all interfaces)
 * @param port Server port number (standard RTSP port is 554)
 * @return 0 on success, negative error code on failure
 *
 * @note This function should be called before plugin initialization.
 *       Changing the address after the server is started requires
 *       stopping and restarting the plugin.
 */
int rtsp_set_server_address(Plugin* self, const char* ip, int port);

#ifdef __cplusplus
}
#endif

#endif // RTSP_PLUGIN_H
