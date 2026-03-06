/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/manager/audio_subsys_manager.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-24 11:38:07
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-07-31 18:33:25
 */
#ifndef __AUDIO_SUBSYS_MANAGER_H__
#define __AUDIO_SUBSYS_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

/**
 * @brief Create audio subsystem manager instance
 *
 * @param self - handle pointer
 * @param cfg - configuration for manager
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_create(void **self, void *cfg);

/**
 * @brief Destroy audio subsystem manager instance
 *
 * @param self - handle
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_destroy(void *self);

/**
 * @brief Register audio subsystem module
 *
 * @note Register and a created module instance,
 *      and the module will be destroyed when unregistered.
 *
 * @warning alias `main` means current manager, `all` means all registered modules.
 *      So don't use `main` or `all` for register module alias.
 *
 * @param self - handle
 * @param alias - audio subsystem module alias
 * @param module - audio subsystem module pointer
 * @param cfg - configuration for module
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_register(void *self, const char *alias, audio_subsys_module_t *module, void *cfg);

/**
 * @brief Unregister audio subsystem module
 *
 * @note Unregister and destroy module instance, if the module is registered.
 *
 * @param self - handle
 * @param alias - audio subsystem module alias
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_unregister(void *self, const char *alias);

/**
 * @brief Get audio subsystem module
 *
 * @param self - handle
 * @param alias - audio subsystem module alias
 * @param module - audio subsystem module pointer
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_get_module(void *self, const char *alias, audio_subsys_module_t **module);

/**
 * @brief Set audio subsystem callback
 *
 * @note When alias is `main` / `all`, the callback is set for:
 *      1. all register audio subsystem modules? - When `all`！
 *      2. current manager main callback? - When `main`!
 *
 * @note Default, it is the corresponding module registered by alias.
 *
 * @param self - handle
 * @param alias - audio subsystem module alias
 * @param cb - audio subsystem callback
 * @param userdata - callback userdata, user context
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_set_callback(void *self, const char *alias, audio_subsys_callback_t cb, void *userdata);

/**
 * @brief Control audio subsystem module
 *
 * @note When alias is `main` / `all`, the callback is set for:
 *      1. all register audio subsystem modules? - When `all`！
 *      2. current manager main callback? - When `main`!
 *
 * @note Default, it is the corresponding module registered by alias.
 *
 * @param self - handle
 * @param alias - audio subsystem module alias
 * @param cmd - control command, command enumeration
 * @param arg - control command argument pinter
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_ioctl(void *self, const char *alias, int cmd, void *arg);

/**
 * @brief Set pipeline for audio subsystem modules
 *
 * @code
 *  const char *alias[] = { "agc", "ns", "aec" };
 *  int num = sizeof(alias) / sizeof(alias[0]);
 *  audio_subsys_manager_pipeline(self, alias, num);
 * @endcode
 *
 * @param self - handle
 * @param alias - audio subsystem module alias table pinter
 * @param num - audio subsystem module alias table numbers
 * @return int - 0: success; -1: fail
 */
int audio_subsys_manager_pipeline(void *self, const char *alias[], int num);

/**
 * @brief Process audio data
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
 * @param self - handle
 * @param in - input data pointer, byte stream
 * @param inlen - input data length, number of bytes
 * @param out - output data pointer, byte stream
 * @param outlen - output data length(maximum length), number of bytes
 * @return int - Greater than or equal to 0: success, value is number of out bytes;
 *               Less than to 0: fail
 */
int audio_subsys_manager_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif