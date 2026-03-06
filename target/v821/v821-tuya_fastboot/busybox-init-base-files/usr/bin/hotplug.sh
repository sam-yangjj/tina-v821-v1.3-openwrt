#!/bin/sh

# remove
if [ "${ACTION}" == "remove" ]; then
    MOUNTPOINT="$(grep -w "^/dev/${MDEV}" /proc/mounts | awk '{print $2}')"
    [ -n "${MOUNTPOINT}" ] \
        && /bin/umount -l ${MOUNTPOINT} && rm -rf ${MOUNTPOINT} 2>/dev/null 1>/dev/null
    exit 0
fi

# add
if [ "${ACTION}" == "add" ]; then
    case ${MDEV} in
        mmcblk[0-9])
            [ -d "/sys/block/${MDEV}/${MDEV}p1" ] && exit 0
            MOUNTPOINT=/mnt/sdcard
            # in case of grab lock while mount sub-partition
            sleep 0.1
            ;;
        mmcblk[0-9]p[0-9])
            MOUNTPOINT=/mnt/sdcard
            ;;
        sd[a-z])
            [ -d "/sys/block/${MDEV}/${MDEV}1" ] && exit 0
            MOUNTPOINT=/mnt/exUDISK
            # in case of grab lock while mount sub-partition
            sleep 0.1
            ;;
        sd[a-z][0-9])
            MOUNTPOINT=/mnt/exUDISK
            ;;
        *)
            exit 0
            ;;
    esac
    for fstype in vfat exfat ntfs-3g ext4
    do
        if [ ${MOUNTPOINT} != "NULL" ]; then
            [ "${fstype}" = "ext4" -a -x "/usr/sbin/e2fsck" ] \
                && e2fsck -p /dev/${MDEV} >/dev/null
            mkdir -p ${MOUNTPOINT}
            /bin/mount -t ${fstype} /dev/${MDEV} ${MOUNTPOINT}
            exit 0
        fi
    done
fi
exit 0
