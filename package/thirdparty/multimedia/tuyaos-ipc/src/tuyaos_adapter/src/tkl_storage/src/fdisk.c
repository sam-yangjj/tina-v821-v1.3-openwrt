#ifndef _LARGEFILE64_SOURCE
/* For lseek64 */
# define _LARGEFILE64_SOURCE
#endif
#include <assert.h>             /* assert */
#include <sys/mount.h>
#if !defined(BLKSSZGET)
# define BLKSSZGET _IO(0x12, 104)
#endif
#if !defined(BLKGETSIZE64)
# define BLKGETSIZE64 _IOR(0x12,114,size_t)
#endif

#include <mntent.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mntent.h>
#include <sys/types.h>    
#include <sys/stat.h>
#include <stdbool.h> 
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <setjmp.h>
#include <errno.h>

#include <sys/sysmacros.h>

#include <tkl_storage.h>

//#define isdigit(a) ((unsigned char)((a) - '0') <= 9)

/* Tested to work correctly with all int types (IIRC :]) */
#define MAXINT(T) (T)( \
	((T)-1) > 0 \
	? (T)-1 \
	: (T)~((T)1 << (sizeof(T)*8-1)) \
	)

#define MININT(T) (T)( \
	((T)-1) > 0 \
	? (T)0 \
	: ((T)1 << (sizeof(T)*8-1)) \
	)

typedef unsigned long uoff_t;

typedef unsigned int smallint;

#define bb_dev_null "/dev/null"

#if BB_LITTLE_ENDIAN
# define inline_if_little_endian ALWAYS_INLINE
#else
# define inline_if_little_endian /* nothing */
#endif

/* Looks like someone forgot to add this to config system */
#ifndef ENABLE_FEATURE_FDISK_BLKSIZE
# define ENABLE_FEATURE_FDISK_BLKSIZE 0
# define IF_FEATURE_FDISK_BLKSIZE(a)
#endif

#define DEFAULT_SECTOR_SIZE      512
#define DEFAULT_SECTOR_SIZE_STR "512"
#define MAX_SECTOR_SIZE         2048
#define SECTOR_SIZE              512 /* still used in osf/sgi/sun code */
#define MAXIMUM_PARTS             60

#define ACTIVE_FLAG             0x80

#define EXTENDED                0x05
#define WIN98_EXTENDED          0x0f
#define LINUX_PARTITION         0x81
#define LINUX_SWAP              0x82
#define LINUX_NATIVE            0x83
#define LINUX_EXTENDED          0x85
#define LINUX_LVM               0x8e
#define LINUX_RAID              0xfd


enum {
	OPT_b = 1 << 0,
	OPT_C = 1 << 1,
	OPT_H = 1 << 2,
	OPT_l = 1 << 3,
	OPT_S = 1 << 4,
	OPT_u = 1 << 5,
	OPT_s = (1 << 6) * ENABLE_FEATURE_FDISK_BLKSIZE,
};


typedef unsigned long long ullong;
/* Used for sector numbers. Partition formats we know
 * do not support more than 2^32 sectors
 */
typedef UINT32_T sector_t;
#if UINT_MAX == 0xffffffff
# define SECT_FMT ""
#elif ULONG_MAX == 0xffffffff
# define SECT_FMT "l"
#else
# error Cant detect sizeof(UINT32_T)
#endif

struct hd_geometry {
	unsigned char heads;
	unsigned char sectors;
	unsigned short cylinders;
	unsigned long start;
};

#define HDIO_GETGEO     0x0301  /* get device geometry */

#if 0
/* TODO: #if ENABLE_FEATURE_FDISK_WRITABLE */
/* (currently fdisk_sun/sgi.c do not have proper WRITABLE #ifs) */
static const char msg_building_new_label[]  =
"Building a new %s. Changes will remain in memory only,\n"
"until you decide to write them. After that the previous content\n"
"won't be recoverable.\n\n";

static const char msg_part_already_defined[]  =
"Partition %u is already defined, delete it before re-adding\n";
/* #endif */
#endif


struct partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* what partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
} PACKED;

/*
 * per partition table entry data
 *
 * The four primary partitions have the same sectorbuffer (MBRbuffer)
 * and have NULL ext_pointer.
 * Each logical partition table entry has two pointers, one for the
 * partition and one link to the next one.
 */
struct pte {
	struct partition *part_table;   /* points into sectorbuffer */
	struct partition *ext_pointer;  /* points into sectorbuffer */
	sector_t offset_from_dev_start; /* disk sector number */
	char *sectorbuffer;             /* disk sector contents */
};

#define unable_to_open "can't open '%s'"
#define unable_to_read "can't read from %s"
#define unable_to_seek "can't seek on %s"

enum label_type {
	LABEL_DOS, LABEL_SUN, LABEL_SGI, LABEL_AIX, LABEL_OSF, LABEL_GPT
};

#define LABEL_IS_DOS	(LABEL_DOS == current_label_type)

#define LABEL_IS_GPT	(LABEL_GPT == current_label_type)
#define STATIC_GPT static

enum action { OPEN_MAIN, TRY_ONLY, CREATE_EMPTY_DOS, CREATE_EMPTY_SUN };

static void update_units(void);
// static const char *partition_type(unsigned char type);
static void get_geometry(void);
static void read_pte(struct pte *pe, sector_t offset);
static int get_boot(void);

#define PLURAL   0
#define SINGULAR 1

static sector_t get_start_sect(const struct partition *p);
static sector_t get_nr_sects(const struct partition *p);

