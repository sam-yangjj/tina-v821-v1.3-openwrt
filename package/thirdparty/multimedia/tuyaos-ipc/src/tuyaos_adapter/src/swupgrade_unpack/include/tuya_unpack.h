/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 *
 */

#ifndef _TUYA_UNPACK_H
#define _TUYA_UNPACK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>

#ifdef BSP_PACK_PART_CNT
#define MAX_OTA_ZONE            BSP_PACK_PART_CNT
#define OTA_HEAD_SIZE           512*BSP_PACK_PART_CNT
#else
#define MAX_OTA_ZONE            10
#define OTA_HEAD_SIZE           4096
#endif

#define MAX_OTA_FILE		2
#define HASH_LENGTH     	32
#define TUYA_SIGN_SIZE		256

typedef struct ota_sign {
        int len;
        unsigned char data[TUYA_SIGN_SIZE];
}__attribute__ ((packed)) ota_sign;

typedef struct ota_zone {
	char name[32];
	char version[32];
	int offset;
	int image_size;
	char device[2][32];
	int device_offset;
	char filesystem[32];
	int copy_in_boot;
	char sha256[HASH_LENGTH];
}__attribute__ ((packed)) ota_zone;

typedef struct ota_file {
	char name[32];
	char version[32];
	int image_size;
	char storage_path[128];
	char installer[128];
	char sha256[HASH_LENGTH];
}__attribute__ ((packed)) ota_file;

typedef struct ota_head {
	char magic[32]; /* "TUYA_OTA" */
	char flash_type[32];
	char ota_mode[32]; /* single-copy double-copy recovery */
	int zone_num;
	ota_zone zone[MAX_OTA_ZONE];
	int file_num;
	ota_file file[MAX_OTA_FILE];
}__attribute__ ((packed)) ota_head;

typedef int (*ota_progress)(int percentage, void *user_data);

typedef struct ota_image {
	char *img_buf;
	int zone_idx;
	int file_idx;
	ota_head head;
	/* for progress */
	int total_len;
	int ld_percent;
	int wr_percent;
	ota_progress progress_cb;
	void *user_data;
	int abort;
	pthread_t progress_daemon;
	pthread_mutex_t prog_mutex;
} ota_image;

enum {
	GET_HEAD,
	GET_IMG,
	GET_FILE
};

typedef struct ota_stream {
	int non_block;
	char *buf;
	int len_owe;
	int len_earn;
	ota_image img;
	int status;
	int abort;
	/* for non-block upgrade */
	pthread_t async_thread_id;
	pthread_cond_t ota_cond;
	pthread_mutex_t ota_mutex;
} ota_stream;

/* used to post progress info when upgrading
 * all of the params must be filled
 */
typedef struct progress_post {
	char url[256];
	char api_name[64];
	char api_ver[64];
	char aes_key[64];
	char dev_id[64];
} progress_post;

/* internal struct */
typedef struct ota_data {
	char *buf;
	int total_len;
	int cur_len;
	void *priv_data;
} ota_data;

/* internal function */
void update_writein_progress(ota_image *pimg, int write_len);

/**
 * @brief prepare for upgrade
 * @param block block or not
 * @param cb callback function to be call when upgrading
 * @param user_data privite data for cb
 * @return success return handle, failed return NULL
 */
ota_stream *device_upgrade_open(int block, ota_progress cb, void *user_data);


void device_upgrade_set_stmcnt(unsigned int length);
int produce_stream(int fd, char *buf, int len);
int consume_stream(ota_stream *obj, char *data, int data_len);
int get_device_geometry(char *dev);


/* reserved function */
int device_upgrade_feed_internal(ota_stream *obj, char *data, int data_len);

/**
 * @brief feed ota stream data
 * @param obj handle returned by device_upgrade_open function
 * @param data ota data buffer pointer
 * @param data_len ota data buffer len
 * @return success return 0, failed return -errno
 */
int device_upgrade_feed(ota_stream *obj, char *data, int data_len);

/**
 * @brief stop upgrade or release upgrade resource when done
 * @param obj handle returned by device_upgrade_open function
 * @return success return 0, failed return -errno
 */
int device_upgrade_close(struct ota_stream *obj);

/**
 * @brief set frequency of tx carrier
 * @param id board ID to open IR device
 * @param frequency frequency of the carrier to be set, in Hz
 * @return success return 0, failed return -errno
 */
int install_from_file(char *fw_name);

/**
 * @brief upgrade by http
 * @param url url where ota firmware download from
 * @param post infos for progress post
 * @return success return 0, failed return -errno
 */
int install_from_url(char *url, progress_post *post);

/**
 * @brief print ota firmware head information
 * @param fw_name ota firmware path/name
 * @return success return 0, failed return -errno
 */
int tuya_upgrade_dump(char *fw_name);

/**
 * @brief extract image from ota firmware
 * @param fw_name ota firmware path/name
 * @param dst_path destination of the extracted image
 * @return success return 0, failed return -errno
 */
int tuya_upgrade_extract(char *fw_name, char *dst_path);

/**
 * @brief upgrade version compare
 * @param ota firmware version
 * @return success return 0, failed return -errno
 */
int tuya_upgrade_version(char ver[32]);

/**
 * @brief set ota upgrade version compare mode
 * @param mode:0 version don't compare,1:version compare
 * @return success return 0, failed return -errno
 */
int set_version_compare_mode(int mode);

/**
 * @brief get force upgrade mode
 * @return 1,ota upgrate version compare; 0,ota upgrate don't version compare
 */
int get_version_compare_mode(void);


/**
 * @brief get force upgrade mode
 * @return success return mode, failed return -errno
 */
int get_force_upgrade_mode();

/**
 * @brief set no verification signature mode
 * @param mode 0:not verify signature, 1:verify signature
 * @return success return 0, failed return -errno
 */
int set_verify_sign_mode(int mode);

/**
 * @brief get verification signature mode
 * @return not verify signature return 0, verify signature return 1
 */
int get_verify_sign_mode();

#ifdef __cplusplus
}
#endif

#endif
