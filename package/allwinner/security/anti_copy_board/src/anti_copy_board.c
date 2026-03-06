#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#include "anti_copy_board.h"

#define STR(x) #x
#define TO_STRING(x) STR(x)
#define SUNXI_SSK_READ_FLAG_REG	TO_STRING(SSK_READ_FLAG_REG)"\n"
#define SUNXI_REG_DUMP_PATH	"/sys/class/sunxi_dump/dump"
#define SUNXI_REG_WRITE_PATH	"/sys/class/sunxi_dump/write"

//#define ACB_DEBUG

static int sunxi_hexdump(const unsigned char *buf, int bytes)
{
#define HEXDUMP_BUFF_LEN   16
	unsigned char line[HEXDUMP_BUFF_LEN] = {0};
	int addr;

	for (addr = 0; addr < bytes; addr += HEXDUMP_BUFF_LEN) {
		int len = MIN(bytes - addr, HEXDUMP_BUFF_LEN), i;
		memcpy(line, buf + addr, len);
		memset(line + len, 0, HEXDUMP_BUFF_LEN - len);

		/* print addr */
		printf("    0x%.8X: ", addr);
		/* print hex */
		for (i = 0; i < HEXDUMP_BUFF_LEN; i++) {
			if (i < len) {
				printf("%.2X ", line[i]);
			} else {
				printf("   ");
			}
		}
		/* print char */
		printf("|");
		for (i = 0; i < HEXDUMP_BUFF_LEN; i++) {
			if (i < len) {
				if (line[i] >= 0x20 && line[i] <= 0x7E) {
					printf("%c", line[i]);
				} else {
					printf(".");
				}
			} else {
				printf(" ");
			}
		}
		printf("|\n");
	}
	return 0;
}

static int read_ssk_protect_flag(void)
{
	int fd;
	int ret = -1;
	char buffer[20];

	fd = open(SUNXI_REG_DUMP_PATH, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s \n", SUNXI_REG_DUMP_PATH);
		return ACB_VERIFY_RET_OPEN_NODE_ERR;
	}

	ret = write(fd, SUNXI_SSK_READ_FLAG_REG, sizeof(SUNXI_SSK_READ_FLAG_REG));
	if (ret < 0) {
		printf("Failed to write %s\n", SUNXI_REG_DUMP_PATH);
		ret = ACB_VERIFY_RET_WRITE_NODE_ERR;
		goto out;
	}

	lseek(fd, 0, SEEK_SET);
	ret = read(fd, buffer, sizeof(buffer));
	if (ret < 0) {
		printf("Failed to read  %s %d(%s)\n", SUNXI_REG_DUMP_PATH, ret, strerror(ret));
		ret = ACB_VERIFY_RET_READ_NODE_ERR;
		goto out;
	}

	ret = (strtoul(buffer, NULL, 16) & (1 << SSK_READ_PROTECT_FLAG) ? 0 : ACB_EFUSE_NO_SSK);

out:
	close(fd);
	return ret;
}

/* default AES-128-ECB */
static int acb_verity(u8 *plaintext, u8 *ciphertext, u32 data_len)
{
	int ret = 0, err = -1;
	int channel_id = -1;
	crypto_aes_req_ctx_t *aes_ctx = NULL;
	int fd;
	char *ptr;

	aes_ctx = malloc(sizeof(*aes_ctx));
	if (aes_ctx == NULL) {
		printf("aes_ctx malloc error\n");
		ret = ACB_VERIFY_RET_MALLOC_ERR;
		goto out;
	}
	memset(aes_ctx, 0x0, sizeof(*aes_ctx));

	if (data_len % 16 != 0) {
		printf("data len: %d, should align to 16!\n", data_len);
	}

	/* only CFB/CTR need set bit_width */
	//aes_ctx->bit_width = bit_width;
	aes_ctx->method = SS_METHOD_AES;
	aes_ctx->src_buffer = ciphertext;
	aes_ctx->src_length = data_len;
	aes_ctx->key_buffer = "KEY_SEL_SSK";
	aes_ctx->key_length = SSK_BYTES;
	aes_ctx->aes_mode = AES_MODE_ECB;
	aes_ctx->ion_flag = 0;

	if (aes_ctx->src_length & 0xF)
		aes_ctx->dst_length = ((aes_ctx->src_length + 15) / 16) * 16;
	else
		aes_ctx->dst_length = aes_ctx->src_length;
	aes_ctx->dst_buffer = malloc(aes_ctx->dst_length);
	if (aes_ctx->dst_buffer == NULL) {
		printf("aes_ctx->dst_buffer malloc error\n");
		ret = ACB_VERIFY_RET_MALLOC_ERR;
		goto out;
	}
	memset(aes_ctx->dst_buffer, 0x0, aes_ctx->dst_length);

	fd = open("/dev/ce", O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s\n", "/dev/ce");
		ret = ACB_VERIFY_RET_OPEN_NODE_ERR;
		goto fail;
	}

	err = ioctl(fd, CE_IOC_REQUEST, &channel_id);
	if (err) {
		printf("ioctl('CE_IOC_AES_CRYPTO REQUEST') failed\n");
		ret = ACB_VERIFY_RET_IOCTL_RET_ERR;
		goto out;
	}

	aes_ctx->channel_id = channel_id;
	aes_ctx->dir = AES_DIR_DECRYPT;
	err = ioctl(fd, CE_IOC_AES_CRYPTO, (unsigned long)aes_ctx);
	if (err) {
		printf("ioctl('CE_IOC_AES_CRYPTO CRYPTO') failed\n");
		ret = ACB_VERIFY_RET_CE_CACUL_ERR;
		goto out;
	}

	if (memcmp(plaintext, aes_ctx->dst_buffer, aes_ctx->dst_length)) {
#ifdef ACB_DEBUG
		printf("Origin data:\n");
		sunxi_hexdump(plaintext, aes_ctx->dst_length);
		printf("Decoded data:\n");
		sunxi_hexdump(aes_ctx->dst_buffer, aes_ctx->dst_length);
#endif
		ret = ACB_VERIFY_RET_VERIFY_ERR;
		goto out;
	}
out:
	err = ioctl(fd, CE_IOC_FREE, &channel_id);
	if (err) {
		printf("ioctl('CE_IOC_AES_CRYPTO CE_IOC_FREE') failed\n");
		ret = ACB_VERIFY_RET_IOCTL_RET_ERR;
	}

fail:
	close(fd);

	if (aes_ctx) {
		if (aes_ctx->dst_buffer)
			free(aes_ctx->dst_buffer);
		free(aes_ctx);
	}

	return ret;
}

int sunxi_acb_verity(u8 *plaintext, u8 *ciphertext, u32 data_len)
{
	int ret;

	ret = read_ssk_protect_flag();
	if (ret == 0)
		printf("efuse had write ssk!\n");
	else if (ret == 1) {
		printf("efuse have not write ssk! do not verity\n");
		return ret;
	} else {
		printf("read efuse fail :%d\n", ret);
		return ret;
	}

	ret = acb_verity(plaintext, ciphertext, data_len);
	if (ret == 0) {
		printf("ACB verity success!\n");
	} else {
		printf("ACB verity fail: %d\n", ret);
	}

	return ret;
}
