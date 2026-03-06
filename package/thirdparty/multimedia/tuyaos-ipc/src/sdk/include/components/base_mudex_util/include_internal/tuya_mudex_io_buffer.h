
#ifndef __TUYA_MUDEX_IO_BUFFER_H__
#define __TUYA_MUDEX_IO_BUFFER_H__

/**
 * @file
 * @ingroup lavf_io
 * Buffered I/O operations
 */
#include <stdint.h>

#include "tuya_mudex_util_common.h"
#include "tuya_mudex_log_util.h"


/**
 * Seeking works like for a local file.
 */
#define AVIO_SEEKABLE_NORMAL (1 << 0)

/**
 * Seeking by timestamp with avio_seek_time() is possible.
 */
#define AVIO_SEEKABLE_TIME   (1 << 1)


typedef struct AVIOInterruptCB {
    int (*callback)(void*);
    void *opaque;
} AVIOInterruptCB;


/**
 * Different data types that can be returned via the AVIO
 * write_data_type callback.
 */
enum AVIODataMarkerType {
    /**
     * Header data; this needs to be present for the stream to be decodeable.
     */
    AVIO_DATA_MARKER_HEADER,
    /**
     * A point in the output bytestream where a decoder can start decoding
     * (i.e. a keyframe). A demuxer/decoder given the data flagged with
     * AVIO_DATA_MARKER_HEADER, followed by any AVIO_DATA_MARKER_SYNC_POINT,
     * should give decodeable results.
     */
    AVIO_DATA_MARKER_SYNC_POINT,
    /**
     * A point in the output bytestream where a demuxer can start parsing
     * (for non self synchronizing bytestream formats). That is, any
     * non-keyframe packet start point.
     */
    AVIO_DATA_MARKER_BOUNDARY_POINT,
    /**
     * This is any, unlabelled data. It can either be a muxer not marking
     * any positions at all, it can be an actual boundary/sync point
     * that the muxer chooses not to mark, or a later part of a packet/fragment
     * that is cut into multiple write callbacks due to limited IO buffer size.
     */
    AVIO_DATA_MARKER_UNKNOWN,
    /**
     * Trailer data, which doesn't contain actual content, but only for
     * finalizing the output file.
     */
    AVIO_DATA_MARKER_TRAILER,
    /**
     * A point in the output bytestream where the underlying TYM_IO_CONTEXT might
     * flush the buffer depending on latency or buffering requirements. Typically
     * means the end of a packet.
     */
    AVIO_DATA_MARKER_FLUSH_POINT,
};