/* DOS partition types */
#if 0
static const char *const i386_sys_types[] = {
	"\x00" "Empty",
	"\x01" "FAT12",
	"\x04" "FAT16 <32M",
	"\x05" "Extended",         /* DOS 3.3+ extended partition */
	"\x06" "FAT16",            /* DOS 16-bit >=32M */
	"\x07" "HPFS/NTFS",        /* OS/2 IFS, eg, HPFS or NTFS or QNX */
	"\x0a" "OS/2 Boot Manager",/* OS/2 Boot Manager */
	"\x0b" "Win95 FAT32",
	"\x0c" "Win95 FAT32 (LBA)",/* LBA really is 'Extended Int 13h' */
	"\x0e" "Win95 FAT16 (LBA)",
	"\x0f" "Win95 Ext'd (LBA)",
	"\x11" "Hidden FAT12",
	"\x12" "Compaq diagnostics",
	"\x14" "Hidden FAT16 <32M",
	"\x16" "Hidden FAT16",
	"\x17" "Hidden HPFS/NTFS",
	"\x1b" "Hidden Win95 FAT32",
	"\x1c" "Hidden W95 FAT32 (LBA)",
	"\x1e" "Hidden W95 FAT16 (LBA)",
	"\x3c" "Part.Magic recovery",
	"\x41" "PPC PReP Boot",
	"\x42" "SFS",
	"\x63" "GNU HURD or SysV", /* GNU HURD or Mach or Sys V/386 (such as ISC UNIX) */
	"\x80" "Old Minix",        /* Minix 1.4a and earlier */
	"\x81" "Minix / old Linux",/* Minix 1.4b and later */
	"\x82" "Linux swap",       /* also Solaris */
	"\x83" "Linux",
	"\x84" "OS/2 hidden C: drive",
	"\x85" "Linux extended",
	"\x86" "NTFS volume set",
	"\x87" "NTFS volume set",
	"\x8e" "Linux LVM",
	"\x9f" "BSD/OS",           /* BSDI */
	"\xa0" "Thinkpad hibernation",
	"\xa5" "FreeBSD",          /* various BSD flavours */
	"\xa6" "OpenBSD",
	"\xa8" "Darwin UFS",
	"\xa9" "NetBSD",
	"\xab" "Darwin boot",
	"\xb7" "BSDI fs",
	"\xb8" "BSDI swap",
	"\xbe" "Solaris boot",
	"\xeb" "BeOS fs",
	"\xee" "EFI GPT",                    /* Intel EFI GUID Partition Table */
	"\xef" "EFI (FAT-12/16/32)",         /* Intel EFI System Partition */
	"\xf0" "Linux/PA-RISC boot",         /* Linux/PA-RISC boot loader */
	"\xf2" "DOS secondary",              /* DOS 3.3+ secondary */
	"\xfd" "Linux raid autodetect",      /* New (2.2.x) raid partition with
						autodetect using persistent
						superblock */
#if 0 /* ENABLE_WEIRD_PARTITION_TYPES */
	"\x02" "XENIX root",
	"\x03" "XENIX usr",
	"\x08" "AIX",              /* AIX boot (AIX -- PS/2 port) or SplitDrive */
	"\x09" "AIX bootable",     /* AIX data or Coherent */
	"\x10" "OPUS",
	"\x18" "AST SmartSleep",
	"\x24" "NEC DOS",
	"\x39" "Plan 9",
	"\x40" "Venix 80286",
	"\x4d" "QNX4.x",
	"\x4e" "QNX4.x 2nd part",
	"\x4f" "QNX4.x 3rd part",
	"\x50" "OnTrack DM",
	"\x51" "OnTrack DM6 Aux1", /* (or Novell) */
	"\x52" "CP/M",             /* CP/M or Microport SysV/AT */
	"\x53" "OnTrack DM6 Aux3",
	"\x54" "OnTrackDM6",
	"\x55" "EZ-Drive",
	"\x56" "Golden Bow",
	"\x5c" "Priam Edisk",
	"\x61" "SpeedStor",
	"\x64" "Novell Netware 286",
	"\x65" "Novell Netware 386",
	"\x70" "DiskSecure Multi-Boot",
	"\x75" "PC/IX",
	"\x93" "Amoeba",
	"\x94" "Amoeba BBT",       /* (bad block table) */
	"\xa7" "NeXTSTEP",
	"\xbb" "Boot Wizard hidden",
	"\xc1" "DRDOS/sec (FAT-12)",
	"\xc4" "DRDOS/sec (FAT-16 < 32M)",
	"\xc6" "DRDOS/sec (FAT-16)",
	"\xc7" "Syrinx",
	"\xda" "Non-FS data",
	"\xdb" "CP/M / CTOS / ...",/* CP/M or Concurrent CP/M or
	                              Concurrent DOS or CTOS */
	"\xde" "Dell Utility",     /* Dell PowerEdge Server utilities */
	"\xdf" "BootIt",           /* BootIt EMBRM */
	"\xe1" "DOS access",       /* DOS access or SpeedStor 12-bit FAT
	                              extended partition */
	"\xe3" "DOS R/O",          /* DOS R/O or SpeedStor */
	"\xe4" "SpeedStor",        /* SpeedStor 16-bit FAT extended
	                              partition < 1024 cyl. */
	"\xf1" "SpeedStor",
	"\xf4" "SpeedStor",        /* SpeedStor large partition */
	"\xfe" "LANstep",          /* SpeedStor >1024 cyl. or LANstep */
	"\xff" "BBT",              /* Xenix Bad Block Table */
#endif
	NULL
};
#endif
#if 0
enum {
	dev_fd = 3                  /* the disk */
};
#else
static int dev_fd=0;
#endif

/* Globals */
struct globals {
	char *line_ptr;

	const char *disk_device;
	int g_partitions; // = 4;       /* maximum partition + 1 */
	unsigned units_per_sector; // = 1;
	unsigned sector_size; // = DEFAULT_SECTOR_SIZE;
	unsigned user_set_sector_size;
	unsigned sector_offset; // = 1;
	unsigned g_heads, g_sectors, g_cylinders;
	smallint /* enum label_type */ current_label_type;
	smallint display_in_cyl_units;
#if ENABLE_FEATURE_OSF_LABEL
	smallint possibly_osf_label;
#endif

	smallint listing;               /* no aborts for fdisk -l */
	smallint dos_compatible_flag; // = 1;
#if ENABLE_FEATURE_FDISK_WRITABLE
	//int dos_changed;
	smallint nowarn;                /* no warnings for fdisk -l/-s */
#endif
	int ext_index;                  /* the prime extended partition */
	unsigned user_cylinders, user_heads, user_sectors;
	unsigned pt_heads, pt_sectors;
	unsigned kern_heads, kern_sectors;
	sector_t extended_offset;       /* offset of link pointers */
	//sector_t total_number_of_sectors;
	unsigned long long total_number_of_sectors;

	jmp_buf listingbuf;
	char line_buffer[80];
	/* Raw disk label. For DOS-type partition tables the MBR,
	 * with descriptions of the primary partitions. */
	char MBRbuffer[MAX_SECTOR_SIZE];
	/* Partition tables */
	struct pte ptes[MAXIMUM_PARTS];
};

