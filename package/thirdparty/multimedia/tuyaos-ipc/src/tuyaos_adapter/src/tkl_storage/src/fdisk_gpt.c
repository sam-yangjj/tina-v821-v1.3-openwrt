static UINT32_T *global_crc32_table;

UINT32_T*  crc32_filltable(UINT32_T *crc_table, int endian)
{
	UINT32_T polynomial = endian ? 0x04c11db7 : 0xedb88320;
	UINT32_T c;
	unsigned i, j;

	if (!crc_table)
		crc_table = xmalloc(256 * sizeof(UINT32_T));

	for (i = 0; i < 256; i++) {
		c = endian ? (i << 24) : i;
		for (j = 8; j; j--) {
			if (endian)
				c = (c&0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
			else
				c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);
		}
		*crc_table++ = c;
	}

	return crc_table - 256;
}
/* Common uses: */
UINT32_T*  crc32_new_table_le(void)
{
	return crc32_filltable(NULL, 0);
}
UINT32_T*  global_crc32_new_table_le(void)
{
	global_crc32_table = crc32_new_table_le();
	return global_crc32_table;
}

UINT32_T  crc32_block_endian0(UINT32_T val, const void *buf, unsigned len, UINT32_T *crc_table)
{
	const void *end = (PUINT8_T)buf + len;

	while (buf != end) {
		val = crc_table[(UINT8_T)val ^ *(PUINT8_T)buf] ^ (val >> 8);
		buf = (PUINT8_T)buf + 1;
	}
	return val;
}

/*
 * Copyright (C) 2010 Kevin Cernekee <cernekee@gmail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

#define GPT_MAGIC 0x5452415020494645ULL
enum {
	LEGACY_GPT_TYPE = 0xee,
	GPT_MAX_PARTS   = 256,
	GPT_MAX_PART_ENTRY_LEN = 4096,
	GUID_LEN        = 16,
};

typedef struct {
	UINT64_T magic;
	UINT32_T revision;
	UINT32_T hdr_size;
	UINT32_T hdr_crc32;
	UINT32_T reserved;
	UINT64_T current_lba;
	UINT64_T backup_lba;
	UINT64_T first_usable_lba;
	UINT64_T last_usable_lba;
	UINT8_T  disk_guid[GUID_LEN];
	UINT64_T first_part_lba;
	UINT32_T n_parts;
	UINT32_T part_entry_len;
	UINT32_T part_array_crc32;
} gpt_header;

typedef struct {
	UINT8_T  type_guid[GUID_LEN];
	UINT8_T  part_guid[GUID_LEN];
	UINT64_T lba_start;
	UINT64_T lba_end;
	UINT64_T flags;
	UINT16_T name36[36];
} gpt_partition;

static gpt_header *gpt_hdr;

static char *part_array;
static unsigned int n_parts;
static unsigned int part_entry_len;

static inline gpt_partition *
gpt_part(int i)
{
	if (i >= n_parts) {
		return NULL;
	}
	return (gpt_partition *)&part_array[i * part_entry_len];
}

static UINT32_T
gpt_crc32(void *buf, int len)
{
	return ~crc32_block_endian0(0xffffffff, buf, len, global_crc32_table);
}

static void
gpt_print_guid(PUINT8_T buf)
{
	printf(
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		buf[3], buf[2], buf[1], buf[0],
		buf[5], buf[4],
		buf[7], buf[6],
		buf[8], buf[9],
		buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}

static void
gpt_print_wide36(PUINT16_T s)
{
#if ENABLE_UNICODE_SUPPORT
	char buf[37 * 4];
	wchar_t wc[37];
	int i = 0;
	while (i < ARRAY_SIZE(wc)-1) {
		if (s[i] == 0)
			break;
		wc[i] = s[i];
		i++;
	}
	wc[i] = 0;
	if (wcstombs(buf, wc, sizeof(buf)) <= sizeof(buf)-1)
		fputs(printable_string(buf), stdout);
#else
	char buf[37];
	int i = 0;
	while (i < ARRAY_SIZE(buf)-1) {
		if (s[i] == 0)
			break;
		buf[i] = (0x20 <= s[i] && s[i] < 0x7f) ? s[i] : '?';
		i++;
	}
	buf[i] = '\0';
	fputs(buf, stdout);
#endif
}


/* vda's implementations of the similar idea */

/* Convert unsigned long long value into compact 5-char representation.
 * String is not terminated (buf[5] is untouched) */
char*  smart_ulltoa5(unsigned long long ul, char buf[5], const char *scale)
{
	const char *fmt;
	char c;
	unsigned v, u, idx = 0;

	if (ul > 99999) { // do not scale if 99999 or less
		ul *= 10;
		do {
			ul /= 1024;
			idx++;
		} while (ul >= 100000);
	}
	v = ul; // ullong divisions are expensive, avoid them

	fmt = " 123456789";
	u = v / 10;
	v = v % 10;
	if (!idx) {
		// 99999 or less: use "12345" format
		// u is value/10, v is last digit
		c = buf[0] = " 123456789"[u/1000];
		if (c != ' ') fmt = "0123456789";
		c = buf[1] = fmt[u/100%10];
		if (c != ' ') fmt = "0123456789";
		c = buf[2] = fmt[u/10%10];
		if (c != ' ') fmt = "0123456789";
		buf[3] = fmt[u%10];
		buf[4] = "0123456789"[v];
	} else {
		// value has been scaled into 0..9999.9 range
		// u is value, v is 1/10ths (allows for 92.1M format)
		if (u >= 100) {
			// value is >= 100: use "1234M', " 123M" formats
			c = buf[0] = " 123456789"[u/1000];
			if (c != ' ') fmt = "0123456789";
			c = buf[1] = fmt[u/100%10];
			if (c != ' ') fmt = "0123456789";
			v = u % 10;
			u = u / 10;
			buf[2] = fmt[u%10];
		} else {
			// value is < 100: use "92.1M" format
			c = buf[0] = " 123456789"[u/10];
			if (c != ' ') fmt = "0123456789";
			buf[1] = fmt[u%10];
			buf[2] = '.';
		}
		buf[3] = "0123456789"[v];
		buf[4] = scale[idx]; /* typically scale = " kmgt..." */
	}
	return buf + 5;
}

