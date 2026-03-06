#
# tuya upgrade device side Makefile
#
LDLIBS = -lpthread 
LIBVERSION = 1.0.0
CC ?= gcc
PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib
BINARY_PATH ?= bin
INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)
INSTALL_BINARY_PATH = $(DESTDIR)$(PREFIX)/$(BINARY_PATH)
INSTALL ?= cp -a

#库文件名称
USER_LIB_STATIC_NAME := libunpack.a
USER_LIB_SHARED_NAME := libunpack.so
USER_PROGRAM_NAME := unpack
USER_PROGRESS_NAME := progress
USER_CLIENT_NAME := client

#这里添加头文件目录
USER_INC_DIRS := ./ ./nvram ./http ./ipc ./ 3rdparty/mbedtls/crypto/include/ 3rdparty/mbedtls/include/ 3rdparty/mtd-utils/include/ bsp_verify

#这里添加源码文件列表
USER_SRCS := handler.c notifier.c property.c swupgrade_async.c swupgrade_network.c swupgrade_progress.c tuya_unpack.c ipc/network_ipc.c ipc/progress_ipc.c nvram/nvram.c bsp_verify/ecdsa.c  bsp_verify/sha256.c  bsp_verify/tuya_verify.c
USER_SRCS += handlers/amlogic_nand_flash_handler.c handlers/dummy_handler.c handlers/nand_flash_handler.c handlers/nor_flash_handler.c handlers/rockchip_nand_flash_handler.c 3rdparty/mtd-utils/lib/libmtd.c ./3rdparty/mtd-utils/lib/libmtd_legacy.c

ifdef ENABLE_URL_INSTALL
USER_SRCS += http/aes_inf.c http/httpc.c http/http_inf.c http/md5.c http/tuya_tls.c
USER_SRCS += \
3rdparty/mbedtls/library/aesni.c        3rdparty/mbedtls/library/ecdh.c                 3rdparty/mbedtls/library/oid.c            3rdparty/mbedtls/library/ssl_ciphersuites.c \
3rdparty/mbedtls/library/arc4.c         3rdparty/mbedtls/library/ecdsa.c                3rdparty/mbedtls/library/padlock.c        3rdparty/mbedtls/library/ssl_cli.c\
3rdparty/mbedtls/library/aria.c         3rdparty/mbedtls/library/ecjpake.c              3rdparty/mbedtls/library/pem.c            3rdparty/mbedtls/library/ssl_cookie.c\
3rdparty/mbedtls/library/asn1parse.c    3rdparty/mbedtls/library/ecp.c                  3rdparty/mbedtls/library/pk.c             3rdparty/mbedtls/library/ssl_srv.c\
3rdparty/mbedtls/library/asn1write.c    3rdparty/mbedtls/library/ecp_curves.c           3rdparty/mbedtls/library/pkcs11.c         3rdparty/mbedtls/library/ssl_ticket.c\
3rdparty/mbedtls/library/base64.c       3rdparty/mbedtls/library/entropy.c              3rdparty/mbedtls/library/pkcs12.c         3rdparty/mbedtls/library/ssl_tls.c\
3rdparty/mbedtls/library/bignum.c       3rdparty/mbedtls/library/entropy_poll.c         3rdparty/mbedtls/library/pkcs5.c          3rdparty/mbedtls/library/threading.c\
3rdparty/mbedtls/library/blowfish.c     3rdparty/mbedtls/library/error.c                3rdparty/mbedtls/library/pkparse.c        3rdparty/mbedtls/library/timing.c\
3rdparty/mbedtls/library/camellia.c     3rdparty/mbedtls/library/gcm.c                  3rdparty/mbedtls/library/pk_wrap.c        3rdparty/mbedtls/library/version.c\
3rdparty/mbedtls/library/ccm.c          3rdparty/mbedtls/library/havege.c               3rdparty/mbedtls/library/pkwrite.c        3rdparty/mbedtls/library/version_features.c\
3rdparty/mbedtls/library/certs.c        3rdparty/mbedtls/library/hkdf.c                 3rdparty/mbedtls/library/platform.c       3rdparty/mbedtls/library/x509.c\
3rdparty/mbedtls/library/chacha20.c     3rdparty/mbedtls/library/hmac_drbg.c            3rdparty/mbedtls/library/platform_util.c  3rdparty/mbedtls/library/x509_create.c\
3rdparty/mbedtls/library/chachapoly.c   3rdparty/mbedtls/library/md2.c                  3rdparty/mbedtls/library/poly1305.c       3rdparty/mbedtls/library/x509_crl.c\
3rdparty/mbedtls/library/cipher.c       3rdparty/mbedtls/library/md4.c                  3rdparty/mbedtls/library/ripemd160.c      3rdparty/mbedtls/library/x509_crt.c\
3rdparty/mbedtls/library/cipher_wrap.c  3rdparty/mbedtls/library/md5.c                  3rdparty/mbedtls/library/rsa.c            3rdparty/mbedtls/library/x509_csr.c\
3rdparty/mbedtls/library/cmac.c         3rdparty/mbedtls/library/md.c                   3rdparty/mbedtls/library/rsa_internal.c   3rdparty/mbedtls/library/x509write_crt.c\
3rdparty/mbedtls/library/ctr_drbg.c     3rdparty/mbedtls/library/md_wrap.c              3rdparty/mbedtls/library/sha1.c           3rdparty/mbedtls/library/x509write_csr.c\
3rdparty/mbedtls/library/debug.c        3rdparty/mbedtls/library/memory_buffer_alloc.c  3rdparty/mbedtls/library/sha256.c         3rdparty/mbedtls/library/xtea.c\
3rdparty/mbedtls/library/des.c          3rdparty/mbedtls/library/net_sockets.c          3rdparty/mbedtls/library/sha512.c	  3rdparty/mbedtls/library/aes.c\
./3rdparty/mbedtls/library/dhm.c
endif