struct globals *ptr_to_globals;

#define G (*ptr_to_globals)
#define line_ptr             (G.line_ptr            )
#define disk_device          (G.disk_device         )
#define g_partitions         (G.g_partitions        )
#define units_per_sector     (G.units_per_sector    )
#define sector_size          (G.sector_size         )
#define user_set_sector_size (G.user_set_sector_size)
#define sector_offset        (G.sector_offset       )
#define g_heads              (G.g_heads             )
#define g_sectors            (G.g_sectors           )
#define g_cylinders          (G.g_cylinders         )
#define current_label_type   (G.current_label_type  )
#define display_in_cyl_units (G.display_in_cyl_units)
#define possibly_osf_label   (G.possibly_osf_label  )
#define listing                 (G.listing                )
#define dos_compatible_flag     (G.dos_compatible_flag    )
#define nowarn                  (G.nowarn                 )
#define ext_index               (G.ext_index              )
#define user_cylinders          (G.user_cylinders         )
#define user_heads              (G.user_heads             )
#define user_sectors            (G.user_sectors           )
#define pt_heads                (G.pt_heads               )
#define pt_sectors              (G.pt_sectors             )
#define kern_heads              (G.kern_heads             )
#define kern_sectors            (G.kern_sectors           )
#define extended_offset         (G.extended_offset        )
#define total_number_of_sectors (G.total_number_of_sectors)
#define listingbuf      (G.listingbuf     )
#define line_buffer     (G.line_buffer    )
#define MBRbuffer       (G.MBRbuffer      )
#define ptes            (G.ptes           )

#define barrier() __asm__ __volatile__("":::"memory")
#define SET_PTR_TO_GLOBALS(x) do { \
     (*(struct globals**)&ptr_to_globals) = (void*)(x); \
     barrier(); \
 } while (0)

#define INIT_G() do { \
	SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
	sector_size = DEFAULT_SECTOR_SIZE; \
	sector_offset = 1; \
	g_partitions = 4; \
	units_per_sector = 1; \
	dos_compatible_flag = 1; \
} while (0)

ssize_t  safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A short read is returned on an end of file.
 */
ssize_t  full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}


// Die if we can't allocate size bytes of memory.
void*  xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL && size != 0)
		printf("malloc failed\n");
	return ptr;
}

// Die if we can't allocate and zero size bytes of memory.
void*  xzalloc(size_t size)
{
	void *ptr = xmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void  xmove_fd(int from, int to)
{
	if (from == to)
		return;
	dup2(from, to); // dangerous
	close(from);
}

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))


/* SWAP_LEnn means "convert CPU<->little_endian by swapping bytes" */
#if BB_BIG_ENDIAN
# define SWAP_BE16(x) (x)
# define SWAP_BE32(x) (x)
# define SWAP_BE64(x) (x)
# define SWAP_LE16(x) bswap_16(x)
# define SWAP_LE32(x) bswap_32(x)
# define SWAP_LE64(x) bb_bswap_64(x)
# define IF_BIG_ENDIAN(...) __VA_ARGS__
# define IF_LITTLE_ENDIAN(...)
#else
# define SWAP_BE16(x) bswap_16(x)
# define SWAP_BE32(x) bswap_32(x)
# define SWAP_BE64(x) bb_bswap_64(x)
# define SWAP_LE16(x) (x)
# define SWAP_LE32(x) (x)
# define SWAP_LE64(x) (x)
# define IF_BIG_ENDIAN(...)
# define IF_LITTLE_ENDIAN(...) __VA_ARGS__
#endif

typedef UINT32_T bb__aliased_uint32_t;
#define move_from_unaligned32(v, u32p) ((v) = *(bb__aliased_uint32_t*)(u32p))

char*  auto_string(char *str)
{
	static char *saved[4];
	static UINT8_T cur_saved; /* = 0 */

	free(saved[cur_saved]);
	saved[cur_saved] = str;
	cur_saved = (cur_saved + 1) & (ARRAY_SIZE(saved)-1);

	return str;
}

static unsigned long long BLKGETSIZE_sectors(int fd)
{
	UINT64_T v64;

	if (ioctl(fd, BLKGETSIZE64, &v64) == 0) {
		/* Got bytes, convert to 512 byte sectors */
		v64 >>= 9;
		if (v64 != (sector_t)v64) {
			//printf("device has more than 2^32 sectors\n");
			//v64 = (UINT32_T)-1L;
		}
		return v64;
	}

	return 0;
}


#define IS_EXTENDED(i) \
	((i) == EXTENDED || (i) == WIN98_EXTENDED || (i) == LINUX_EXTENDED)

#define cround(n)       (display_in_cyl_units ? ((n)/units_per_sector)+1 : (n))

#define scround(x)      (((x)+units_per_sector-1)/units_per_sector)

#define pt_offset(b, n) \
	((struct partition *)((b) + 0x1be + (n) * sizeof(struct partition)))

#define sector(s)       ((s) & 0x3f)

#define cylinder(s, c)  ((c) | (((s) & 0xc0) << 2))

static void
close_dev_fd(void)
{
	/* Not really closing, but making sure it is open, and to harmless place */
	//xmove_fd(xopen(bb_dev_null, O_RDONLY), dev_fd);
	if(dev_fd)
	{
        close(dev_fd);
        dev_fd=0;
	}
}

#if 0
/* Return partition name */
static const char *
partname(const char *dev, int pno, int lth)
{
	const char *p;
	int w, wp;
	int bufsiz;
	char *bufp;

	bufp = auto_string(xzalloc(80));
	bufsiz = 80;

	w = strlen(dev);
	p = "";

	if (isdigit(dev[w-1]))
		p = "p";

	/* devfs kludge - note: fdisk partition names are not supposed
	   to equal kernel names, so there is no reason to do this */
	if (strcmp(dev + w - 4, "disc") == 0) {
		w -= 4;
		p = "part";
	}

	wp = strlen(p);

	if (lth) {
		snprintf(bufp, bufsiz, "%*.*s%s%-2u",
			lth-wp-2, w, dev, p, pno);
	} else {
		snprintf(bufp, bufsiz, "%.*s%s%-2u", w, dev, p, pno);
	}
	return bufp;
}

static struct partition *
get_part_table(int i)
{
	return ptes[i].part_table;
}

