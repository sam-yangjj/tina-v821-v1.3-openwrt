#include <mntent.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <mntent.h>
#include <sys/types.h>    
#include <sys/stat.h>
#include <stdbool.h> 
#include <string.h>

#include <tkl_storage.h>


#define ENABLE_FEATURE_DF_FANCY	0
#define ENABLE_FEATURE_HUMAN_READABLE 0
#define ENABLE_FEATURE_SKIP_ROOTFS 1
#define bb_path_mtab_file "/proc/mounts"


/*
 * Given a block device, find the mount table entry if that block device
 * is mounted.
 *
 * Given any other file (or directory), find the mount table entry for its
 * filesystem.
 */
struct mntent* find_mount_point(const char *name, int subdir_too)
{
	struct stat s;
	FILE *mtab_fp;
	struct mntent *mountEntry;
	dev_t devno_of_name;
	bool block_dev;

	if (stat(name, &s) != 0)
		return NULL;

	devno_of_name = s.st_dev;
	block_dev = 0;
	/* Why S_ISCHR? - UBI volumes use char devices, not block */
	if (S_ISBLK(s.st_mode) || S_ISCHR(s.st_mode)) {
		devno_of_name = s.st_rdev;
		block_dev = 1;
	}

	mtab_fp = setmntent(bb_path_mtab_file, "r");
	if (!mtab_fp)
		return NULL;

	while ((mountEntry = getmntent(mtab_fp)) != NULL) {
		/* rootfs mount in Linux 2.6 exists always,
		 * and it makes sense to always ignore it.
		 * Otherwise people can't reference their "real" root! */
		if (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(mountEntry->mnt_fsname, "rootfs") == 0)
			continue;

		if (strcmp(name, mountEntry->mnt_dir) == 0
		 || strcmp(name, mountEntry->mnt_fsname) == 0
		) { /* String match. */
			break;
		}

		if (!(subdir_too || block_dev))
			continue;

		/* Is device's dev_t == name's dev_t? */
		if (mountEntry->mnt_fsname[0] == '/'
		/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		 * avoid stat'ing "sysfs", "proc", "none" and such,
		 * useless at best, can stat unrelated files at worst.
		 */
		 && stat(mountEntry->mnt_fsname, &s) == 0
		 && s.st_rdev == devno_of_name
		) {
			break;
		}
		/* Match the directory's mount point. */
		if (stat(mountEntry->mnt_dir, &s) == 0
		 && s.st_dev == devno_of_name
		) {
			break;
		}
	}
	endmntent(mtab_fp);

	return mountEntry;
}

static unsigned long kscale(unsigned long b, unsigned long bs)
{
	return (b * (unsigned long long) bs + 1024/2) / 1024;
}

int df_main(CHAR_T *mount_dir, TKL_STORAGE_DF_T *df)
{
	unsigned long df_disp_hr = 1024;
	int status = 0;
	unsigned opt;
	FILE *mount_table;
	struct mntent *mount_entry;
	struct statvfs s;

	enum {
		OPT_KILO  = (1 << 0),
		OPT_POSIX = (1 << 1),
		OPT_FSTYPE  = (1 << 2),
		OPT_ALL   = (1 << 3) * ENABLE_FEATURE_DF_FANCY,
		OPT_INODE = (1 << 4) * ENABLE_FEATURE_DF_FANCY,
		OPT_BSIZE = (1 << 5) * ENABLE_FEATURE_DF_FANCY,
		OPT_HUMAN = (1 << (3 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
		OPT_MEGA  = (1 << (4 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
	};

	opt = OPT_FSTYPE;

    /*
	const char *disp_units_hdr = NULL;
	disp_units_hdr = "1K-blocks";
	printf("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

    */
	mount_table = NULL;

#ifdef PRINT_ALL
	mount_table = setmntent(bb_path_mtab_file, "r");
	if (!mount_table)
		printf("error %s\n", bb_path_mtab_file);
#endif

	while (1) {
		const char *device;
		const char *mount_point;
		const char *fs_type;

		if (mount_table) {
			mount_entry = getmntent(mount_table);
			if (!mount_entry) {
				endmntent(mount_table);
				break;
			}
		} else {
			mount_point = mount_dir;
			mount_entry = find_mount_point(mount_point, 1);
			if (!mount_entry) {
				printf("%s: can't find mount point", mount_point);
 set_error:
				status = -1;
				break;//continue;
			}
		}

		device = mount_entry->mnt_fsname;
		mount_point = mount_entry->mnt_dir;
		fs_type = mount_entry->mnt_type;

		if (statvfs(mount_point, &s) != 0) {
			printf("error %s\n", mount_point);
			goto set_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		if (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		if ((s.f_blocks > 0) || !mount_table || (opt & OPT_ALL)) {
			// unsigned long long blocks_used;
			// unsigned long long blocks_total;
			// unsigned blocks_percent_used;

			if (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				if (df_disp_hr)
					df_disp_hr = 1;
			}
			// blocks_used = s.f_blocks - s.f_bfree;
			// blocks_total = blocks_used + s.f_bavail;
			// blocks_percent_used = blocks_total; /* 0% if blocks_total == 0, else... */
			// if (blocks_total != 0) {
			// 	/* Downscale sizes for narrower division */
			// 	unsigned u;
			// 	while (blocks_total >= INT_MAX / 101) {
			// 		blocks_total >>= 1;
			// 		blocks_used >>= 1;
			// 	}
			// 	u = (unsigned)blocks_used * 100u + (unsigned)blocks_total / 2;
			// 	blocks_percent_used = u / (unsigned)blocks_total;
			// }

            /*
			if (printf("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				printf("\n%-20s", "");
			if (opt & OPT_FSTYPE) {
				if (printf(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					printf("\n%-30s", "");
			}

			printf(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				blocks_percent_used, mount_point);
            */

			//strcpy(df->fs_dev, device);
 			strcpy(df->dev_name, device);
           
			strcpy(df->fs_type, fs_type);
			df->size = kscale(s.f_blocks, s.f_frsize);
			df->used = kscale(s.f_blocks - s.f_bfree, s.f_frsize);
			df->avail = kscale(s.f_bavail, s.f_frsize);
			//df->use_rate = blocks_percent_used;
			strcpy(df->mounted_on, mount_point);
		}

		break;
	}

	return status;
}
