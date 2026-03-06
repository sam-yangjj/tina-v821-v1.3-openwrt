#CROSS_COMPILE := arm-anykav500-linux-uclibcgnueabi-
#CROSS_COMPILE := arm-buildroot-linux-uclibcgnueabihf-
CROSS_COMPILE := arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RANLIB := $(CROSS_COMPILE)ranlib

TARGET_NAME ?= test_tkl_storage

PWD := $(shell pwd)
BUILD_PATH := $(PWD)/build


#CFLAGS := -I$(PWD)/../../xz/include -I$(PWD)/../../xz/ -I$(PWD)/../../include -I$(PWD)/../../mtd-utils/include -fPIC
#LDFLAGS := -L$(PWD)/../../xz/lib -L$(PWD)/../../src/lib -L$(PWD)/../../mtd-utils/lib/lib -ldiff2ya -lxz -lmtd

SOURCE := storage_test.c tkl_storage_api.c df.c fdisk.c


all:
	$(CC) -o $(TARGET_NAME) $(SOURCE) $(CFLAGS) $(LDFLAGS)


clean:
	rm -rf *.o $(TARGET_NAME)

.PHONY: all clean