static const char *
str_units(int n)
{
    /* n==1: use singular */
	if (n == 1)
		return display_in_cyl_units ? "cylinder" : "sector";
	return display_in_cyl_units ? "cylinders" : "sectors";
}
#endif

static int
valid_part_table_flag(const char *mbuffer)
{
	return (mbuffer[510] == 0x55 && (UINT8_T)mbuffer[511] == 0xaa);
}

static void fdisk_fatal(const char *why)
{
	if (listing) {
		close_dev_fd();
		longjmp(listingbuf, 1);
	}
	printf("%s: %s %s", __func__, why, disk_device);
}

#define ENABLE_FDISK_SUPPORT_LARGE_DISKS 1
static void seek_sector(sector_t secno)
{
#if ENABLE_FDISK_SUPPORT_LARGE_DISKS
	off64_t off = (off64_t)secno * sector_size;
	if (lseek64(dev_fd, off, SEEK_SET) == (off64_t) -1)
		fdisk_fatal(unable_to_seek);
#else
	UINT64_T off = (UINT64_T)secno * sector_size;
	if (off > MAXINT(off_t)
	 || lseek(dev_fd, (off_t)off, SEEK_SET) == (off_t) -1
	) {
		fdisk_fatal(unable_to_seek);
	}
#endif
}

// STATIC_GPT void gpt_list_table(int xtra);
#include "fdisk_gpt.c"

#if 0
static inline_if_little_endian unsigned
read4_little_endian(const unsigned char *cp)
{
	UINT32_T v;
	move_from_unaligned32(v, cp);
	return SWAP_LE32(v);
}
#endif

static sector_t get_start_sect(const struct partition *p)
{
#if 0    
	return read4_little_endian(p->start4);
#else
    sector_t start=0;
    sector_t tmp1=0, tmp2=0, tmp3=0, tmp4=0;

    // big 2 little end
    tmp1 = p->start4[0];
    tmp2 = p->start4[1];
    tmp3 = p->start4[2];
    tmp4 = p->start4[3];    
    start = (tmp4<<24) | (tmp3<<16) | (tmp2<<8) | (tmp1);

	return start;
#endif
}

static sector_t get_nr_sects(const struct partition *p)
{
#if 0    
	return read4_little_endian(p->size4);
#else
    sector_t size=0;
    sector_t tmp1=0, tmp2=0, tmp3=0, tmp4=0;

    // big 2 little end
    tmp1 = p->size4[0];
    tmp2 = p->size4[1];
    tmp3 = p->size4[2];
    tmp4 = p->size4[3];    
    size = (tmp4<<24) | (tmp3<<16) | (tmp2<<8) | (tmp1);

	return size;
#endif
}

/* Allocate a buffer and read a partition table sector */
static void read_pte(struct pte *pe, sector_t offset)
{
	pe->offset_from_dev_start = offset;
	pe->sectorbuffer = xzalloc(sector_size);
	seek_sector(offset);
	/* xread would make us abort - bad for fdisk -l */
	if (full_read(dev_fd, pe->sectorbuffer, sector_size) != sector_size)
		fdisk_fatal(unable_to_read);
	pe->part_table = pe->ext_pointer = NULL;
}

#if 0
static sector_t get_partition_start_from_dev_start(const struct pte *pe)
{
	return pe->offset_from_dev_start + get_start_sect(pe->part_table);
}

#define get_sys_types() i386_sys_types

static const char *partition_type(unsigned char type)
{
	int i;
	const char *const *types = get_sys_types();

	for (i = 0; types[i]; i++)
		if ((unsigned char)types[i][0] == type)
			return types[i] + 1;

	return "Unknown";
}

static int is_cleared_partition(const struct partition *p)
{
	/* We consider partition "cleared" only if it has only zeros */
	const char *cp = (const char *)p;
	int cnt = sizeof(*p);
	char bits = 0;
	while (--cnt >= 0)
		bits |= *cp++;
	return (bits == 0);
}
#endif

static void clear_partition(struct partition *p)
{
	if (p)
		memset(p, 0, sizeof(*p));
}

static void update_units(void)
{
	int cyl_units = g_heads * g_sectors;

	if (display_in_cyl_units && cyl_units)
		units_per_sector = cyl_units;
	else
		units_per_sector = 1;   /* in sectors */
}

static void read_extended(int ext)
{
	int i;
	struct pte *pex;
	struct partition *p, *q;

	ext_index = ext;
	pex = &ptes[ext];
	pex->ext_pointer = pex->part_table;

	p = pex->part_table;
	if (!get_start_sect(p)) {
		puts("Bad offset in primary extended partition");
		return;
	}

	while (IS_EXTENDED(p->sys_ind)) {
		struct pte *pe = &ptes[g_partitions];

		if (g_partitions >= MAXIMUM_PARTS) {
			/* This is not a Linux restriction, but
			   this program uses arrays of size MAXIMUM_PARTS.
			   Do not try to 'improve' this test. */
			struct pte *pre = &ptes[g_partitions - 1];
			clear_partition(pre->ext_pointer);
			return;
		}

		read_pte(pe, extended_offset + get_start_sect(p));

		if (!extended_offset)
			extended_offset = get_start_sect(p);

		q = p = pt_offset(pe->sectorbuffer, 0);
		for (i = 0; i < 4; i++, p++) if (get_nr_sects(p)) {
			if (IS_EXTENDED(p->sys_ind)) {
				if (pe->ext_pointer)
					printf("Warning: extra link "
						"pointer in partition table"
						" %u\n", g_partitions + 1);
				else
					pe->ext_pointer = p;
			} else if (p->sys_ind) {
				if (pe->part_table)
					printf("Warning: ignoring extra "
						  "data in partition table"
						  " %u\n", g_partitions + 1);
				else
					pe->part_table = p;
			}
		}

		/* very strange code here... */
		if (!pe->part_table) {
			if (q != pe->ext_pointer)
				pe->part_table = q;
			else
				pe->part_table = q + 1;
		}
		if (!pe->ext_pointer) {
			if (q != pe->part_table)
				pe->ext_pointer = q;
			else
				pe->ext_pointer = q + 1;
		}

		p = pe->ext_pointer;
		g_partitions++;
	}
}

