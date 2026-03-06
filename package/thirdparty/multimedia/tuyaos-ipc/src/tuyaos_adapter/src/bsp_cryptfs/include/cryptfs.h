#ifndef __TUYA_CRYPTFS_H__
#define __TUYA_CRYPTFS_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern int cryptfs_create(const char *blkdev, const char *key);
extern int cryptfs_enable(const char *blkdev, const char *name, const char *key);
extern int cryptfs_disable(const char *name);

#ifdef __cplusplus
}
#endif
#endif