typedef struct TYM_IO_CONTEXT {

    /*
     * The following shows the relationship between buffer, buf_ptr,
     * buf_ptr_max, buf_end, buf_size, and pos, when reading and when writing
     * (since TYM_IO_CONTEXT is used for both):
     *
     **********************************************************************************
     *                                   READING
     **********************************************************************************
     *
     *                            |              buffer_size              |
     *                            |---------------------------------------|
     *                            |                                       |
     *
     *                         buffer          buf_ptr       buf_end
     *                            +---------------+-----------------------+
     *                            |/ / / / / / / /|/ / / / / / /|         |
     *  read buffer:              |/ / consumed / | to be read /|         |
     *                            |/ / / / / / / /|/ / / / / / /|         |
     *                            +---------------+-----------------------+
     *
     *                                                         pos
     *              +-------------------------------------------+-----------------+
     *  input file: |                                           |                 |
     *              +-------------------------------------------+-----------------+
     *
     *
     **********************************************************************************
     *                                   WRITING
     **********************************************************************************
     *
     *                             |          buffer_size                 |
     *                             |--------------------------------------|
     *                             |                                      |
     *
     *                                                buf_ptr_max
     *                          buffer                 (buf_ptr)       buf_end
     *                             +-----------------------+--------------+
     *                             |/ / / / / / / / / / / /|              |
     *  write buffer:              | / / to be flushed / / |              |
     *                             |/ / / / / / / / / / / /|              |
     *                             +-----------------------+--------------+
     *                               buf_ptr can be in this
     *                               due to a backward seek
     *
     *                            pos
     *               +-------------+----------------------------------------------+
     *  output file: |             |                                              |
     *               +-------------+----------------------------------------------+
     *
     */
    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end; /**< End of the data, may be less than
                                 buffer+buffer_size if the read function returned
                                 less data than requested, e.g. for streams where
                                 no more data has been received yet. */
    void *opaque;           /**< A private pointer, passed to the read/write/seek/...
                                 functions. */
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
    
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);

    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;            /**< position in the file of the current buffer */
    int eof_reached;        /**< true if was unable to read due to error or eof */
    int write_flag;         /**< true if open for writing */
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;              /**< contains the error code or 0 if no error happened */
    /**
     * Pause or resume playback for network streaming protocols - e.g. MMS.
     */
    int (*read_pause)(void *opaque, int pause);
    /**
     * Seek to a given timestamp in stream with the specified stream_index.
     * Needed for some network streaming protocols which don't support seeking
     * to byte position.
     */
    int64_t (*read_seek)(void *opaque, int stream_index,
                         int64_t timestamp, int flags);
    /**
     * A combination of AVIO_SEEKABLE_ flags or 0 when the stream is not seekable.
     */
    int seekable;

    /**
     * max filesize, used to limit allocations
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int64_t maxsize;

    /**
     * avio_read and avio_write should if possible be satisfied directly
     * instead of going through a buffer, and avio_seek will always
     * call the underlying seek function directly.
     */
    int direct;

    /**
     * Bytes read statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int64_t bytes_read;

    /**
     * seek statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int seek_count;

    /**
     * writeout statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int writeout_count;

    /**
     * Original buffer size
     * used internally after probing and ensure seekback to reset the buffer size
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int orig_buffer_size;

    /**
     * Threshold to favor readahead over seek.
     * This is current internal only, do not use from outside.
     */
    int short_seek_threshold;

    /**
     * ',' separated list of allowed protocols.
     */
    // const char *protocol_whitelist;

    /**
     * ',' separated list of disallowed protocols.
     */
    // const char *protocol_blacklist;

    /**
     * A callback that is used instead of write_packet.
     */
    int (*write_data_type)(void *opaque, uint8_t *buf, int buf_size,
                           enum AVIODataMarkerType type, int64_t time);
    /**
     * If set, don't call write_data_type separately for AVIO_DATA_MARKER_BOUNDARY_POINT,
     * but ignore them and treat them as AVIO_DATA_MARKER_UNKNOWN (to avoid needlessly
     * small chunks of data returned from the callback).
     */
    int ignore_boundary_point;

    /**
     * Internal, not meant to be used from outside of TYM_IO_CONTEXT.
     */
    enum AVIODataMarkerType current_type;
    int64_t last_time;

    /**
     * A callback that is used instead of short_seek_threshold.
     * This is current internal only, do not use from outside.
     */
    int (*short_seek_get)(void *opaque);

    int64_t written;

    /**
     * Maximum reached position before a backward seek in the write buffer,
     * used keeping track of already written data for a later flush.
     */
    unsigned char *buf_ptr_max;

    /**
     * Try to buffer at least this amount of data before flushing it
     */
    int min_packet_size;
} TYM_IO_CONTEXT;



TYM_IO_CONTEXT *tym_io_alloc_context(
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence));

/**
 * Free the supplied IO context and everything associated with it.
 *
 * @param s Double pointer to the IO context. This function will write NULL
 * into s.
 */
void avio_context_free(TYM_IO_CONTEXT **s);

void avio_w8(TYM_IO_CONTEXT *s, int b);
void avio_write(TYM_IO_CONTEXT *s, const unsigned char *buf, int size);
void avio_wl64(TYM_IO_CONTEXT *s, uint64_t val);
void avio_wb64(TYM_IO_CONTEXT *s, uint64_t val);
void avio_wl32(TYM_IO_CONTEXT *s, unsigned int val);
void avio_wb32(TYM_IO_CONTEXT *s, unsigned int val);
void avio_wl24(TYM_IO_CONTEXT *s, unsigned int val);
void avio_wb24(TYM_IO_CONTEXT *s, unsigned int val);
void avio_wl16(TYM_IO_CONTEXT *s, unsigned int val);
void avio_wb16(TYM_IO_CONTEXT *s, unsigned int val);