static void get_sectorsize(void)
{
	if (!user_set_sector_size) {
		int arg;
		if (ioctl(dev_fd, BLKSSZGET, &arg) == 0)
			sector_size = arg;
		if (sector_size != DEFAULT_SECTOR_SIZE)
			printf("Note: sector size is %u "
				"(not " DEFAULT_SECTOR_SIZE_STR ")\n",
				sector_size);
	}
}

static void get_kernel_geometry(void)
{
	struct hd_geometry geometry;

	if (!ioctl(dev_fd, HDIO_GETGEO, &geometry)) {
		kern_heads = geometry.heads;
		kern_sectors = geometry.sectors;
		/* never use geometry.cylinders - it is truncated */
	}
}

static void get_partition_table_geometry(void)
{
	const unsigned char *bufp = (const unsigned char *)MBRbuffer;
	struct partition *p;
	int i, h, s, hh, ss;
	int first = 1;
	int bad = 0;

	if (!(valid_part_table_flag((char*)bufp)))
		return;

	hh = ss = 0;
	for (i = 0; i < 4; i++) {
		p = pt_offset(bufp, i);
		if (p->sys_ind != 0) {
			h = p->end_head + 1;
			s = (p->end_sector & 077);
			if (first) {
				hh = h;
				ss = s;
				first = 0;
			} else if (hh != h || ss != s)
				bad = 1;
		}
	}

	if (!first && !bad) {
		pt_heads = hh;
		pt_sectors = ss;
	}
}

static void get_geometry(void)
{
	int sec_fac;

	get_sectorsize();
	sec_fac = sector_size / 512;
	g_heads = g_cylinders = g_sectors = 0;
	kern_heads = kern_sectors = 0;
	pt_heads = pt_sectors = 0;

	get_kernel_geometry();
	get_partition_table_geometry();

	g_heads = user_heads ? user_heads :
		pt_heads ? pt_heads :
		kern_heads ? kern_heads : 255;
	g_sectors = user_sectors ? user_sectors :
		pt_sectors ? pt_sectors :
		kern_sectors ? kern_sectors : 63;
	total_number_of_sectors = BLKGETSIZE_sectors(dev_fd);

	sector_offset = 1;
	if (dos_compatible_flag)
		sector_offset = g_sectors;

	g_cylinders = total_number_of_sectors / (g_heads * g_sectors * sec_fac);
	if (!g_cylinders)
		g_cylinders = user_cylinders;
}

/*
 * Opens disk_device and optionally reads MBR.
 *    If what == OPEN_MAIN:
 *      Open device, read MBR.  Abort program on short read.  Create empty
 *      disklabel if the on-disk structure is invalid (WRITABLE mode).
 *    If what == TRY_ONLY:
 *      Open device, read MBR.  Return an error if anything is out of place.
 *      Do not create an empty disklabel.  This is used for the "list"
 *      operations: "fdisk -l /dev/sda" and "fdisk -l" (all devices).
 *    If what == CREATE_EMPTY_*:
 *      This means that get_boot() was called recursively from create_*label().
 *      Do not re-open the device; just set up the ptes array and print
 *      geometry warnings.
 *
 * Returns:
 *   -1: no 0xaa55 flag present (possibly entire disk BSD)
 *    0: found or created label
 *    1: I/O error
 */
