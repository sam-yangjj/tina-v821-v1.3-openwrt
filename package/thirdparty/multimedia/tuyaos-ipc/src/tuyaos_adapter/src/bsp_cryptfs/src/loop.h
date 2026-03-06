#ifndef __TUYA_LOOP_DEV_H__
#define __TUYA_LOOP_DEV_H__

char *loopdev_create(const char *target_dev);
void loopdev_destroy(char *loopdev);

#endif