#编译所用变量
OUTPUT_DIR := output
OUTPUT_DIR_OBJS := $(OUTPUT_DIR)/.objs

#user的obj命令
USER_OBJS := $(addsuffix .o, $(USER_SRCS))

#user的实际obj地址
USER_OBJS_OUT :=  $(addprefix $(OUTPUT_DIR_OBJS)/, $(USER_OBJS))

#包含头文件列表
CFLAGS += $(addprefix -I, $(USER_INC_DIRS))
CFLAGS += $(USER_CFLAGS) -fPIC

ifdef ENABLE_URL_INSTALL
CFLAGS += -DENABLE_URL_INSTALL=1
endif

all: $(OUTPUT_DIR)/libs/$(USER_LIB_STATIC_NAME) \
	$(OUTPUT_DIR)/libs/$(USER_LIB_SHARED_NAME) \
	$(OUTPUT_DIR)/$(USER_PROGRAM_NAME) \
	$(OUTPUT_DIR)/$(USER_PROGRESS_NAME) \
	$(OUTPUT_DIR)/$(USER_CLIENT_NAME)

$(OUTPUT_DIR)/libs/$(USER_LIB_STATIC_NAME): $(USER_OBJS_OUT)
	mkdir -p $(dir $@)
	$(AR) -rcs $@ $^
	@echo "Build $@ Finish"

$(OUTPUT_DIR)/libs/$(USER_LIB_SHARED_NAME): $(USER_OBJS_OUT)
	mkdir -p $(dir $@)
	$(CC) -fPIC -shared -o $@ $^ $(LDLIBS)
	@echo "Build $@ Finish"

$(OUTPUT_DIR)/$(USER_PROGRAM_NAME): main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) -Loutput/libs -lunpack
	#$(CC) -I. -I ipc $^ -o $@ -static output/libs/libunpack.a $(LDLIBS)

$(OUTPUT_DIR)/$(USER_CLIENT_NAME): tools/swupdate-client.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) -Loutput/libs -lunpack

$(OUTPUT_DIR)/$(USER_PROGRESS_NAME): tools/swupdate-progress.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) -Loutput/libs -lunpack

$(OUTPUT_DIR_OBJS)/%.c.o: %.c
	@mkdir -p $(dir $@);
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: all clean
clean:
	rm -rf $(OUTPUT_DIR)

#install
install-tuya_unpack:
	mkdir -p $(INSTALL_LIBRARY_PATH) $(INSTALL_INCLUDE_PATH)
	$(INSTALL) tuya_unpack.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) ipc/network_ipc.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) ipc/progress_ipc.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(OUTPUT_DIR)/libs/$(USER_LIB_SHARED_NAME) $(INSTALL_LIBRARY_PATH)
	$(INSTALL) $(OUTPUT_DIR)/$(USER_PROGRAM_NAME) $(INSTALL_BINARY_PATH)

install: install-tuya_unpack

#uninstall
uninstall-tuya_unpack:
	$(RM) $(INSTALL_BINARY_PATH)/$(USER_PROGRAM_NAME)
	$(RM) $(INSTALL_LIBRARY_PATH)/$(USER_LIB_SHARED_NAME)
	$(RM) $(INSTALL_INCLUDE_PATH)/tuya_unpack.h
	$(RM) $(INSTALL_INCLUDE_PATH)/network_ipc.h
	$(RM) $(INSTALL_INCLUDE_PATH)/progress_ipc.h

remove-dir:
	$(if $(wildcard $(INSTALL_LIBRARY_PATH)/*.*),,rmdir $(INSTALL_LIBRARY_PATH))
	$(if $(wildcard $(INSTALL_INCLUDE_PATH)/*.*),,rmdir $(INSTALL_INCLUDE_PATH))

uninstall: uninstall-tuya_unpack remove-dir