void ffio_fill(TYM_IO_CONTEXT *s, int b, int count);

int avio_printf(TYM_IO_CONTEXT *s, const char *fmt, ...);

/**
 * Mark the written bytestream as a specific type.
 *
 * Zero-length ranges are omitted from the output.
 *
 * @param time the stream time the current bytestream pos corresponds to
 *             (in AV_TIME_BASE units), or TYM_NOPTS_VALUE if unknown or not
 *             applicable
 * @param type the kind of data written starting at the current pos
 */
void avio_write_marker(TYM_IO_CONTEXT *s, int64_t time, enum AVIODataMarkerType type);

/**
 * ORing this as the "whence" parameter to a seek function causes it to
 * return the filesize without seeking anywhere. Supporting this is optional.
 * If it is not supported then the seek function will return <0.
 */
#define AVSEEK_SIZE 0x10000

/**
 * Passing this flag as the "whence" parameter to a seek function causes it to
 * seek by any means (like reopening and linear reading) or other normally unreasonable
 * means that can be extremely slow.
 * This may be ignored by the seek code.
 */
#define AVSEEK_FORCE 0x20000

/**
 * fseek() equivalent for TYM_IO_CONTEXT.
 * @return new position or AVERROR.
 */
int64_t avio_seek(TYM_IO_CONTEXT *s, int64_t offset, int whence);
/**
 * Skip given number of bytes forward
 * @return new position or AVERROR.
 */
int64_t avio_skip(TYM_IO_CONTEXT *s, int64_t offset);

/**
 * ftell() equivalent for TYM_IO_CONTEXT.
 * @return position or AVERROR.
 */
static inline int64_t avio_tell(TYM_IO_CONTEXT *s)
{
    return avio_seek(s, 0, SEEK_CUR);
}


/**
 * Force flushing of buffered data.
 *
 * For write streams, force the buffered data to be immediately written to the output,
 * without to wait to fill the internal buffer.
 *
 * For read streams, discard all currently buffered data, and advance the
 * reported file position to that of the underlying stream. This does not
 * read new data, and does not perform any seeks.
 */
void tym_io_flush(TYM_IO_CONTEXT *s);

/**
 * Read size bytes from TYM_IO_CONTEXT into buf.
 * @return number of bytes read or AVERROR
 */
int avio_read(TYM_IO_CONTEXT *s, unsigned char *buf, int size);



/**
 * @name Functions for reading from TYM_IO_CONTEXT
 * @{
 *
 * @note return 0 if EOF, so you cannot use it if EOF handling is
 *       necessary
 */
int          avio_r8  (TYM_IO_CONTEXT *s);
unsigned int avio_rl16(TYM_IO_CONTEXT *s);
unsigned int avio_rl24(TYM_IO_CONTEXT *s);
unsigned int avio_rl32(TYM_IO_CONTEXT *s);
uint64_t     avio_rl64(TYM_IO_CONTEXT *s);
unsigned int avio_rb16(TYM_IO_CONTEXT *s);
unsigned int avio_rb24(TYM_IO_CONTEXT *s);
unsigned int avio_rb32(TYM_IO_CONTEXT *s);
uint64_t     avio_rb64(TYM_IO_CONTEXT *s);

/**
 * @}
 */

/**
 * Use non-blocking mode.
 * If this flag is set, operations on the context will return
 * AVERROR(EAGAIN) if they can not be performed immediately.
 * If this flag is not set, operations on the context will never return
 * AVERROR(EAGAIN).
 * Note that this flag does not affect the opening/connecting of the
 * context. Connecting a protocol will always block if necessary (e.g. on
 * network protocols) but never hang (e.g. on busy devices).
 * Warning: non-blocking protocols is work-in-progress; this flag may be
 * silently ignored.
 */
#define AVIO_FLAG_NONBLOCK 8

/**
 * Use direct mode.
 * avio_read and avio_write should if possible be satisfied directly
 * instead of going through a buffer, and avio_seek will always
 * call the underlying seek function directly.
 */
