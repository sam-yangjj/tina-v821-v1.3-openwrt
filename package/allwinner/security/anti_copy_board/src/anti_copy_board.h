#ifndef _SUNXI_ANTI_COPY_BOARD_H_
#define _SUNXI_ANTI_COPY_BOARD_H_

#include <asm/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <linux/types.h>

#define AES_DIR_ENCRYPT		0  /* 加密 */
#define AES_DIR_DECRYPT		1  /* 解密 */

/* ioctl cmd */
#define CE_IOC_MAGIC			'C'
#define CE_IOC_REQUEST			_IOR(CE_IOC_MAGIC, 0, int)
#define CE_IOC_FREE			_IOW(CE_IOC_MAGIC, 1, int)
#define CE_IOC_AES_CRYPTO		_IOW(CE_IOC_MAGIC, 2, crypto_aes_req_ctx_t)  /* 执行加解密运算 */

/* AES method */
#define AES_MODE_ECB		0
#define AES_MODE_CBC		1
#define AES_MODE_CTR		2
#define AES_MODE_CTS		3
#define AES_MODE_OFB		4
#define AES_MODE_CFB		5
#define AES_MODE_CBC_MAC	6
#define AES_MODE_OCB		7
#define AES_MODE_GCM		8
#define AES_MODE_XTS		9

/* symm method */
#define SS_METHOD_AES 0x0
#define SS_METHOD_DES 0x1
#define SS_METHOD_3DES 0x2

/* ACB return value */
#define ACB_EFUSE_NO_SSK		(1)
#define ACB_VERIFY_RET_OK		(0)
#define ACB_VERIFY_RET_VERIFY_ERR	(-1)
#define ACB_VERIFY_RET_MALLOC_ERR	(-2)
#define ACB_VERIFY_RET_OPEN_NODE_ERR	(-3)
#define ACB_VERIFY_RET_IOCTL_RET_ERR	(-4)
#define ACB_VERIFY_RET_CE_CACUL_ERR	(-5)
#define ACB_VERIFY_RET_WRITE_NODE_ERR	(-6)
#define ACB_VERIFY_RET_READ_NODE_ERR	(-7)

#ifndef MIN
#define MIN(a, b) 		((a) > (b) ? (b) : (a))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ( sizeof(a) / sizeof(a[0]) )
#endif

typedef unsigned char		u8;
typedef unsigned int		u32;
typedef signed int		s32;

/* define the ctx for aes requtest */
typedef struct {
	u32 bit_width;	/* the bitwidth of CFB/CTR mode */
	u32 method;	/* symm method, such as AES/DES/3DES ... */
	u8 *src_buffer;
	u32 src_length;
	u8 *dst_buffer;
	u32 dst_length;
	u8 *key_buffer;
	u32 key_length;
	u8 *iv_buffer;
	u32 iv_length;
	u32 aes_mode;	/* symm mode, such as ECB/CBC/CTR ... */
	u32 dir;	/* encrypt or decrypt */
	u32 ion_flag;
	unsigned long src_phy;
	unsigned long dst_phy;
	unsigned long iv_phy;
	unsigned long key_phy;
	s32 channel_id;
} crypto_aes_req_ctx_t;

int sunxi_acb_verity(unsigned char *plaintext, unsigned char *ciphertext, unsigned int data_len);
#endif
