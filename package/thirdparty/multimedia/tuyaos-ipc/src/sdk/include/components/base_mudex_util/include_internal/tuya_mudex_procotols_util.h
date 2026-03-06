
#ifndef __TUYA_MUDEX_PROCOTOLS_UTIL_H__
#define __TUYA_MUDEX_PROCOTOLS_UTIL_H__

#include "tuya_mudex_io_buffer.h"
#include "tuya_mudex_log_util.h"
#include "tuya_mudex_io_util.h"

typedef struct URLContext {
    const struct _mudex_stream *prot;
    void *priv_data;
    char *filename;             /**< specified URL */
    int flags;
    int max_packet_size;        /**< if non zero, the stream is packetized with this max packet size */
    int is_streamed;            /**< true if streamed (no seek possible), default = false */
    int is_connected;
    AVIOInterruptCB interrupt_callback;
    // const char *protocol_whitelist;
    // const char *protocol_blacklist;
    int min_packet_size;        /**< if non zero, the stream is packetized with this min packet size */
} URLContext;

int tuya_url_alloc(URLContext **puc, const char *filename, int flags,
                const AVIOInterruptCB *int_cb,enum TUYA_MUDEX_IO_STREAM_TYPE mode);
/**
 * Connect an URLContext that has been allocated by ffurl_alloc
 *
 * @param options  A dictionary filled with options for nested protocols,
 * i.e. it will be passed to url_open2() for protocols implementing it.
 * This parameter will be destroyed and replaced with a dict containing options
 * that were not found. May be NULL.
 */
int tuya_url_connect(URLContext *uc);

int tuya_muxer_url_open(URLContext **puc, const char *filename, int flags,
                         const AVIOInterruptCB *int_cb,enum TUYA_MUDEX_IO_STREAM_TYPE mode);



/**
 * Read up to size bytes from the resource accessed by h, and store
 * the read bytes in buf.
 *
 * @return The number of bytes actually read, or a negative value
 * corresponding to an AVERROR code in case of error. A value of zero
 * indicates that it is not possible to read more from the accessed
 * resource (except if the value of the size argument is also zero).
 */
int tuya_url_read(URLContext *h, unsigned char *buf, int size);



/**
 * Write size bytes from buf to the resource accessed by h.
 *
 * @return the number of bytes actually written, or a negative value
 * corresponding to an AVERROR code in case of failure
 */
int tuya_url_write(URLContext *h, const unsigned char *buf, int size);
/**
 * Change the position that will be used by the next read/write
 * operation on the resource accessed by h.
 *
 * @param pos specifies the new position to set
 * @param whence specifies how pos should be interpreted, it must be
 * one of SEEK_SET (seek from the beginning), SEEK_CUR (seek from the
 * current position), SEEK_END (seek from the end), or AVSEEK_SIZE
 * (return the filesize of the requested resource, pos is ignored).
 * @return a negative value corresponding to an AVERROR code in case
 * of failure, or the resulting file position, measured in bytes from
 * the beginning of the file. You can use this feature together with
 * SEEK_CUR to read the current file position.
 */
int64_t tuya_url_seek(URLContext *h, int64_t pos, int whence);
/**
 * Close the resource accessed by the URLContext h, and free the
 * memory used by it. Also set the URLContext pointer to NULL.
 *
 * @return a negative value if an error condition occurred, 0
 * otherwise
 */
int tuya_url_closep(URLContext **h);
int tuya_url_close(URLContext *h);

/**
 * Check if the user has requested to interrupt a blocking function
 * associated with cb.
 */
int ff_check_interrupt(AVIOInterruptCB *cb);

//根据procotl_mode的模式，进行参数的配置
int tym_io_param_setting(URLContext *h,unsigned char *base,int size,int procotl_mode);


#endif /* AVFORMAT_URL_H */