void gpt_list_table(int xtra)
{
	int i;
	char numstr6[6];

	smart_ulltoa5(total_number_of_sectors * sector_size, numstr6, " KMGTPEZY")[0] = '\0';
	printf("Disk %s: %llu sectors, %s\n", disk_device,
		(unsigned long long)total_number_of_sectors,
		numstr6);
	printf("Logical sector size: %u\n", sector_size);
	printf("Disk identifier (GUID): ");
	gpt_print_guid(gpt_hdr->disk_guid);
	printf("\nPartition table holds up to %u entries\n",
		(int)SWAP_LE32(gpt_hdr->n_parts));
	printf("First usable sector is %llu, last usable sector is %llu\n\n",
		(unsigned long long)SWAP_LE64(gpt_hdr->first_usable_lba),
		(unsigned long long)SWAP_LE64(gpt_hdr->last_usable_lba));

/* "GPT fdisk" has a concept of 16-bit extension of the original MBR 8-bit type codes,
 * which it displays here: its output columns are ... Size Code Name
 * They are their own invention and are not stored on disk.
 * Looks like they use them to support "hybrid" GPT: for example, they have
 *   AddType(0x8307, "69DAD710-2CE4-4E3C-B16C-21A1D49ABED3", "Linux ARM32 root (/)");
 * and then (code>>8) matches what you need to put into MBR's type field for such a partition.
 * To print those codes, we'd need a GUID lookup table. Lets just drop the "Code" column instead:
 */
	puts("Number  Start (sector)    End (sector)  Size Name");
	//    123456 123456789012345 123456789012345 12345 abc
	for (i = 0; i < n_parts; i++) {
		gpt_partition *p = gpt_part(i);
		if (p->lba_start) {
			smart_ulltoa5((1 + SWAP_LE64(p->lba_end) - SWAP_LE64(p->lba_start)) * sector_size,
				numstr6, " KMGTPEZY")[0] = '\0';
			printf("%6u %15llu %15llu %s ",
				i + 1,
				(unsigned long long)SWAP_LE64(p->lba_start),
				(unsigned long long)SWAP_LE64(p->lba_end),
				numstr6
			);
			gpt_print_wide36(p->name36);
			putchar('\n');
		}
	}
}

static int check_gpt_label(void)
{
	unsigned part_array_len;
	struct partition *first = pt_offset(MBRbuffer, 0);
	struct pte pe;
	UINT32_T crc;

	/* LBA 0 contains the legacy MBR */

	if (!valid_part_table_flag(MBRbuffer)
	 || first->sys_ind != LEGACY_GPT_TYPE
	) {
		current_label_type = 0;
		return 0;
	}

	/* LBA 1 contains the GPT header */

	read_pte(&pe, 1);
	gpt_hdr = (void *)pe.sectorbuffer;

	if (gpt_hdr->magic != SWAP_LE64(GPT_MAGIC)) {
		current_label_type = 0;
		return 0;
	}

	if (!global_crc32_table) {
		global_crc32_new_table_le();
	}

	crc = SWAP_LE32(gpt_hdr->hdr_crc32);
	gpt_hdr->hdr_crc32 = 0;
	if (gpt_crc32(gpt_hdr, SWAP_LE32(gpt_hdr->hdr_size)) != crc) {
		/* FIXME: read the backup table */
		puts("\nwarning: GPT header CRC is invalid\n");
	}

	n_parts = SWAP_LE32(gpt_hdr->n_parts);
	part_entry_len = SWAP_LE32(gpt_hdr->part_entry_len);
	if (n_parts > GPT_MAX_PARTS
	 || part_entry_len > GPT_MAX_PART_ENTRY_LEN
	 || SWAP_LE32(gpt_hdr->hdr_size) > sector_size
	) {
		puts("\nwarning: unable to parse GPT disklabel\n");
		current_label_type = 0;
		return 0;
	}

	part_array_len = n_parts * part_entry_len;
	part_array = xmalloc(part_array_len);
	seek_sector(SWAP_LE64(gpt_hdr->first_part_lba));
	if (full_read(dev_fd, part_array, part_array_len) != part_array_len) {
		fdisk_fatal(unable_to_read);
	}

	if (gpt_crc32(part_array, part_array_len) != gpt_hdr->part_array_crc32) {
		/* FIXME: read the backup table */
		puts("\nwarning: GPT array CRC is invalid\n");
	}

	//puts("Found valid GPT with protective MBR; using GPT\n");

	current_label_type = LABEL_GPT;
	return 1;
}