static int get_boot(void)
{
	int i, fd;

	g_partitions = 4;
	for (i = 0; i < 4; i++) {
		struct pte *pe = &ptes[i];
		pe->part_table = pt_offset(MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->offset_from_dev_start = 0;
		pe->sectorbuffer = MBRbuffer;
	}

	fd = open(disk_device, O_RDONLY);
	if (fd < 0)
		return -1;

	if (512 != full_read(fd, MBRbuffer, 512)) {
		close(fd);
		return -1;
	}
	//xmove_fd(fd, dev_fd);
	dev_fd = fd;

	get_geometry();
	update_units();

	if (check_gpt_label())
		return 0;
	else if (valid_part_table_flag(MBRbuffer))
	{
#if 1
    	for (i = 0; i < 4; i++) {
    		if (IS_EXTENDED(ptes[i].part_table->sys_ind)) {
    			if (g_partitions != 4)
    				printf("Ignoring extra extended "
    					"partition %u\n", i + 1);
    			else
    				read_extended(i);
    		}
    	}

    	for (i = 3; i < g_partitions; i++) {
    		struct pte *pe = &ptes[i];
    		if (!valid_part_table_flag(pe->sectorbuffer)) {
    			printf("Warning: invalid flag 0x%02x,0x%02x of partition "
    				"table %u will be corrected by w(rite)\n",
    				pe->sectorbuffer[510],
    				pe->sectorbuffer[511],
    				i + 1);
    			//IF_FEATURE_FDISK_WRITABLE(pe->changed = 1;)
    		}
    	}
#endif
		return 1;
    }
	else
		return 2; //private patition table
}

#if 0
/* check_consistency() and linear2chs() added Sat Mar 6 12:28:16 1993,
 * faith@cs.unc.edu, based on code fragments from pfdisk by Gordon W. Ross,
 * Jan.  1990 (version 1.2.1 by Gordon W. Ross Aug. 1990; Modified by S.
 * Lubkin Oct.  1991). */

static void linear2chs(unsigned ls, unsigned *c, unsigned *h, unsigned *s)
{
	int spc = g_heads * g_sectors;

	*c = ls / spc;
	ls = ls % spc;
	*h = ls / g_sectors;
	*s = ls % g_sectors + 1;  /* sectors count from 1 */
}

static void check_consistency(const struct partition *p, int partition)
{
	unsigned pbc, pbh, pbs;          /* physical beginning c, h, s */
	unsigned pec, peh, pes;          /* physical ending c, h, s */
	unsigned lbc, lbh, lbs;          /* logical beginning c, h, s */
	unsigned lec, leh, les;          /* logical ending c, h, s */

	if (!g_heads || !g_sectors || (partition >= 4))
		return;         /* do not check extended partitions */

/* physical beginning c, h, s */
	pbc = cylinder(p->sector, p->cyl);
	pbh = p->head;
	pbs = sector(p->sector);

/* physical ending c, h, s */
	pec = cylinder(p->end_sector, p->end_cyl);
	peh = p->end_head;
	pes = sector(p->end_sector);

/* compute logical beginning (c, h, s) */
	linear2chs(get_start_sect(p), &lbc, &lbh, &lbs);

/* compute logical ending (c, h, s) */
	linear2chs(get_start_sect(p) + get_nr_sects(p) - 1, &lec, &leh, &les);

/* Same physical / logical beginning? */
	if (g_cylinders <= 1024 && (pbc != lbc || pbh != lbh || pbs != lbs)) {
		printf("Partition %u has different physical/logical "
			"start (non-Linux?):\n", partition + 1);
		printf("     phys=(%u,%u,%u) ", pbc, pbh, pbs);
		printf("logical=(%u,%u,%u)\n", lbc, lbh, lbs);
	}

/* Same physical / logical ending? */
	if (g_cylinders <= 1024 && (pec != lec || peh != leh || pes != les)) {
		printf("Partition %u has different physical/logical "
			"end:\n", partition + 1);
		printf("     phys=(%u,%u,%u) ", pec, peh, pes);
		printf("logical=(%u,%u,%u)\n", lec, leh, les);
	}
}

static void list_disk_geometry(void)
{
	ullong bytes = ((ullong)total_number_of_sectors << 9);
	ullong xbytes = bytes / (1024*1024);
	char x = 'M';
    char *units;

	if (xbytes >= 10000) {
		xbytes += 512; /* fdisk util-linux 2.28 does this */
		xbytes /= 1024;
		x = 'G';
	}
    
    //units = str_units(PLURAL);
	if (PLURAL == 1)
		units =  display_in_cyl_units ? "cylinder" : "sector";

	units = display_in_cyl_units ? "cylinders" : "sectors";

	printf("Disk %s: %llu %cB, %llu bytes, %llu sectors\n", disk_device, xbytes, x, bytes, total_number_of_sectors);
	printf("%u cylinders, %u heads, %u sectors/track\n", g_cylinders, g_heads, g_sectors);
	printf("Units: %s of %u * %u = %u bytes\n",units, units_per_sector, sector_size, units_per_sector * sector_size);
}

/*
 * Check whether partition entries are ordered by their starting positions.
 * Return 0 if OK. Return i if partition i should have been earlier.
 * Two separate checks: primary and logical partitions.
 */
static int wrong_p_order(int *prev)
{
	const struct pte *pe;
	const struct partition *p;
	sector_t last_p_start_pos = 0, p_start_pos;
	unsigned i, last_i = 0;

	for (i = 0; i < g_partitions; i++) {
		if (i == 4) {
			last_i = 4;
			last_p_start_pos = 0;
		}
		pe = &ptes[i];
		p = pe->part_table;
		if (p->sys_ind) {
			p_start_pos = get_partition_start_from_dev_start(pe);

			if (last_p_start_pos > p_start_pos) {
				if (prev)
					*prev = last_i;
				return i;
			}

			last_p_start_pos = p_start_pos;
			last_i = i;
		}
	}
	return 0;
}

static const char *chs_string11(unsigned cyl, unsigned head, unsigned sect)
{
	char *buf = auto_string(xzalloc(sizeof(int)*3 * 3));
	sprintf(buf, "%u,%u,%u", cylinder(sect,cyl), head, sector(sect));
	return buf;
}

static void list_table(int xtra)
{
	// int i, w;

	if (LABEL_IS_GPT) {
		gpt_list_table(xtra);
		return;
	}

	list_disk_geometry();
#if 0
	/* Heuristic: we list partition 3 of /dev/foo as /dev/foo3,
	 * but if the device name ends in a digit, say /dev/foo1,
	 * then the partition is called /dev/foo1p3.
	 */
	w = strlen(disk_device);
	if (w && isdigit(disk_device[w-1]))
		w++;
	if (w < 7)
		w = 7;

	printf("%-*s Boot StartCHS    EndCHS        StartLBA     EndLBA    Sectors  Size Id Type\n",
		   w-1, "Device");

	for (i = 0; i < g_partitions; i++) {
		const struct partition *p;
		const struct pte *pe = &ptes[i];
		char boot4[4];
		char numstr6[6];
		sector_t start_sect;
		sector_t end_sect;
		sector_t nr_sects;

		p = pe->part_table;
		if (!p || is_cleared_partition(p))
			continue;

		sprintf(boot4, "%02x", p->boot_ind);
		if ((p->boot_ind & 0x7f) == 0) {
			/* 0x80 shown as '*', 0x00 is ' ' */
			boot4[0] = p->boot_ind ? '*' : ' ';
			boot4[1] = ' ';
		}

		start_sect = get_partition_start_from_dev_start(pe);
		end_sect = start_sect;
		nr_sects = get_nr_sects(p);
		if (nr_sects != 0)
			end_sect += nr_sects - 1;

		smart_ulltoa5((ullong)nr_sects * sector_size,
			numstr6, " KMGTPEZY")[0] = '\0';

#define SFMT SECT_FMT
		//      Boot StartCHS    EndCHS        StartLBA     EndLBA    Sectors  Size Id Type
		printf("%s%s %-11s"/**/" %-11s"/**/" %10"SFMT"u %10"SFMT"u %10"SFMT"u %s %2x %s\n",
			partname(disk_device, i+1, w+2),
			boot4,
			chs_string11(p->cyl, p->head, p->sector),
			chs_string11(p->end_cyl, p->end_head, p->end_sector),
			start_sect,
			end_sect,
			nr_sects,
			numstr6,
			p->sys_ind,
			partition_type(p->sys_ind)
		);
#undef SFMT
		check_consistency(p, i);
	}

	/* Is partition table in disk order? It need not be, but... */
	/* partition table entries are not checked for correct order
	 * if this is a sgi, sun or aix labeled disk... */
	if (LABEL_IS_DOS && wrong_p_order(NULL)) {
		/* FIXME */
		puts("\nPartition table entries are not in disk order");
	}
#endif
}
#endif

extern int get_bus_type(char *name, char *type);

/* 根据设备节点获取cid */
#define LINUX_SD_CID_PATH		"/sys/block/%s/device/cid"
/* 
	char *name 		入参，节点名称   /dev/mmcblk0
	char *cid		出参，cid
	int cid_len		入参，cid最大长度
 */
static int get_device_cid(char *name, char *cid, int cid_len)
{
	FILE *fp = NULL;
	char cid_path[64] = {0};

	/* 检测文件是否存在 */
	snprintf(cid_path, sizeof(cid_path), LINUX_SD_CID_PATH, name + 5);
	if(access(cid_path,F_OK)) {
		printf("not find %s\n", cid_path);
		return -1;
	}

	fp = fopen(cid_path, "r");
	if(fp == NULL) {
		// printf("open %s err\n", cid_path);
		return -1;  
	}

	memset(cid,0,sizeof(cid_len));
	if(fgets(cid,cid_len,fp) == NULL) {
		fclose(fp);
		return -1;
	}

	fclose(fp);

	return 0;
}

static int get_device_info(const char *device, TKL_STORAGE_DEVICE_INFO_T *info)
{
	int gb;
    struct stat st = {0};

	disk_device = device;
	errno = 0;

	memset(info, 0, sizeof(*info));

	gb = get_boot();
	if (gb < 0) {   /* I/O error */
		printf("can't open '%s'", device);
		return -1;
	} else if (gb == 0 && LABEL_IS_GPT) {	/* GPT signature */
		//gpt_list_table(0);
		strncpy(info->dev_name, disk_device, MAX_PATH_LEN);
		get_bus_type(info->dev_name, info->bus_type);
		info->capacity = (total_number_of_sectors * sector_size)/(1<<10); //KB
		info->sect_size = sector_size;
		strncpy(info->part_table_type, "gpt", 16);
		strncpy(info->dev_id, (const char *)gpt_hdr->disk_guid, 16);
	} else if (gb == 1) {  /* DOS signature */
		//list_disk_geometry();
		strncpy(info->dev_name, disk_device, MAX_PATH_LEN);
		get_bus_type(info->dev_name, info->bus_type);
		ullong bytes = ((ullong)total_number_of_sectors << 9);
		ullong xbytes = bytes / (1024); // KB
		info->capacity = xbytes;
		info->sect_size = sector_size;
		strncpy(info->part_table_type, "mbr", 16);
		if (0 == strcmp(info->bus_type, "sdcard")) {
			get_device_cid(info->dev_name, info->dev_id, sizeof(info->dev_id));
		}
	} else {
		printf("private patition table\n");
		strncpy(info->dev_name, disk_device, MAX_PATH_LEN);
		get_bus_type(info->dev_name, info->bus_type);
		info->capacity = (total_number_of_sectors * sector_size)/(1<<10); //KB
		info->sect_size = sector_size;
		strncpy(info->part_table_type, "private", 16);
		if (0 == strcmp(info->bus_type, "sdcard")) {
			get_device_cid(info->dev_name, info->dev_id, sizeof(info->dev_id));
		}
	}

    /* get device major/minor id*/
    if (stat(disk_device, &st) < 0)
    {
		printf("can't stat '%s'", disk_device);
        info->major = 0;
        info->minor = 0;
    }
    else
    {
        info->major = major(st.st_rdev);
        info->minor = minor(st.st_rdev);
    }

	close_dev_fd();

	return 0;
}

static int get_custom_partition_info(const char *device, const char *partdev, const char *info)
{
	FILE *procpt;
    char line[16];
    int value=0;
    char *dev, *pdev;
    char filename[64]={0};

    dev = strrchr(device, '/');
    pdev = strrchr(partdev, '/');
    sprintf(filename, "/sys/block/%s/%s/%s", dev, pdev, info);

	procpt = fopen(filename, "r");
	if (procpt == NULL)
		return -1;

    fgets(line, sizeof(line), procpt);
    if (strlen(line) <= 0)
    {
        fclose(procpt);
		return -1;
    }
	if (sscanf(line, "%d", &value) != 1)
    {
        fclose(procpt);
		return -1;
    }
	fclose(procpt);

    return value; // unit sector
}

#if 0
static int get_dev_partition_info(const char *device, TKL_STORAGE_PART_INFO_T *list, int num)
{
	int gb;
	int i = 0;
	int index = 0;
	char partdev[32]={0};

	disk_device = device;
	errno = 0;

	memset(list, 0, sizeof(TKL_STORAGE_PART_INFO_T)*num);

	gb = get_boot();
	if (gb < 0) {   /* I/O error */
		printf("can't open '%s'", device);
		return -1;
	} else if (gb == 0 && LABEL_IS_GPT) {	/* GPT signature */
		//gpt_list_table(0);
		for (i = 0; i < n_parts; i++) {
			if(index > num)
				break;

			gpt_partition *p = gpt_part(i);
			if (p->lba_start) {
				if(!strncmp(disk_device, "/dev/mmcblk", 11)) {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%sp%d", disk_device, index+1);
				} else {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%s%d", disk_device, index+1);
				}
                
				list[index].start = (SWAP_LE64(p->lba_start)*sector_size)/(1<<10);
				list[index].end = (SWAP_LE64(p->lba_end)*sector_size)/(1<<10);
				list[index].size = ((SWAP_LE64(p->lba_end) - SWAP_LE64(p->lba_start))*sector_size)/(1<<10);
				strncpy(list[index].part_label, (char *)p->name36, MAX_FSTP_LEN);
				index++;
			}
		}
	} else if (gb == 1) {  /* DOS signature */
		//list_disk_geometry();
		//list_table(0);
		for (i = 0; i < g_partitions; i++) {
			const struct pte *pe = &ptes[i];
			UINT64_T tmpval=0;

			if(index > num)
				break;

			if((pe->ext_pointer) && (IS_EXTENDED(pe->ext_pointer->sys_ind)))
			{
				if(!strncmp(disk_device, "/dev/mmcblk", 11)) {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%sp%d", disk_device, index+1);
				} else {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%s%d", disk_device, index+1);
				}

				tmpval = get_start_sect(pe->part_table)+ pe->offset_from_dev_start;
				list[index].start = tmpval*sector_size/(1<<10);
				tmpval = get_nr_sects(pe->part_table);
				list[index].size = tmpval*sector_size/(1<<10);
				list[index].end = list[index].start + list[index].size;

                // MBR partition not have label
				//strncpy(list[index].part_label, p->name36, MAX_FSTP_LEN);				

				index++;
			}
			else if (i<4 && pe->part_table->sys_ind) 
			{
				if(!strncmp(disk_device, "/dev/mmcblk", 11)) {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%sp%d", disk_device, index+1);
				} else {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%s%d", disk_device, index+1);
				}

				tmpval = get_start_sect(pe->part_table);
				list[index].start = tmpval*sector_size/(1<<10);
				tmpval = get_nr_sects(pe->part_table);
				list[index].size = tmpval*sector_size/(1<<10);
				list[index].end = list[index].start + list[index].size;

                // MBR partition not have label
				//strncpy(list[index].part_label, p->name36, MAX_FSTP_LEN);	

				index++;
			}            
			else if(i>=4 && pe->part_table->sys_ind)
			{
				if(!strncmp(disk_device, "/dev/mmcblk", 11)) {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%sp%d", disk_device, index+1);
				} else {
					snprintf(list[index].dev_name, MAX_PATH_LEN, "%s%d", disk_device, index+1);
				}

				tmpval = get_start_sect(pe->part_table)+ pe->offset_from_dev_start;
				list[index].start = tmpval*sector_size/(1<<10);
				tmpval = get_nr_sects(pe->part_table);
				list[index].size = tmpval*sector_size/(1<<10);
				list[index].end = list[index].start + list[index].size;

                // MBR partition not have label
				//strncpy(list[index].part_label, p->name36, MAX_FSTP_LEN);	
				
				index++;
			}
		}
	} else if(gb == 2) {
	    UINT32_T tvalue=0;
		printf("private patition table\n");
        // max 16 partitions
        for(i=0; i<num; i++)
        {
            sprintf(partdev, "%s%d", device, (i+1));//no /dev/sda0
            if((access(partdev, F_OK)) != 0)
                continue;

            sprintf(list[index].dev_name, "%s", partdev);
            tvalue = get_custom_partition_info(device, partdev, "start");
            list[index].start = tvalue*sector_size/(1<<10);
            tvalue = get_custom_partition_info(device, partdev, "size");
            list[index].size = tvalue*sector_size/(1<<10);
            list[index].end = list[index].start + list[index].size;
            index++;
        }
	}
	close_dev_fd();

	return index;
}
#endif

static int is_whole_disk(const char *disk);

// partition table from /sys/block/xxx/ instead of gpt/mbr
static int get_dev_partition_info_v2(const char *device, TKL_STORAGE_PART_INFO_T *list, int num)
{
    int fd,ret=0;
	int i = 0;
	int index = 0;
	UINT64_T tvalue=0;
    UINT32_T sec_size;
	FILE *procpt;
	char line[100], ptname[100], devname[120];
	int ma, mi, sz;

    fd = open(device, O_RDONLY);
    if(fd<0)
    {
        printf("%s %d open %s fail!\n", __func__, __LINE__, device);
        return -1;
    }

    ret = ioctl(fd, BLKSSZGET, &sec_size);
	if (ret)
    {
        printf("%s %d ioctl %s fail, ret=%d\n", __func__, __LINE__, device, ret);
        return -1;
    }

    close(fd);

	if (sec_size != DEFAULT_SECTOR_SIZE)
		printf("Note: sector size is %u "
			"(not " DEFAULT_SECTOR_SIZE_STR ")\n",
			sec_size);

	procpt = fopen("/proc/partitions", "r");
	if (procpt == NULL)
		return -1;

	while (fgets(line, sizeof(line), procpt)) {
		if (strlen(line) <= 0)
			break;

		if (sscanf(line, " %u %u %u %[^\n ]", &ma, &mi, &sz, ptname) != 4)
			continue;

		sprintf(devname, "/dev/%s", ptname);
		if (0 != strncmp(device, devname, strlen(device)))
			continue;

		if (is_whole_disk(devname))
			continue;

        if((access(devname, F_OK)) != 0)
            continue;

        sprintf(list[index].dev_name, "%s", devname);
        list[index].part_id = get_custom_partition_info(device, devname, "partition");
        tvalue = get_custom_partition_info(device, devname, "start");
        list[index].start = tvalue*sec_size/(1<<10);
        tvalue = get_custom_partition_info(device, devname, "size");
        list[index].size = tvalue*sec_size/(1<<10);
        list[index].end = list[index].start + list[index].size;
        index++;
		if (index >= num) {
			break;
		}
	}

	fclose(procpt);

	return index;
}


/* Is it a whole disk? The digit check is still useful
   for Xen devices for example. */
static int is_whole_disk(const char *disk)
{
	unsigned len;
	int fd = open(disk, O_RDONLY);

	if (fd != -1) {
		struct hd_geometry geometry;
		int err = ioctl(fd, HDIO_GETGEO, &geometry);

		close(fd);
		if (!err)
			return (geometry.start == 0);
	}

	/* Treat "nameN" as a partition name, not whole disk */
	/* note: mmcblk0 should work from the geometry check above */
	len = strlen(disk);
	if (len != 0 && isdigit(disk[len - 1]))
		return 0;
    
	return 1;
}

int list_devs_in_one_device(CHAR_T *dev_name, TKL_STORAGE_DEVICE_INFO_T *list)
{
	int ret=0;
	INIT_G();

	if (is_whole_disk(dev_name)) {
		ret = get_device_info(dev_name, list);
		if (ret < 0)
		{
            printf("%s %d \n", __func__, __LINE__);
		}
	}

	free(ptr_to_globals);
	return ret;
}

/* 输出所有的设备信息 */
int list_devs_in_proc_partititons(TKL_STORAGE_DEVICE_INFO_T *list, int num)//MAX_DEVICE_NUM
{
	FILE *procpt;
	char line[100], ptname[100], devname[120];
	int ma, mi, sz;
	int index = 0;
	int ret;

	INIT_G();

	procpt = fopen("/proc/partitions", "r");
	if (procpt == NULL)
		return -1;

	while (fgets(line, sizeof(line), procpt)) {

		if (strlen(line) <= 0)
			break;

		if (sscanf(line, " %u %u %u %[^\n ]", &ma, &mi, &sz, ptname) != 4)
			continue;

		sprintf(devname, "/dev/%s", ptname);
		if (is_whole_disk(devname)) {
			ret = get_device_info(devname, &list[index]);
			if (ret < 0)
			{
                printf("%s %d \n", __func__, __LINE__);
				continue;
			}
			index++;
		}
	}

	fclose(procpt);

	free(ptr_to_globals);

	return index;
}

/* 输出设备分区信息 */
int list_device_partitions(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, int num)
{
	INIT_G();

	if (is_whole_disk(dev_name))
#if 0
		return get_dev_partition_info(dev_name, list, num);
#else
		return get_dev_partition_info_v2(dev_name, list, num);
#endif

	free(ptr_to_globals);

	return -1;
}
