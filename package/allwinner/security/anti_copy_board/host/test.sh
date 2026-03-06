#!/bin/bash

if [ $# -ne 2 ]; then
        echo "USAGE: $0 <ssk_bin_file> <plaintext>"
        exit
fi

if [ ! -f $1 ]; then
        echo "$1 is not exist!"
        exit
fi

if [ ! -f $2 ]; then
        echo "$2 is not exist!"
        exit
fi

PLAIN=$2		# 明文文件
CIPHER=ciphertext	# 生成的加密文件
DECRYPT=decryptext	# 基于加密文件解密出的文件
KEYLENBITS=`stat -c %s $1 | awk '{print $1 * 8}'`
KEYFILEBITS=`stat -c "%s" $1 | awk '{print $1*8}'`

if [ "$KEYLENBITS" -ne "$KEYFILEBITS" ]; then
        echo "$1 file len is error"
        exit
fi

AES_KEY=`hexdump -v -e '32/1 "%02x" "\n"' $1`

# 将plain加密为cipher
openssl enc -e -aes-${KEYLENBITS}-ecb -K ${AES_KEY} -in ${PLAIN} -out ${CIPHER} -nosalt -nopad

# 将cipher解密为decrypt
openssl enc -d -aes-${KEYLENBITS}-ecb -K ${AES_KEY} -in ${CIPHER} -out ${DECRYPT} -nosalt -nopad

# 比较plain与decrypt
diff ${PLAIN} ${DECRYPT}
if [ $? -ne 0 ]; then
        echo " encrypt/decrypt failed"
        exit
fi

echo plain:
hexdump -C ${PLAIN}
echo enc:
hexdump -C ${CIPHER}
echo dec:
hexdump -C ${DECRYPT}
