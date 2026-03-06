#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include "cryptfs.h"
#include <inttypes.h>

#ifndef CRYPTO_KEY
#error "crypto key must be defined !!!"
#endif

#define DM_CRYPT_SECTOR_SIZE 512

static void uasge(char *prog)
{
	printf("Usage:\n");
	printf("\t%s -i [raw image] -o [encrypt image]\n", prog);
}

static int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
			return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
			return ch - 'a' + 10;
	return -1;
}

static int hex2bin(uint8_t *dst, const char *src, size_t count)
{
	while (count--) {
			int hi = hex_to_bin(*src++);
			int lo = hex_to_bin(*src++);
			if ((hi < 0) || (lo < 0))
					return -EINVAL;

			*dst++ = (hi << 4) | lo;
	}
	return 0;
}

/* This function generate essiv for every sector */
static int dm_essiv_generate(EVP_CIPHER_CTX *ctx, uint64_t sector, unsigned char *essiv)
{
	unsigned char iv[EVP_MAX_IV_LENGTH];
	int len;

	memset(essiv, 0, EVP_MAX_IV_LENGTH);
	memset(iv, 0, EVP_MAX_IV_LENGTH);
	/* convert to little-endian */
	*(uint64_t *)essiv = htole64(sector);
	EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, iv);
	EVP_EncryptUpdate(ctx, essiv, &len, essiv, EVP_MAX_IV_LENGTH);

	return len;
}

static int dm_encrypt(const char *in_file, const char *out_file, unsigned char *ckey)
{
	EVP_CIPHER_CTX *block_ctx, *essiv_ctx;
	EVP_MD_CTX *md_ctx;
	unsigned char essiv[EVP_MAX_IV_LENGTH];
	unsigned char salt[EVP_MAX_MD_SIZE];
	unsigned char key[32];
	unsigned char in_buf[DM_CRYPT_SECTOR_SIZE], out_buf[DM_CRYPT_SECTOR_SIZE];
	unsigned int salt_len;
	uint64_t sector = 0;
	int in_fd, out_fd;
	int err = 0;

	memset(key, 0, 32);
	hex2bin(key, (char *)ckey, 32);

	/* Calculate key's sha256 as essiv cipher salt */
	md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx, EVP_sha256(), NULL);
	EVP_DigestUpdate(md_ctx, key, 32);
	EVP_DigestFinal(md_ctx, salt, &salt_len);
	
	EVP_MD_CTX_free(md_ctx);

	memset(essiv, 0, EVP_MAX_IV_LENGTH);

	/* Init essiv cipher */
	essiv_ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(essiv_ctx, EVP_aes_256_cbc(), NULL, salt, essiv);
	/* Init block data cipher */
	block_ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(block_ctx, EVP_aes_256_cbc(), NULL, key, essiv);

	in_fd = open(in_file, O_RDONLY);
	out_fd = open(out_file, O_CREAT | O_RDWR, 0660);
	if ((in_fd < 0) || (out_fd < 0)) {
		err = -EBADF;
		goto out;
	}

	/* Every thing is ok, let's rock */
	for (sector = 0;; sector++)
	{
		int len, cipher_len;
		int n = read(in_fd, in_buf, DM_CRYPT_SECTOR_SIZE);
		if (n == 0) {
			break;
		}

		dm_essiv_generate(essiv_ctx, sector, essiv);

		EVP_EncryptInit_ex(block_ctx, NULL, NULL, NULL, essiv);
		EVP_EncryptUpdate(block_ctx, out_buf, &len, in_buf, n);
		cipher_len = len;
		if (n < DM_CRYPT_SECTOR_SIZE) {
			EVP_EncryptFinal_ex(block_ctx, out_buf, &len);
			cipher_len += len;
		}

		if (write(out_fd, out_buf, cipher_len) != cipher_len) {
			err = -EIO;
			goto out;
		}
	}

out:
	if (in_fd > 0)
		close(in_fd);
	if (out_fd > 0)
		close(out_fd);
	EVP_CIPHER_CTX_free(essiv_ctx);
	EVP_CIPHER_CTX_free(block_ctx);
	return err;
}

int main(int argc, char **argv)
{
	int opt;
	char *in_file = NULL, *out_file = NULL;

	while ((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch (opt) {
			case 'i':
				in_file = optarg;
                break;
            case 'o':
				out_file = optarg;
               break;
           default: /* '?' */
				uasge(argv[0]);
		   		exit(EXIT_FAILURE);
        }
    }

	if (in_file == NULL) {
		uasge(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (strlen(CRYPTO_KEY) != 64)
	{
		printf("Invalid key length:%ld\n", strlen(CRYPTO_KEY));
		return -1;
	}

	if (out_file == NULL) {
		if (getuid() || geteuid()) {
			printf("Must run with root permission!\n");
			return -1;
		}

		if (cryptfs_create(in_file, CRYPTO_KEY) != 0) {
			printf("Create cryptfs image %s failed\n", argv[1]);
			return -1;
		}
	} else {
		dm_encrypt(in_file, out_file, (unsigned char *)CRYPTO_KEY);
	}
	return 0;
}
