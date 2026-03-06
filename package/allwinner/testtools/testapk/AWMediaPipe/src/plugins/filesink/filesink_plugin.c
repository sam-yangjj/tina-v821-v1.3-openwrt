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
#include "filesink_plugin.h"
#include "../frame_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "../utils/log.h"

/**
 * @brief Private data structure for the file sink plugin
 *
 * Contains the file handle, thread management, and configuration
 * for writing H.264 data to files.
 */
typedef struct {
    FILE* file_handle;                      // File handle for output
    char* filename;                         // Output filename
    int running;                            // Thread running flag
    pthread_t thread;                       // Writer thread handle
    uint64_t frames_written;                // Counter for written frames
    uint64_t bytes_written;                 // Counter for written bytes
} FileSinkPluginPriv;

/**
 * @brief Main thread loop for file writing
 *
 * This function runs in a separate thread and continuously:
 * 1. Pulls encoded frames from upstream plugin
 * 2. Writes the H.264 data to the output file
 * 3. Updates statistics
 *
 * @param arg Pointer to the Plugin instance
 * @return NULL
 */
static void* filesink_thread_loop(void* arg) {
    Plugin* self = (Plugin*)arg;
    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)self->priv;

    printf("FileSink thread started, writing to: %s\n", priv->filename);
    int isFirst = 1;
    while (priv->running) {
        // Check if upstream plugin is available
        if (!self->upstream || !self->upstream->pull) {
            usleep(10 * 1000); // 10ms
            continue;
        }

        // Pull encoded frame from upstream plugin
        FrameBuffer* frame = self->upstream->pull(self->upstream);
        if (!frame) {
            usleep(10 * 1000); // 10ms
            continue;
        }
        frame_get(frame);
        if(!frame->data || frame->fd < 0)
        {
            continue;
        }

        LOG_INFO("FileSink received frame: size=%d bytes, PTS=%lu, format=%d\n",
               frame->size, frame->pts, frame->format);

        // Write frame data to file
        if (frame->data && frame->size > 0 && priv->file_handle) {
            if(isFirst && frame->format == PIXEL_FORMAT_H264 && frame->sps_pps_data)
            {
                fwrite(frame->sps_pps_data, 1, frame->sps_pps_data_len, priv->file_handle);
                isFirst = 0;
            }
            size_t written = fwrite(frame->data, 1, frame->size, priv->file_handle);
            if (written == frame->size) {
                // Flush to ensure data is written immediately
                fflush(priv->file_handle);

                // Update statistics
                priv->frames_written++;
                priv->bytes_written += written;

                LOG_DEBUG("Written frame %lu: %zu bytes (total: %lu frames, %lu bytes)",
                       priv->frames_written, written, priv->frames_written, priv->bytes_written);
            } else {
                LOG_DEBUG("Failed to write complete frame: wrote %zu of %d bytes",
                       written, frame->size);
            }
        } else {
            LOG_DEBUG("Invalid frame data or file handle");
        }

        // Release the frame
        frame_put(frame);
    }

    printf("FileSink thread stopped\n");
    return NULL;
}

/**
 * @brief Initialize the file sink plugin
 *
 * Opens the output file and prepares for writing.
 *
 * @param self Pointer to the plugin instance
 * @param config Configuration parameters (currently unused)
 * @return 0 on success, -1 on failure
 */
static int filesink_init(Plugin* self, void* config) {
    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)self->priv;

    printf("Initializing file sink plugin\n");

    if (!priv->filename) {
        printf("No filename specified for file sink\n");
        return -1;
    }

    // Open output file for writing
    priv->file_handle = fopen(priv->filename, "wb");
    if (!priv->file_handle) {
        printf("Failed to open file for writing: %s\n", priv->filename);
        perror("fopen");
        return -1;
    }

    printf("Opened output file: %s\n", priv->filename);

    // Initialize statistics
    priv->frames_written = 0;
    priv->bytes_written = 0;
    priv->running = 0;

    printf("File sink plugin initialized successfully\n");
    return 0;
}

/**
 * @brief Start the file sink plugin
 *
 * Creates and starts the writer thread.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success, -1 on failure
 */
static int filesink_start(Plugin* self) {
    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)self->priv;

    printf("Starting file sink plugin\n");
    priv->running = 1;

    int ret = pthread_create(&priv->thread, NULL, filesink_thread_loop, self);
    if (ret != 0) {
        printf("Failed to create file sink thread\n");
        priv->running = 0;
        return -1;
    }

    printf("File sink plugin started successfully\n");
    return 0;
}

/**
 * @brief Stop the file sink plugin
 *
 * Stops the writer thread and waits for it to complete.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success
 */
static int filesink_stop(Plugin* self) {
    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)self->priv;

    printf("Stopping file sink plugin\n");
    priv->running = 0;

    // Wait for thread to complete
    pthread_join(priv->thread, NULL);

    // Close file handle
    if (priv->file_handle) {
        fclose(priv->file_handle);
        priv->file_handle = NULL;
    }

    printf("File sink plugin stopped. Statistics:\n");
    printf("  Frames written: %lu\n", priv->frames_written);
    printf("  Bytes written: %lu\n", priv->bytes_written);
    printf("  Output file: %s\n", priv->filename);

    return 0;
}

/**
 * @brief Process function (not used in sink)
 *
 * File sink is a terminal node, so it doesn't process frames.
 *
 * @param self Pointer to the plugin instance
 * @param frame Input frame buffer
 * @return 0 (not used)
 */
static int filesink_process(Plugin* self, FrameBuffer* frame) {
    // Not used - file sink pulls from upstream
    return 0;
}

/**
 * @brief Push function (not used in sink)
 *
 * @param self Pointer to the plugin instance
 * @param frame Input frame buffer
 * @return 0 (not implemented)
 */
static int filesink_push(Plugin* self, FrameBuffer* frame) {
    // Not used - file sink pulls from upstream
    return 0;
}

/**
 * @brief Pull function (not used in sink)
 *
 * File sink is a terminal node, so it doesn't provide output frames.
 *
 * @param self Pointer to the plugin instance
 * @return NULL (no output)
 */
static FrameBuffer* filesink_pull(Plugin* self) {
    // File sink is a terminal node - no output
    return NULL;
}

/**
 * @brief Set output filename for the file sink
 *
 * @param self Pointer to the plugin instance
 * @param filename Path to the new output file
 * @return 0 on success, -1 on failure
 */
int filesink_set_filename(Plugin* self, const char* filename) {
    if (!self || !self->priv || !filename) {
        return -1;
    }

    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)self->priv;

    // Free old filename if exists
    if (priv->filename) {
        free(priv->filename);
    }

    // Copy new filename
    priv->filename = strdup(filename);
    if (!priv->filename) {
        printf("Failed to allocate memory for filename\n");
        return -1;
    }

    printf("Set file sink output to: %s\n", priv->filename);
    return 0;
}

/**
 * @brief Destroy the file sink plugin and free resources
 *
 * @param plugin Pointer to the plugin to destroy
 */
void destroy_filesink_plugin(Plugin* plugin) {
    if (!plugin || !plugin->priv) return;

    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)plugin->priv;

    printf("Destroying file sink plugin\n");

    // Close file if still open
    if (priv->file_handle) {
        fclose(priv->file_handle);
    }

    // Free filename
    if (priv->filename) {
        free(priv->filename);
    }

    // Free plugin name and private data
    if (plugin->name) {
        free((void*)plugin->name);
    }
    free(priv);
    free(plugin);

    printf("File sink plugin destroyed\n");
}

/**
 * @brief Create a new file sink plugin instance
 *
 * @param name Optional name for the plugin instance
 * @param filename Path to the output file where H.264 data will be written
 * @return Pointer to the created plugin, or NULL on failure
 */
Plugin* create_filesink_plugin(const char* name, const char* filename) {
    // Allocate plugin structure
    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    if (!plugin) {
        printf("Failed to allocate Plugin structure\n");
        return NULL;
    }

    // Allocate private data
    FileSinkPluginPriv* priv = (FileSinkPluginPriv*)calloc(1, sizeof(FileSinkPluginPriv));
    if (!priv) {
        printf("Failed to allocate FileSinkPluginPriv structure\n");
        free(plugin);
        return NULL;
    }

    // Set filename
    if (filename) {
        priv->filename = strdup(filename);
        if (!priv->filename) {
            printf("Failed to allocate memory for filename\n");
            free(priv);
            free(plugin);
            return NULL;
        }
    }

    // Set plugin properties
    plugin->name = name ? strdup(name) : strdup("FileSinkPlugin");
    plugin->init = filesink_init;
    plugin->start = filesink_start;
    plugin->stop = filesink_stop;
    plugin->process = filesink_process;
    plugin->push = filesink_push;
    plugin->pull = filesink_pull;
    plugin->priv = priv;

    printf("Created file sink plugin: %s (output: %s)\n",
           plugin->name, filename ? filename : "not set");
    return plugin;
}
