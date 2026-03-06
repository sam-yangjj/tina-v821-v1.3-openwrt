/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/common/audio_subsys_module.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-13 13:59:24
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-07-31 18:33:52
 */
#ifndef __AUDIO_SUBSYS_MODULE_H__
#define __AUDIO_SUBSYS_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief: audio subsystem module callback function
 * @param {void *} data - audio data pointer
 * @param {int} dlen - audio data length，number of bytes
 * @param {int} flag - flag state
 * @param {void *} userdata - user data context
 * @return {int} - 0: success; -1: fail
 */
typedef int(*audio_subsys_callback_t)(void *data, int dlen, int flag, void *userdata);

typedef struct audio_subsys_module {
    /**
     * @brief: audio subsystem module handle
     */
    void *self;

    /**
     * @brief: audio subsystem module name
     */
    char *name;

    /**
     * @brief: create audio subsystem module
     * @param {void **} self - module handle pointer
     * @param {void *} cfg - configuration for module
     * @return {int} - 0: success; -1: fail
     */
    int (*create)(void **self, void *cfg);

    /**
     * @brief: destroy audio subsystem module
     * @param {void *} self - module handle
     * @return {int} - 0: success; -1: fail
     */
    int (*destroy)(void *self);

    /**
     * @brief: set callback function
     * @param {void *} self - module handle
     * @param {audio_subsys_callback_t} cb - callback function
     * @param {void *} userdata - user data context
     * @return {int} - 0: success; -1: fail
     */
    int (*set_callback)(void *self, audio_subsys_callback_t cb, void *userdata);

    /**
     * @brief: control audio subsystem module
     * @param {void *} self - module handle
     * @param {int} cmd - command, e.g. load/unload/start/stop/rest/re_cfg
     * @param {void *} arg - argument pointer
     * @return {int} - 0: success; -1: fail
     */
    int (*ioctl)(void *self, int cmd, void *arg);

    /**
     * @brief: module process audio data
     *
     * @note: the input and output data are byte stream.
     *  If it is multi-channel data, the data should be arranged alternately.
     *  If internal processing necessitates the separate handling of MIC data and REF data,
     *  effect the separation internally.
     *
     * eg. int16_t(short) S16LE 4 channels multi-channel, 2mics 2refs (mic0 mic1 ref0 ref1) 2samples,
     * multi-channel data:┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┬───────
     *                    │16bit │16bit │16bit │16bit │16bit │16bit │16bit │16bit │ ......
     *                    ├──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┼───────
     *                    │ mic0 │ mic1 │ ref0 │ ref1 │ mic0 │ mic1 │ ref0 │ ref1 │ ......
     *                    └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┴───────
     * Internal split multi-channel,
     * microphone data:   ┌──────┬──────┬──────┬──────┬───────
     *                    │16bit │16bit │16bit │16bit │ ......
     *                    ├──────┼──────┼──────┼──────┼───────
     *                    │ mic0 │ mic1 │ mic0 │ mic1 │ ......
     *                    └──────┴──────┴──────┴──────┴───────
     * reference data:    ┌──────┬──────┬──────┬──────┬───────
     *                    │16bit │16bit │16bit │16bit │ ......
     *                    ├──────┼──────┼──────┼──────┼───────
     *                    │ ref0 │ ref1 │ ref0 │ ref1 │ ......
     *                    └──────┴──────┴──────┴──────┴───────
     *
     * @note FYI ![PCM音频格式理解](https://blog.csdn.net/eastcnme/article/details/104598952)
     *
     * @param {void *} self - module handle
     * @param {const void *} in - input data pointer, byte stream
     * @param {int} inlen - input data length, number of bytes
     * @param {void *} out - output data pointer, byte stream
     * @param {int} outlen - output data length(maximum length), number of bytes
     * @return {int} - Greater than or equal to 0: success, value is number of out bytes;
     *                 Less than to 0: fail
     */
    int (*process)(void *self, const void *in, int inlen, void *out, int outlen);
} audio_subsys_module_t;

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_SUBSYS_MODULE_H__
