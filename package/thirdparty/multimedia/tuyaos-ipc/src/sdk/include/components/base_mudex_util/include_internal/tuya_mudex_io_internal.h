

#ifndef __TUYA_MUDEX_IO_INTERNAL_H__
#define __TUYA_MUDEX_IO_INTERNAL_H__

#include "tuya_mudex_io_buffer.h"
#include "tuya_mudex_procotols_util.h"

#include "tuya_mudex_log_util.h"

#define TYM_INPUT_BUFFER_PADDING_SIZE 6

int tym_io_init_context(TYM_IO_CONTEXT *s,
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence));

int tuya_io_fdopen(TYM_IO_CONTEXT **s, URLContext *h);

//如果mode为STREAM_TYPE_FILE时，base和size可以任意填
//如果mode为STREAM_TYPE_MEM时，必须填写base和size
int tym_io_open(TYM_IO_CONTEXT **s, const char *url, int flags,
                         const AVIOInterruptCB *int_cb,enum TUYA_MUDEX_IO_STREAM_TYPE mode,unsigned char *base, int size);



/**
 * Free a dynamic buffer.
 *
 * @param s a pointer to an IO context opened by avio_open_dyn_buf()
 */
void tuya_free_dyn_buf(TYM_IO_CONTEXT **s);

#endif /* AVFORMAT_AVIO_INTERNAL_H */