#define AVIO_FLAG_DIRECT 0x8000

/**
 * Create and initialize a TYM_IO_CONTEXT for accessing the
 * resource indicated by url.
 * @note When the resource indicated by url has been opened in
 * read+write mode, the TYM_IO_CONTEXT can be used only for writing.
 *
 * @param s Used to return the pointer to the created TYM_IO_CONTEXT.
 * In case of failure the pointed to value is set to NULL.
 * @param url resource to access
 * @param flags flags which control how the resource indicated by url
 * is to be opened
 * @return >= 0 in case of success, a negative value corresponding to an
 * AVERROR code in case of failure
 */
int tym_inner_io_open(TYM_IO_CONTEXT **s, const char *url, int flags);


/**
 * Close the resource accessed by the TYM_IO_CONTEXT s and free it.
 * This function can only be used if s was opened by tym_inner_io_open().
 *
 * The internal buffer is automatically flushed before closing the
 * resource.
 *
 * @return 0 on success, an AVERROR < 0 on error.
 * @see avio_closep
 */
int tym_io_close(TYM_IO_CONTEXT *s);

/**
 * Close the resource accessed by the TYM_IO_CONTEXT *s, free it
 * and set the pointer pointing to it to NULL.
 * This function can only be used if s was opened by tym_inner_io_open().
 *
 * The internal buffer is automatically flushed before closing the
 * resource.
 *
 * @return 0 on success, an AVERROR < 0 on error.
 * @see tym_io_close
 */
int avio_closep(TYM_IO_CONTEXT **s);


/**
 * Open a write only memory stream.
 *
 * @param s new IO context
 * @return zero if no error.
 */
int avio_open_dyn_buf(TYM_IO_CONTEXT **s);

/**
 * Return the written size and a pointer to the buffer.
 * The TYM_IO_CONTEXT stream is left intact.
 * The buffer must NOT be freed.
 * No padding is added to the buffer.
 *
 * @param s IO context
 * @param pbuffer pointer to a byte buffer
 * @return the length of the byte buffer
 */
int avio_get_dyn_buf(TYM_IO_CONTEXT *s, uint8_t **pbuffer);

/**
 * Return the written size and a pointer to the buffer. The buffer
 * must be freed with av_free().
 * Padding of TYM_INPUT_BUFFER_PADDING_SIZE is added to the buffer.
 *
 * @param s IO context
 * @param pbuffer pointer to a byte buffer
 * @return the length of the byte buffer
 */
int avio_close_dyn_buf(TYM_IO_CONTEXT *s, uint8_t **pbuffer);





/**
 * Seek to a given timestamp relative to some component stream.
 * Only meaningful if using a network streaming protocol (e.g. MMS.).
 *
 * @param h IO context from which to call the seek function pointers
 * @param stream_index The stream index that the timestamp is relative to.
 *        If stream_index is (-1) the timestamp should be in AV_TIME_BASE
 *        units from the beginning of the presentation.
 *        If a stream_index >= 0 is used and the protocol does not support
 *        seeking based on component streams, the call will fail.
 * @param timestamp timestamp in AVStream.time_base units
 *        or if there is no stream specified then in AV_TIME_BASE units.
 * @param flags Optional combination of AVSEEK_FLAG_BACKWARD, AVSEEK_FLAG_BYTE
 *        and AVSEEK_FLAG_ANY. The protocol may silently ignore
 *        AVSEEK_FLAG_BACKWARD and AVSEEK_FLAG_ANY, but AVSEEK_FLAG_BYTE will
 *        fail if used and not supported.
 * @return >= 0 on success
 * @see AVInputFormat::read_seek
 */
int64_t avio_seek_time(TYM_IO_CONTEXT *h, int stream_index,
                       int64_t timestamp, int flags);




/** @warning must be called before any I/O */
int ffio_set_buf_size(TYM_IO_CONTEXT *s, int buf_size);

static inline void ffio_wfourcc(TYM_IO_CONTEXT *pb, const uint8_t *s)
{
    avio_wl32(pb, MKTAG(s[0], s[1], s[2], s[3]));
}

#endif /* AVFORMAT_AVIO_H */
