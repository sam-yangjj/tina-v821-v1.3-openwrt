/** @file tuya_g711_utils.h
 * @brief g711 encode and decode utility
 */
#ifndef __TUYA_G711_UTILS_H__
#define __TUYA_G711_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define		TUYA_G711_A_LAW     (0)
#define		TUYA_G711_MU_LAW    (1)

#if 1
/** @brief encode 8K pcm to g711
 * @param[in]  type    TUYA_G711_A_LAW or TUYA_G711_MU_LAW
 * @param[in]  src     pcm data
 * @param[in]  src_len  size of pcm data
 * @param[in]  drc     g711 out data
 * @param[in]  p_out   size of g711 out data
 * @return error code
 * - 0  success
 * - -1 fail
 */
int tuya_g711_encode(unsigned char type, unsigned short *src, unsigned int src_len, unsigned char *drc, unsigned int *p_out);

/** @brief decode 8K g711 to pcm
 * @param[in]  type    TUYA_G711_A_LAW or TUYA_G711_MU_LAW
 * @param[in]  src     g711 data
 * @param[in]  src_len  size of g711 data
 * @param[in]  drc     pcm out data
 * @param[in]  p_out   size of pcm out data
 * @return error code
 * - 0 success
 * - -1 fail
 */
int tuya_g711_decode(unsigned char type, unsigned short *src, unsigned int src_len, unsigned char *drc, unsigned int *p_out);

/** @brief encode 16K pcm to g711
 * @param[in]  type    TUYA_G711_A_LAW or TUYA_G711_MU_LAW
 * @param[in]  src     pcm data
 * @param[in]  src_len  size of pcm data
 * @param[in]  drc     g711 out data
 * @param[in]  p_out   size of g711 out data
 * @return error code
 * - 0  success
 * - -1 fail
 */
int tuya_g711_encode_16K(unsigned char type, unsigned short *src, unsigned int src_len, unsigned char *drc, unsigned int *p_out);

/** @brief decode g711 to 16k pcm
 * @param[in]  type    TUYA_G711_A_LAW or TUYA_G711_MU_LAW
 * @param[in]  src     g711 data
 * @param[in]  src_len  size of g711 data
 * @param[out]  drc     16k pcm out data
 * @param[out]  p_out   size of pcm out data
 * @return error code
 * - 0  success
 * - -1 fail
 */
int tuya_g711_decode_16K(unsigned char type, unsigned char *src, unsigned int src_len, unsigned short *drc, unsigned int *p_out);
#else

void tuya_g711_encode(unsigned int type, unsigned int sample_num, unsigned char *samples, unsigned char *bitstream);

void tuya_g711_encode_16k(unsigned int type, unsigned int sample_num, unsigned char *samples, unsigned char *bitstream);

void tuya_g711_decode(unsigned int type, unsigned int sample_num, unsigned char *samples, unsigned char *bitstream);

#endif
#ifdef __cplusplus
}
#endif

#endif // TUYA_G711_UTILS_H
