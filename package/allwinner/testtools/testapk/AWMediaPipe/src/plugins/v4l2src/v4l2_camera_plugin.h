/**
 * @file v4l2_camera_plugin.h
 * @brief V4L2 Camera Source Plugin Interface Header
 *
 * This header defines the interface for the V4L2 camera plugin, which provides
 * video capture capabilities using the Video4Linux2 (V4L2) API.
 *
 * The plugin captures frames from camera devices and outputs them as FrameBuffer
 * objects for downstream processing. It supports various pixel formats and
 * resolutions, with zero-copy DMA buffer operations for optimal performance.
 *
 * @author kezhijies
 * @date 2025
 */

#ifndef V4L2_CAMERA_PLUGIN_H
#define V4L2_CAMERA_PLUGIN_H

#include "../plugin_interface.h"

typedef struct {
    int camea_index;
    int width;
    int height;
    int fps;
    PixelFormat pixel_format;
    int useIsp;
} V4L2CameraConfig;

/**
 * @brief Create a V4L2 camera plugin instance
 *
 * Creates and initializes a V4L2 camera plugin that can capture video
 * frames from camera devices using the Video4Linux2 API.
 *
 * The plugin automatically detects available camera devices and configures
 * them for optimal performance with DMA buffer support.
 *
 * @return Pointer to the created Plugin instance, or NULL on failure
 *
 * @note The caller is responsible for calling the plugin's init() method
 *       before starting frame capture.
 */
Plugin* create_v4l2_camera_plugin(const char* name);


void destroy_v4l2_camera_plugin(Plugin* plugin);
//FrameBuffer* v4l2_camera_pull_frame(Plugin* plugin);

#endif // V4L2_CAMERA_PLUGIN_H
