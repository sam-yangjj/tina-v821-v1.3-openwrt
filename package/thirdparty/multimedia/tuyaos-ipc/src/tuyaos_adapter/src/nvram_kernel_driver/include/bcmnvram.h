/*
 * NVRAM variable manipulation
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmnvram.h,v 1.1 2007/06/08 07:40:19 arthur Exp $
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_CLEAR_MAGIC	0x0
#define NVRAM_INVALID_MAGIC	0xFFFFFFFF
#define NVRAM_VERSION		1
#define NVRAM_DRIVER_VERSION    "0.08"




#define NVRAM_SPACE		(NVRAM_FLASH_SIZE-NVRAM_FLASH_OFFSET)
#define NVRAM_VALUES_SPACE      (NVRAM_SPACE*2)

#define NVRAM_MAX_PARAM_LEN	64
#define NVRAM_MAX_VALUE_LEN	256

#define NVRAM_MAJOR		241

#define NVRAM_IOCTL_COMMIT	10
#define NVRAM_IOCTL_CLEAR	20
#define NVRAM_IOCTL_SET		30
#define NVRAM_IOCTL_GET		40
#define NVRAM_IOCTL_ERASE	50
#define NVRAM_IOCTL_WATCH	60


#define ATTR_LOCK		(1 << 0)
#define ATTR_PERSIST		(1 << 1)
#define ATTR_TEMP		(1 << 2)


#if defined(__KERNEL__) || defined(__UBOOT__) || defined(__TOOL__)

#if defined(__TOOL__)
#include <stdint.h>
#endif

#include <linux/types.h>

#define CRC8_INIT_VALUE		(0xff)
#define htol32(i)		(i)
#define ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

struct nvram_tuple {
	char *name;
	char *value;
	uint32_t val_len:31,
	         val_tmp:1;
	uint32_t flag;
	struct nvram_tuple *next;
};

#else

#include <stdint.h>

struct nvram_pair {
	char *name;
	char *value;
};

#ifdef USE_RET_VAL_MODE
extern char *nvram_get(const char *name);
extern char *nvram_safe_get(const char *name);
#else
extern int nvram_get(const char *name, char *value);
extern int nvram_set_with_attr(const char *name, const char *value, int attr);
#endif

extern int nvram_get_int(const char *name);
extern int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max);

extern int nvram_getall(char *buf, int count, int include_temp);

extern int nvram_set(const char *name, const char *value);
extern int nvram_set_int(const char *name, int value);
extern int nvram_unset(const char *name);

extern int nvram_set_temp(const char *name, const char *value);
extern int nvram_set_int_temp(const char *name, int value);

extern int nvram_match(const char *name, char *match);
extern int nvram_invmatch(const char *name, char *invmatch);

extern int nvram_commit(void);
extern int nvram_clear(void);
extern int nvram_erase(void);

#endif

struct nvram_header {
	uint32_t magic;
	uint32_t len;
	uint32_t version;
	int space_used;
	unsigned short chksum_protected;
	unsigned short chksum;
	unsigned int write_counter;
	char reserved[4];
} __attribute__ ((packed));

#define NVRAM_HEADER_SIZE  sizeof(struct nvram_header)

typedef struct anvram_ioctl_s {
	int size;
	int is_temp;
	int len_param;
	int len_value;
	char *param;
	char *value;
} anvram_ioctl_t;

typedef struct anvram_param_s {
    anvram_ioctl_t nvram_ioctl;
}anvram_param_t;

typedef struct anvram_kwait_s {
    int cnt;
    anvram_param_t *paramp;
}anvram_kwait_t;

#endif /* _bcmnvram_h_ */
