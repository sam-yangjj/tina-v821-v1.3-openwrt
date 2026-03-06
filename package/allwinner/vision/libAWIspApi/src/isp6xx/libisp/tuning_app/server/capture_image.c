#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../isp.h"
#include "../../include/device/isp_dev.h"
#include "../../include/device/video.h"
#include "../log_handle.h"
#include "../thread_pool.h"
#include "../socket_protocol.h"
#include "capture_image.h"
#include "server_core.h"
#include "raw_flow_opt.h"
#include "isp_handle.h"

#define CAPTURE_CHANNEL_MAX     HW_VIDEO_DEVICE_NUM
#define CAPTURE_RAW_FLOW_QUEUE_SIZE   201

struct hw_isp_media_dev *g_media_dev = NULL;
int                      g_sensor_set_flag;  // bit0-CAPTURE_CHANNEL_MAX-1 set flag vich 0~CAPTURE_CHANNEL_MAX-1

typedef struct _frame_buffer_info_s
{
	unsigned int	u32PoolId;
	unsigned int	u32Width;
	unsigned int	u32Height;
	unsigned int	u32PixelFormat;
	unsigned int	u32field;

	unsigned int	u32PhyAddr[3];
	void			*pVirAddr[3];
	unsigned int	u32Stride[3];

	struct timeval stTimeStamp;
} frame_buf_info;

typedef struct _vi_priv_attr_s
{
	int vich;
	int timeout;
	struct video_fmt vi_fmt;
	frame_buf_info vi_frame_info;
} vi_priv_attr;

typedef enum _cap_flag_e {
	CAP_STATUS_ON            = 0x0001,  // 0:cap video off, 1:cap video on
	CAP_THREAD_RUNNING       = 0x0002,  // 0:thread stopped, 1:thread running
	CAP_THREAD_STOP          = 0x0004,  // 0: not stop thread, 1:to stop thread
	CAP_RAW_FLOW_RUNNING     = 0x0008,  // 0: raw flow stopped, 1:raw flow running
	CAP_RAW_FLOW_START       = 0x0010,  // 0: stop raw flow, 1:to start raw flow
} cap_flag;

typedef struct _capture_params_s {
	vi_priv_attr             priv_cap;
	unsigned int             status;
	int                      frame_count;
	pthread_mutex_t          locker;
} capture_params;

typedef enum _cap_init_status_e {
	CAPTURE_INIT_NOT         = 0,
	CAPTURE_INIT_YES         = 1,
} cap_init_status;

cap_init_status              g_cap_init_status = CAPTURE_INIT_NOT;
capture_params               g_cap_handle[CAPTURE_CHANNEL_MAX];
pthread_mutex_t              g_cap_locker;

static void reset_cap_params(capture_params *cap_pa)
{
	if (cap_pa) {
		memset(&cap_pa->priv_cap, 0, sizeof(vi_priv_attr));
		cap_pa->status = 0;
		cap_pa->frame_count = 0;
	}
}

#define VALID_VIDEO_SEL(id)    \
	((id) >= 0 && (id) < CAPTURE_CHANNEL_MAX)

static int init_video(struct hw_isp_media_dev *media_dev, int vich)
{
	return isp_video_open(media_dev, vich);
}

static int exit_video(struct hw_isp_media_dev *media_dev, int vich)
{
	isp_video_close(media_dev, vich);
	return 0;
}

static int set_video_fmt(struct hw_isp_media_dev *media_dev, int vich, struct video_fmt *vi_fmt)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	return video_set_fmt(video, vi_fmt);
}

static int get_video_fmt(struct hw_isp_media_dev *media_dev, int vich, struct video_fmt *vi_fmt)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	memset(vi_fmt, 0, sizeof(struct video_fmt));
	video_get_fmt(video, vi_fmt);

	return 0;
}

static int enable_video(struct hw_isp_media_dev *media_dev, int vich)
{
	struct isp_video_device *video = NULL;
	struct buffers_pool *pool = NULL;
	struct video_fmt vfmt;
	int i;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	pool = buffers_pool_new(video);
	if (!pool) {
		return -1;
	}

	if (video_req_buffers(video, pool) < 0) {
		return -1;
	}

	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	for (i = 0; i < vfmt.nbufs; i++) {
		video_queue_buffer(video, i);
	}

	return video_stream_on(video);
}

static int disable_video(struct hw_isp_media_dev *media_dev, int vich)
{
	struct isp_video_device *video = NULL;
	struct video_fmt vfmt;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	if (vfmt.ptn_en) {
		fclose(vfmt.ptn_file);
		if (vfmt.ptn_cfg.ptn_addr) {
			free(vfmt.ptn_cfg.ptn_addr);
			vfmt.ptn_cfg.ptn_addr = NULL;
		}
	}

	if (video_stream_off(video) < 0) {
		return -1;
	}
	if (video_free_buffers(video) < 0) {
		return -1;
	}
	buffers_pool_delete(video);

	return 0;
}

int update_ptn_count(int vich, unsigned char ptn_type, unsigned int start_frame, unsigned int end_frame)
{
	struct isp_video_device *video = NULL;
	int ret = 0;

	if (!VALID_VIDEO_SEL(vich) || NULL == g_media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = g_media_dev->video_dev[vich];
	}

	video->ptn_type = ptn_type;
	video->start_frame = start_frame;
	video->end_frame = end_frame;

	if (video->start_frame > video->end_frame) {
		LOG("video%d:cannot set start_frame %d more than end_frame %d\n", vich, video->start_frame, video->end_frame);
		ret = -1;
	}
	if (video->end_frame > video->ptn_count_total) {
		LOG("video%d:cannot set end_frame %d more than ptn_tatal_count %d\n", vich, video->end_frame, video->ptn_count_total);
		ret = -1;
	}

	if (!ret)
		LOG("video%d:ptn_type:%d, start_frame:%d, end_frame:%d\n", vich, video->ptn_type, video->start_frame, video->end_frame);

	return ret;
}

static void read_next_bin_file(struct video_fmt *vi_fmt)
{
	int length;
	int start_conut, end_count;

	if (vi_fmt->ptn_file == NULL) {
		return;
	}

	if (vi_fmt->end_frame && vi_fmt->start_frame <= vi_fmt->end_frame) {
		start_conut = vi_fmt->start_frame;
		end_count = min(vi_fmt->end_frame, vi_fmt->ptn_count_total);
	} else {
		start_conut = 0;
		end_count = vi_fmt->ptn_count_total;
	}

	if (vi_fmt->ptn_type) {
		if (start_conut == end_count) {
			fseek(vi_fmt->ptn_file, -1 * vi_fmt->ptn_cfg.ptn_size, SEEK_CUR);
			*vi_fmt->ptn_count = *vi_fmt->ptn_count - 1;
		} else {
			if (*vi_fmt->ptn_count >= end_count) {
				*vi_fmt->ptn_read_dir = 1;
			}
			if (*vi_fmt->ptn_count <= start_conut) {
				*vi_fmt->ptn_read_dir = 0;
			}

			if (*vi_fmt->ptn_read_dir == 1) {
				fseek(vi_fmt->ptn_file, -2 * vi_fmt->ptn_cfg.ptn_size, SEEK_CUR);
				*vi_fmt->ptn_count = *vi_fmt->ptn_count - 2;
			}
		}
	} else {
		if ((*vi_fmt->ptn_count >= end_count) || (*vi_fmt->ptn_count <= start_conut)) {
			rewind(vi_fmt->ptn_file);
			fseek(vi_fmt->ptn_file, start_conut * vi_fmt->ptn_cfg.ptn_size, SEEK_CUR);
			*vi_fmt->ptn_count = start_conut;
		}
	}

	length = fread(vi_fmt->ptn_cfg.ptn_addr, 1, vi_fmt->ptn_cfg.ptn_size, vi_fmt->ptn_file);
	if (length != 0) {
		if (*vi_fmt->ptn_count % 10 == 0)
			LOG("read ptn buf%d size is %d\n", *vi_fmt->ptn_count, length);
		(*vi_fmt->ptn_count)++;
	} else
		LOG("read ptn buf%d size is %d\n", *vi_fmt->ptn_count, length);

}

static int get_video_frame(struct hw_isp_media_dev *media_dev, int vich, frame_buf_info *frame_info, int timeout)
{
	struct isp_video_device *video = NULL;
	struct video_buffer buffer;
	struct video_fmt vfmt;
	int i;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);

	if (video_wait_buffer(video, timeout) < 0) {
		if (vfmt.ptn_en) {
			read_next_bin_file(&vfmt);
			video_set_next_ptn(video, &vfmt);
		}
		return -1;
	}

	if (vfmt.ptn_en) {
		read_next_bin_file(&vfmt);
		video_set_next_ptn(video, &vfmt);
	}

	if (video_dequeue_buffer(video, &buffer) < 0) {
		return -1;
	}

	for (i = 0; i < vfmt.nplanes; i++) {
		frame_info->pVirAddr[i] = buffer.planes[i].mem;
		frame_info->u32Stride[i] = buffer.planes[i].size;
		frame_info->u32PhyAddr[i] = buffer.planes[i].mem_phy;
	}
	frame_info->u32Width = vfmt.format.width;
	frame_info->u32Height = vfmt.format.height;
	frame_info->u32field = vfmt.format.field;
	frame_info->u32PixelFormat = vfmt.format.pixelformat;
	frame_info->stTimeStamp = buffer.timestamp;
	frame_info->u32PoolId = buffer.index;
//	if (ldci_video_sel == TUNINGAPP_VIDEO_IN) {
//		ldci_frame_buf_addr = frame_info->pVirAddr[0];
//	}

	return 0;
}

static int release_video_frame(struct hw_isp_media_dev *media_dev, int vich, frame_buf_info *frame_info)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	if (video_queue_buffer(video, frame_info->u32PoolId) < 0) {
		return -1;
	}

	return 0;
}

/*
 * frame thread
 */
static void *frame_loop_thread(void *params)
{
	int ret = -1, failed_times = 0, thread_status;
	capture_params *cap_pa = (capture_params *)params;
	//capture_format cap_fmt;
	//cap_fmt.buffer = (unsigned char *)malloc(1 << 24); // 16M
	//unsigned char *buffer = NULL;

	if (cap_pa && g_media_dev) {
		LOG("%s: channel %d starts\n", __FUNCTION__, cap_pa->priv_cap.vich);
		pthread_mutex_lock(&cap_pa->locker);
		cap_pa->status |= CAP_THREAD_RUNNING;
		pthread_mutex_unlock(&cap_pa->locker);
		while (1) {
			msleep(1);
			thread_status = CheckThreadsStatus();
			if (thread_status & TH_STATUS_PREVIEW_VENCODE) {
				msleep(100);
				continue;
			}
			ret = pthread_mutex_trylock(&cap_pa->locker);
			if (0 == ret) { // lock ok
				if (!(CAP_STATUS_ON & cap_pa->status)) {
					cap_pa->status &= ~CAP_THREAD_RUNNING;
					pthread_mutex_unlock(&cap_pa->locker);
					LOG("%s: channel %d is off\n", __FUNCTION__, cap_pa->priv_cap.vich);
					break;
				}
				if (CAP_THREAD_STOP & cap_pa->status) {
					disable_video(g_media_dev, cap_pa->priv_cap.vich);
					exit_video(g_media_dev, cap_pa->priv_cap.vich);
					cap_pa->status &= ~CAP_STATUS_ON;
					cap_pa->status &= ~CAP_THREAD_RUNNING;
					pthread_mutex_unlock(&cap_pa->locker);
					LOG("%s: recv stop flag(channel %d)\n", __FUNCTION__, cap_pa->priv_cap.vich);
					break;
				}
				ret = get_video_frame(g_media_dev, cap_pa->priv_cap.vich, &cap_pa->priv_cap.vi_frame_info, cap_pa->priv_cap.timeout);
				if (ret < 0) {
					LOG("%s: failed to get frame(channel %d, %d)\n", __FUNCTION__, cap_pa->priv_cap.vich, failed_times);
					failed_times++;
					if (failed_times >= 10) {
						disable_video(g_media_dev, cap_pa->priv_cap.vich);
						exit_video(g_media_dev, cap_pa->priv_cap.vich);
						cap_pa->status &= ~CAP_STATUS_ON;
						cap_pa->status &= ~CAP_THREAD_RUNNING;
						pthread_mutex_unlock(&cap_pa->locker);
						LOG("%s: failed too many times(channel %d)\n", __FUNCTION__, cap_pa->priv_cap.vich);
						break;
					}
				} else {
					failed_times = 0;
					#if 0
					if (CAP_RAW_FLOW_START & cap_pa->status) {
						cap_fmt.width = cap_pa->priv_cap.vi_fmt.format.width;
						cap_fmt.height = cap_pa->priv_cap.vi_fmt.format.height;
						cap_fmt.format = cap_pa->priv_cap.vi_fmt.format.pixelformat;
						cap_fmt.planes_count = cap_pa->priv_cap.vi_fmt.nplanes;
						cap_fmt.width_stride[0] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline;
						cap_fmt.width_stride[1] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline;
						cap_fmt.width_stride[2] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline;
						buffer = cap_fmt.buffer;
						cap_fmt.length = 0;
						for (ret = 0; ret < cap_pa->priv_cap.vi_fmt.nplanes; ret++) {
							memcpy(buffer, cap_pa->priv_cap.vi_frame_info.pVirAddr[ret],
								cap_pa->priv_cap.vi_frame_info.u32Stride[ret]);
							buffer += cap_pa->priv_cap.vi_frame_info.u32Stride[ret];
							cap_fmt.length += cap_pa->priv_cap.vi_frame_info.u32Stride[ret];
						}
						if (cap_fmt.length < cap_fmt.width * cap_fmt.height) {
							LOG("%s: raw flow - %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt.length, cap_fmt.width, cap_fmt.height);
						} else {
							queue_raw_flow(&cap_fmt);
							//msleep(8);
						}
						cap_pa->status |= CAP_RAW_FLOW_RUNNING;
					} else {
						cap_pa->status &= ~CAP_RAW_FLOW_RUNNING;
					}
					#endif
					cap_pa->status &= ~CAP_RAW_FLOW_RUNNING;
					release_video_frame(g_media_dev, cap_pa->priv_cap.vich, &cap_pa->priv_cap.vi_frame_info);
				}
				pthread_mutex_unlock(&cap_pa->locker);
			}
		}

		LOG("%s: channel %d quits\n", __FUNCTION__, cap_pa->priv_cap.vich);
	}
	//free(cap_fmt.buffer);
	//cap_fmt.buffer = NULL;

	return 0;
}

static void *read_bin_file(struct video_fmt *vi_fmt, char *path, int length)
{
	void *raw_data = NULL;
	long int total_length;

	if (length == 0)
		return NULL;

	vi_fmt->ptn_file = fopen(path, "rb");
	if (vi_fmt->ptn_file == NULL) {
		LOG("open %s fail\n", path);
		return NULL;
	}
	LOG("open %s ok\n", path);
	fseek(vi_fmt->ptn_file, 0, SEEK_END);
	total_length = ftell(vi_fmt->ptn_file);
	if (total_length <= 0) {
		LOG("maybe raw buf is too large, get buf length err\n")
		fclose(vi_fmt->ptn_file);
		return NULL;
	}
	rewind(vi_fmt->ptn_file);
	vi_fmt->ptn_count_total = total_length/length;
	LOG("%s total_length is %ld, raw conut is %d\n", path, total_length, vi_fmt->ptn_count_total);

	raw_data = (void *)malloc((length + 1) * sizeof(char));
	if (!raw_data) {
		LOG("malloc raw buf err, size is %d\n", length + 1)
		fclose(vi_fmt->ptn_file);
		return NULL;
	}

	length = fread(raw_data, 1, length, vi_fmt->ptn_file);
	if (length == 0) {
		LOG("read ptn size is %d\n", length);
		free(raw_data);
		raw_data = NULL;
		return NULL;
	}
	LOG("buf_addr %p, size = %d\n", raw_data, length);

	return raw_data;
}

static int set_video_ptn(struct hw_isp_media_dev *media_dev, int vich, struct video_fmt *vi_fmt)
{
	struct isp_video_device *video = NULL;
	int buf_len = 0;
	void *ptn_vaddr = NULL;
	FILE *fd = NULL;
	char raw_path[100], bayer[50];
	size_t fget_len = 0;
	int pixelformat;
#ifdef ANDROID_TUNING
	char *offline_tuning_path = "/data/OfflineTuning.txt";
#else
	char *offline_tuning_path = "/tmp/OfflineTuning.txt";
#endif
	char in_fmt[10] = {'\0'};

	if (!vi_fmt->ptn_en)
		return 0;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	fd = fopen(offline_tuning_path, "r");
	if (!fd){
		LOG("%s open failed\n", offline_tuning_path);
		vi_fmt->ptn_en = 0;
		return -1;
	}

	if (fgets(raw_path, 50, fd) != NULL) {
		fget_len = strlen(raw_path);
		if (fget_len && raw_path[fget_len - 1] == '\n')
			raw_path[fget_len - 1] = '\0';

		LOG("read raw_path is %s\n", raw_path);
	} else {
		vi_fmt->ptn_en = 0;
		fclose(fd);
		fd = NULL;
		return -1;
	}

	if (fgets(bayer, 50, fd) != NULL) {
		fget_len = strlen(bayer);
		if (fget_len && bayer[fget_len - 1] == '\n')
			bayer[fget_len - 1] = '\0';

		pixelformat = atoi(bayer);
		switch (pixelformat) {
		case V4L2_PIX_FMT_SBGGR8:
			sprintf(in_fmt, "%s", "BGGR8");
			buf_len = vi_fmt->format.width * vi_fmt->format.height;
			break;
		case V4L2_PIX_FMT_SGBRG8:
			sprintf(in_fmt, "%s", "GBRG8");
			buf_len = vi_fmt->format.width * vi_fmt->format.height;
			break;
		case V4L2_PIX_FMT_SGRBG8:
			sprintf(in_fmt, "%s", "GRBG8");
			buf_len = vi_fmt->format.width * vi_fmt->format.height;
			break;
		case V4L2_PIX_FMT_SRGGB8:
			sprintf(in_fmt, "%s", "RGGB8");
			buf_len = vi_fmt->format.width * vi_fmt->format.height;
			break;
		case V4L2_PIX_FMT_SBGGR10:
			sprintf(in_fmt, "%s", "BGGR10");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SGBRG10:
			sprintf(in_fmt, "%s", "GBRG10");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SGRBG10:
			sprintf(in_fmt, "%s", "GRBG10");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SRGGB10:
			sprintf(in_fmt, "%s", "RGGB10");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SBGGR12:
			sprintf(in_fmt, "%s", "BGGR12");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SGBRG12:
			sprintf(in_fmt, "%s", "GBRG12");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SGRBG12:
			sprintf(in_fmt, "%s", "GRBG12");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		case V4L2_PIX_FMT_SRGGB12:
			sprintf(in_fmt, "%s", "RGGB12");
			buf_len = vi_fmt->format.width * vi_fmt->format.height * 2;
			break;
		default:
			sprintf(in_fmt, "%s", "incorrect");
			break;
		}
		LOG("read pixelformat is %s\n", in_fmt);
	} else {
		vi_fmt->ptn_en = 0;
		fclose(fd);
		fd = NULL;
		return -1;
	}

	fclose(fd);
	fd = NULL;

	ptn_vaddr = read_bin_file(vi_fmt, raw_path, buf_len);
	if (!ptn_vaddr) {
		vi_fmt->ptn_en = 0;
		return -1;
	}
	memset(&vi_fmt->ptn_cfg, 0, sizeof(vi_fmt->ptn_cfg));
	vi_fmt->ptn_cfg.ptn_en = 1;
	vi_fmt->ptn_cfg.ptn_addr = ptn_vaddr;
	vi_fmt->ptn_cfg.ptn_size = buf_len;
	vi_fmt->ptn_cfg.ptn_w = vi_fmt->format.width;
	vi_fmt->ptn_cfg.ptn_h = vi_fmt->format.height;
	vi_fmt->ptn_cfg.ptn_fmt = pixelformat;
	vi_fmt->ptn_cfg.ptn_type = 1;

	return 0;
}

/*
 * start video node
 * returns CAP_ERR_NONE if OK, others if something went wrong
 */
int start_video(capture_params *cap_pa, capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	int fmt_changed = 0;

	if (!cap_pa || !cap_fmt || !VALID_VIDEO_SEL(cap_fmt->channel)) {
		LOG("%s: invalid params\n", __FUNCTION__);
		return CAP_ERR_INVALID_PARAMS;
	}

	// check whether set sensor or not
 	if (!g_sensor_set_flag) {
		LOG("%s: not set sensor input yet(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		return CAP_ERR_NOT_SET_INPUT;
	}

	pthread_mutex_lock(&cap_pa->locker);
	if (CAP_STATUS_ON & cap_pa->status) {
		if (cap_pa->priv_cap.vi_fmt.format.pixelformat != cap_fmt->format ||
			cap_pa->priv_cap.vi_fmt.format.width != cap_fmt->width ||
			cap_pa->priv_cap.vi_fmt.format.height != cap_fmt->height) {
			LOG("%s: vich%d format changes: fmt-%d, %dx%d -> fmt-%d, %dx%d\n", __FUNCTION__,
				cap_fmt->channel,
				cap_pa->priv_cap.vi_fmt.format.pixelformat,
				cap_pa->priv_cap.vi_fmt.format.width,
				cap_pa->priv_cap.vi_fmt.format.height,
				cap_fmt->format, cap_fmt->width, cap_fmt->height);

			disable_video(g_media_dev, cap_fmt->channel);
			exit_video(g_media_dev, cap_fmt->channel);
		} else {
			fmt_changed = 0;
			goto start_video_get_fmt;
		}
	}

	fmt_changed = 1;
	cap_pa->status &= ~CAP_STATUS_ON;

	ret = init_video(g_media_dev, cap_fmt->channel);
	if (ret) {
		ret = CAP_ERR_CH_INIT;
		LOG("%s: failed to init channel %d\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

	cap_pa->priv_cap.vich = cap_fmt->channel;
	cap_pa->priv_cap.timeout = 2000;  // ms
	cap_pa->priv_cap.vi_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	cap_pa->priv_cap.vi_fmt.memtype = V4L2_MEMORY_MMAP;
	cap_pa->priv_cap.vi_fmt.format.pixelformat = cap_fmt->format;
	cap_pa->priv_cap.vi_fmt.format.field = V4L2_FIELD_NONE;
	cap_pa->priv_cap.vi_fmt.format.width = cap_fmt->width;
	cap_pa->priv_cap.vi_fmt.format.height = cap_fmt->height;
	cap_pa->priv_cap.vi_fmt.nbufs = 3;
	cap_pa->priv_cap.vi_fmt.nplanes = cap_fmt->planes_count;
	cap_pa->priv_cap.vi_fmt.capturemode = V4L2_MODE_VIDEO;
	cap_pa->priv_cap.vi_fmt.fps = cap_fmt->fps;
	cap_pa->priv_cap.vi_fmt.wdr_mode = cap_fmt->wdr;
	cap_pa->priv_cap.vi_fmt.use_current_win = 1;  //!!! not to change sensor input format
	cap_pa->priv_cap.vi_fmt.index = cap_fmt->index;
	cap_pa->priv_cap.vi_fmt.pixel_num = MIPI_ONE_PIXEL;
	cap_pa->priv_cap.vi_fmt.tdm_speed_down_en = 0;
	cap_pa->priv_cap.vi_fmt.large_dma_merge_en = cap_fmt->framecount >> 16;
	cap_pa->priv_cap.vi_fmt.ptn_en = cap_fmt->framecount & 0xf;

	ret = set_video_ptn(g_media_dev, cap_fmt->channel, &cap_pa->priv_cap.vi_fmt);
	if (ret) {
		LOG("%s: failed to set ptn(channel %d)\n", __FUNCTION__, cap_fmt->channel);
	}

	ret = set_video_fmt(g_media_dev, cap_fmt->channel, &cap_pa->priv_cap.vi_fmt);
	if (ret) {
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_SET_FMT;
		LOG("%s: failed to set format(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

	ret = enable_video(g_media_dev, cap_fmt->channel);
	if (ret) {
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_ENABLE;
		LOG("%s: failed to enble channel %d\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

start_video_get_fmt:
	ret = get_video_fmt(g_media_dev, cap_fmt->channel, &cap_pa->priv_cap.vi_fmt);
	if (ret) {
		disable_video(g_media_dev, cap_fmt->channel);
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_GET_FMT;
		LOG("%s: failed to get format(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

	if (fmt_changed) {
		LOG("%s: vich%d format - fmt-%d, %dx%d@%d, wdr-%d, planes-%d[%d, %d, %d]\n", __FUNCTION__,
			cap_fmt->channel,
			cap_pa->priv_cap.vi_fmt.format.pixelformat,
			cap_pa->priv_cap.vi_fmt.format.width,
			cap_pa->priv_cap.vi_fmt.format.height,
			cap_pa->priv_cap.vi_fmt.fps,
			cap_pa->priv_cap.vi_fmt.wdr_mode,
			cap_pa->priv_cap.vi_fmt.nplanes,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline);
	}

	cap_fmt->width = cap_pa->priv_cap.vi_fmt.format.width;
	cap_fmt->height = cap_pa->priv_cap.vi_fmt.format.height;
	cap_fmt->format = cap_pa->priv_cap.vi_fmt.format.pixelformat;
	cap_fmt->planes_count = cap_pa->priv_cap.vi_fmt.nplanes;
	cap_fmt->width_stride[0] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline;
	cap_fmt->width_stride[1] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline;
	cap_fmt->width_stride[2] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline;

	cap_pa->status |= CAP_STATUS_ON;
	if (!(CAP_THREAD_RUNNING & cap_pa->status)) {
		add_work(&frame_loop_thread, cap_pa);
	}
	ret = CAP_ERR_NONE;

	if (fmt_changed) {
		msleep(1000);
		int isp_id = 0;
		ret = select_isp(isp_id, 1);//PC Tools input isp_id
		if (ret) {
			LOG("%s: failed to select isp %d\n", __FUNCTION__, cap_fmt->channel);
		}
	}

start_video_end:
	pthread_mutex_unlock(&cap_pa->locker);
	if (CAP_ERR_NONE == ret) {
		do {
			msleep(1);
			pthread_mutex_lock(&cap_pa->locker);
			if (CAP_THREAD_RUNNING & cap_pa->status) {
				pthread_mutex_unlock(&cap_pa->locker);
				break;
			}
			pthread_mutex_unlock(&cap_pa->locker);
		} while (1);
	}
	return ret;
}

int init_capture_module()
{
	int i = 0;
	capture_params *cap_handle = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();

	if (CAPTURE_INIT_YES == g_cap_init_status) {
		exit_capture_module();
	}

	if (!ini_cfg.enable) {
		g_media_dev = isp_md_open("/dev/media0");
		if (!g_media_dev) {
			LOG("%s: failed to init media\n", __FUNCTION__);
			return CAP_ERR_MPI_INIT;
		}
	}

	for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
		pthread_mutex_init(&cap_handle->locker, NULL);
		reset_cap_params(cap_handle);
	}

	pthread_mutex_init(&g_cap_locker, NULL);
	g_cap_init_status = CAPTURE_INIT_YES;

	LOG("%s: init done\n", __FUNCTION__);
	return CAP_ERR_NONE;
}

int exit_capture_module()
{
	int i = 0;
	capture_params *cap_handle = NULL;
	struct ToolsIniTuning_cfg ini_cfg = GetIniTuningEn();

	LOG("%s: ready to exit\n", __FUNCTION__);

	if (CAPTURE_INIT_YES == g_cap_init_status) {
		pthread_mutex_lock(&g_cap_locker);
		for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
			pthread_mutex_lock(&cap_handle->locker);
			cap_handle->status |= CAP_THREAD_STOP;  // set stop
			pthread_mutex_unlock(&cap_handle->locker);
		}
		msleep(32);

		for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
			do {
				msleep(32);
				pthread_mutex_lock(&cap_handle->locker);
				if (!(CAP_THREAD_RUNNING & cap_handle->status)) {
					pthread_mutex_unlock(&cap_handle->locker);
					break;
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} while (1);

			pthread_mutex_lock(&cap_handle->locker);
			if (CAP_STATUS_ON & cap_handle->status) {
				if (!ini_cfg.enable) {
					disable_video(g_media_dev, cap_handle->priv_cap.vich);
					exit_video(g_media_dev, cap_handle->priv_cap.vich);
				}
				cap_handle->status &= ~CAP_STATUS_ON;
			}
			pthread_mutex_unlock(&cap_handle->locker);
			pthread_mutex_destroy(&cap_handle->locker);
		}

		if (!ini_cfg.enable) {
			if (g_media_dev) {
				isp_md_close(g_media_dev);
				g_media_dev = NULL;
			}
		}

		pthread_mutex_unlock(&g_cap_locker);
		pthread_mutex_destroy(&g_cap_locker);
		g_cap_init_status = CAPTURE_INIT_NOT;
	}

	LOG("%s: exits\n", __FUNCTION__);
	return CAP_ERR_NONE;
}

int get_vich_status()
{
	capture_params *cap_handle = NULL;
	int i = 0, ret = 0;
	for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
		if (CAP_STATUS_ON & cap_handle->status) {
			ret |= (1 << i);
		}
	}
	return ret;
}

int set_sensor_input(const sensor_input *sensor_in)
{
	capture_params *cap_handle = NULL;
	int ret = CAP_ERR_NONE;

	if (sensor_in && VALID_VIDEO_SEL(sensor_in->channel)) {
		// check whether same format or not
		//sensor_set_flag = g_sensor_set_flag & (1<<sensor_in->channel);
		//LOG("%s: channel %d, set flag %d\n", __FUNCTION__, sensor_in->channel, sensor_set_flag);
		//if (sensor_set_flag) {
		//	return CAP_ERR_NONE;
		//}

		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + sensor_in->channel;

		pthread_mutex_lock(&cap_handle->locker);
		if (CAP_STATUS_ON & cap_handle->status) {
			disable_video(g_media_dev, sensor_in->channel);
			exit_video(g_media_dev, sensor_in->channel);
		}
		ret = init_video(g_media_dev, sensor_in->channel);
		if (ret) {
			ret = CAP_ERR_CH_INIT;
			LOG("%s: failed to init channel %d\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}
		cap_handle->status &= ~CAP_STATUS_ON;

		cap_handle->priv_cap.vich = sensor_in->channel;
		cap_handle->priv_cap.timeout = 2000;
		cap_handle->priv_cap.vi_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		cap_handle->priv_cap.vi_fmt.memtype = V4L2_MEMORY_MMAP;
		//cap_handle->priv_cap.vi_fmt.format.pixelformat = V4L2_PIX_FMT_NV12M;
		cap_handle->priv_cap.vi_fmt.format.pixelformat = sensor_in->format;
		cap_handle->priv_cap.vi_fmt.format.field = V4L2_FIELD_NONE;
		cap_handle->priv_cap.vi_fmt.format.width = sensor_in->width;
		cap_handle->priv_cap.vi_fmt.format.height = sensor_in->height;
		cap_handle->priv_cap.vi_fmt.nbufs = 3;
		cap_handle->priv_cap.vi_fmt.nplanes = 2;
		cap_handle->priv_cap.vi_fmt.capturemode = V4L2_MODE_VIDEO;
		cap_handle->priv_cap.vi_fmt.fps = sensor_in->fps;
		cap_handle->priv_cap.vi_fmt.wdr_mode = sensor_in->wdr;
		cap_handle->priv_cap.vi_fmt.use_current_win = 0;  //!!! try to change sensor input format
		cap_handle->priv_cap.vi_fmt.index = sensor_in->index;
		cap_handle->priv_cap.vi_fmt.pixel_num = MIPI_ONE_PIXEL;
		cap_handle->priv_cap.vi_fmt.tdm_speed_down_en = 0;
		cap_handle->priv_cap.vi_fmt.large_dma_merge_en = sensor_in->stitch_mode;
		cap_handle->priv_cap.vi_fmt.ptn_en = sensor_in->ptn_en;

//		if (cap_handle->priv_cap.vich) {
//			LOG("ldci video from awtuningapp, please set 160*90\n");
//			ldci_video_sel = TUNINGAPP_VIDEO_IN;
//		}
		ret = set_video_ptn(g_media_dev, sensor_in->channel, &cap_handle->priv_cap.vi_fmt);
		if (ret) {
			LOG("%s: failed to set ptn(channel %d)\n", __FUNCTION__, sensor_in->channel);
		}

		ret = set_video_fmt(g_media_dev, sensor_in->channel, &cap_handle->priv_cap.vi_fmt);
		if (ret) {
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_SET_FMT;
			LOG("%s: failed to set format(channel %d)\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}

		ret = enable_video(g_media_dev, sensor_in->channel);
		if (ret) {
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_ENABLE;
			LOG("%s: failed to enable channel %d\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}

		ret = get_video_fmt(g_media_dev, sensor_in->channel, &cap_handle->priv_cap.vi_fmt);
		if (ret) {
			disable_video(g_media_dev, sensor_in->channel);
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_GET_FMT;
			LOG("%s: failed to get format(channel %d)\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}

		video_set_flip(sensor_in->channel, 0, 0);

		LOG("%s: vich%d format - fmt-%d, %dx%d@%d, wdr-%d, planes-%d[%d, %d, %d]\n", __FUNCTION__,
			sensor_in->channel,
			cap_handle->priv_cap.vi_fmt.format.pixelformat,
			cap_handle->priv_cap.vi_fmt.format.width,
			cap_handle->priv_cap.vi_fmt.format.height,
			cap_handle->priv_cap.vi_fmt.fps,
			cap_handle->priv_cap.vi_fmt.wdr_mode,
			cap_handle->priv_cap.vi_fmt.nplanes,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline);

		g_sensor_set_flag |= (1<<sensor_in->channel);
		cap_handle->status |= CAP_STATUS_ON;
		if (!(CAP_THREAD_RUNNING & cap_handle->status)) {
			add_work(&frame_loop_thread, cap_handle);
		}
		ret = CAP_ERR_NONE;

set_sensor_input_end:
		pthread_mutex_unlock(&cap_handle->locker);
		if (CAP_ERR_NONE == ret) {
			do {
				msleep(1);
				pthread_mutex_lock(&cap_handle->locker);
				if (CAP_THREAD_RUNNING & cap_handle->status) {
					pthread_mutex_unlock(&cap_handle->locker);
					break;
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} while (1);
		}
		pthread_mutex_unlock(&g_cap_locker);
		return ret;
	}

	return -1;
}


int get_capture_buffer(capture_format *cap_fmt, sock_packet *comm_packet, int sock_fd)
{
	int ret = CAP_ERR_NONE;
	unsigned char *buffer = NULL;
	capture_params *cap_handle = NULL;
	unsigned char *uptr = NULL;
	unsigned int i, divide_total = cap_fmt->framecount, send_len = 0, send_cnt = 0, offset = 0, tmp, Stride_cnt[3];

	if (cap_fmt->planes_count < 1 || cap_fmt->planes_count > 3) {
		LOG("%s: cap_fmt->planes_count error : %d\n", __FUNCTION__, cap_fmt->planes_count);
	}

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		ret = start_video(cap_handle, cap_fmt);
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		if (CAP_ERR_NONE == ret) {
			// get frame
			//LOG("%s: ready to get frame(channel %d)\n", __FUNCTION__, cap_fmt->channel);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			pthread_mutex_lock(&cap_handle->locker);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info, cap_handle->priv_cap.timeout);
			//LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, frames_count, cap_fmt->channel);
			cap_handle->frame_count++;
			if (ret < 0) {
				ret = CAP_ERR_GET_FRAME;
				LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
			} else {
				if (cap_handle->frame_count % 50 == 0) {
					LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				}
				if (!divide_total) {//full image data
					buffer = cap_fmt->buffer;
					cap_fmt->length = 0;
					for (ret = 0; ret < cap_fmt->planes_count; ret++) {
						memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[ret],
							cap_handle->priv_cap.vi_frame_info.u32Stride[ret]);
						buffer += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
						cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
					}
					if((cap_fmt->format != V4L2_PIX_FMT_SBGGR8) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG8) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG8) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB8) &&
						(cap_fmt->format != V4L2_PIX_FMT_SBGGR10) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG10) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG10) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB10) &&
						(cap_fmt->format != V4L2_PIX_FMT_SBGGR12) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG12) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG12) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB12) &&
						(cap_fmt->format != V4L2_PIX_FMT_LBC_2_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_2_5X) &&
						(cap_fmt->format != V4L2_PIX_FMT_LBC_1_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_1_5X)) {
						if(cap_fmt->height % 16) { // height ALIGN
							uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
							for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
								*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
							}
						}
					}
				} else {
					//cap_fmt->length = 0;
					//for (ret = 0; ret < cap_fmt->planes_count; ret++) {
					//	cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
					//}
					Stride_cnt[0] = cap_handle->priv_cap.vi_frame_info.u32Stride[0];
					if((cap_fmt->format != V4L2_PIX_FMT_SBGGR8) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG8) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG8) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB8) &&
						(cap_fmt->format != V4L2_PIX_FMT_SBGGR10) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG10) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG10) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB10) &&
						(cap_fmt->format != V4L2_PIX_FMT_SBGGR12) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG12) &&
						(cap_fmt->format != V4L2_PIX_FMT_SGRBG12) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB12) &&
						(cap_fmt->format != V4L2_PIX_FMT_LBC_2_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_2_5X) &&
						(cap_fmt->format != V4L2_PIX_FMT_LBC_1_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_1_5X)) {
						if(cap_fmt->height % 16) { // height ALIGN
							Stride_cnt[0] -= cap_fmt->width * (16 - cap_fmt->height % 16);
						}
					}
					Stride_cnt[1] = Stride_cnt[0] + cap_handle->priv_cap.vi_frame_info.u32Stride[1];
					Stride_cnt[2] = Stride_cnt[1] + cap_handle->priv_cap.vi_frame_info.u32Stride[2];
					cap_fmt->length = Stride_cnt[cap_fmt->planes_count - 1];
					send_len = cap_fmt->length / divide_total + 1;
					comm_packet->data_length = htonl(cap_fmt->length);
					comm_packet->reserved[0] = htonl(cap_fmt->format);
					comm_packet->reserved[1] = htonl((cap_fmt->width << 16) | (cap_fmt->height & 0x0000ffff));
					comm_packet->reserved[2] = htonl((cap_fmt->width_stride[0] << 16) | (cap_fmt->width_stride[1] & 0x0000ffff));
					comm_packet->reserved[3] = htonl((cap_fmt->width_stride[2] << 16));
					//printf("u32Stride[0] = %d, u32Stride[1] = %d\n", cap_handle->priv_cap.vi_frame_info.u32Stride[0], cap_handle->priv_cap.vi_frame_info.u32Stride[1]);
					ret = sock_write_check_packet(__FUNCTION__, sock_fd, comm_packet, SOCK_CMD_PREVIEW, SOCK_DEFAULT_TIMEOUT);
					if (SOCK_RW_CHECK_OK == ret) {
						for (i = 0; i < divide_total; i++) {
							buffer = cap_fmt->buffer;
							offset = 0;
							if (send_cnt < Stride_cnt[0]) {
								memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[0] + send_cnt, min(send_len, Stride_cnt[0] - send_cnt));
								offset += min(send_len, Stride_cnt[0] - send_cnt);
								buffer = buffer + min(send_len, Stride_cnt[0] - send_cnt);
								send_cnt += min(send_len, Stride_cnt[0] - send_cnt);
								//printf("1 --- offset = %d, send_cnt = %d\n", offset, send_cnt);
							}
							if (cap_fmt->planes_count > 1 && offset < send_len && (send_cnt >= Stride_cnt[0]) && (send_cnt < Stride_cnt[1])) {
								if (offset) {
									memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[1], min(send_len - offset, Stride_cnt[1] - send_cnt));
									tmp = offset;
									offset += min(send_len - offset, Stride_cnt[1] - send_cnt);
									buffer = buffer + min(send_len - offset, Stride_cnt[1] - send_cnt);
									send_cnt += min(send_len - tmp, Stride_cnt[1] - send_cnt);
									//printf("2 --- offset = %d, send_cnt = %d\n", offset, send_cnt);
								} else {
									memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[1] + (send_cnt - Stride_cnt[0]), min(send_len, Stride_cnt[1] - send_cnt));
									offset += min(send_len, Stride_cnt[1] - send_cnt);
									buffer = buffer + min(send_len, Stride_cnt[1] - send_cnt);
									send_cnt += min(send_len, Stride_cnt[1] - send_cnt);
									//printf("2 --- offset = %d, send_cnt = %d\n", offset, send_cnt);
								}
							}
							if (cap_fmt->planes_count > 2 && offset < send_len && (send_cnt >= Stride_cnt[1]) && (send_cnt < Stride_cnt[2])) {
								if (offset) {
									memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[2], min(send_len - offset, Stride_cnt[2] - send_cnt));
									tmp = offset;
									offset += min(send_len - offset, Stride_cnt[2] - send_cnt);
									buffer = buffer + min(send_len - offset, Stride_cnt[2] - send_cnt);
									send_cnt += min(send_len - tmp, Stride_cnt[2] - send_cnt);
								} else {
									memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[2] + (send_cnt - Stride_cnt[1]), min(send_len, Stride_cnt[2] - send_cnt));
									offset += min(send_len, Stride_cnt[2] - send_cnt);
									buffer = buffer + min(send_len, Stride_cnt[2] - send_cnt);
									send_cnt += min(send_len, Stride_cnt[2] - send_cnt);
								}
							}
							sock_write(sock_fd, (const void *)cap_fmt->buffer, offset, SOCK_DEFAULT_TIMEOUT);
						}
						if (send_cnt != cap_fmt->length)
							LOG("%s: get frame error, send_cnt:%u != length:%d\n", __FUNCTION__, send_cnt, cap_fmt->length);
					}
				}
				//if (cap_fmt->length < cap_fmt->width * cap_fmt->height) {
				//	LOG("%s: %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt->length, cap_fmt->width, cap_fmt->height);
				//	ret = CAP_ERR_GET_FRAME;
				//} else {
					ret = CAP_ERR_NONE;
				//}
				release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
			}
			pthread_mutex_unlock(&cap_handle->locker);
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;
}

int get_capture_blockinfo(capture_format *cap_fmt, int GrayBlocksFlag, SRegion *region)
{
	int ret = CAP_ERR_NONE;
	unsigned char *buffer = NULL;
	capture_params *cap_handle = NULL;
	//unsigned char *uptr = NULL;
	unsigned int i, j, k, b_width, b_height;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		ret = start_video(cap_handle, cap_fmt);
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		if (CAP_ERR_NONE == ret) {
			// get frame
			//LOG("%s: ready to get frame(channel %d)\n", __FUNCTION__, cap_fmt->channel);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			pthread_mutex_lock(&cap_handle->locker);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info, cap_handle->priv_cap.timeout);
			//LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, frames_count, cap_fmt->channel);
			cap_handle->frame_count++;
			if (ret < 0) {
				ret = CAP_ERR_GET_FRAME;
				LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
			} else {
				if (cap_handle->frame_count % 50 == 0) {
					LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				}
				buffer = cap_fmt->buffer;
				cap_fmt->length = 0;

				for (i = 0; i < 20 ; i++) {
					if (GrayBlocksFlag & (1 << i)) {
						b_width = region[i].right - region[i].left + 1;
						b_height = region[i].bottom - region[i].top + 1;
						for (j = 0; j < b_height; j++) {
							for (k = 0; k < b_width; k++) {
								*buffer = *((unsigned char *)cap_handle->priv_cap.vi_frame_info.pVirAddr[0] + (region[i].top + j) * cap_fmt->width + region[i].left + k);
								buffer++;
							}
						}
						cap_fmt->length += b_width * b_height;
					}
				}
				//for (ret = 0; ret < cap_fmt->planes_count; ret++) {
				//	memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[ret],
				//		cap_handle->priv_cap.vi_frame_info.u32Stride[ret]);
				//	buffer += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
				//	cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
				//}

				//if (cap_fmt->length < cap_fmt->width * cap_fmt->height) {
				//	LOG("%s: %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt->length, cap_fmt->width, cap_fmt->height);
				//	ret = CAP_ERR_GET_FRAME;
				//} else {
				ret = CAP_ERR_NONE;
				//}
				release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
			}
			pthread_mutex_unlock(&cap_handle->locker);
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;
}


#ifdef ANDROID_VENCODE
int set_vencode_config(capture_format *cap_fmt, encode_param_t *encode_param, int type)
{
	if(cap_fmt->width == 0 || cap_fmt->height == 0) {
		LOG("%s: cap_fmt.width=%d height=%d is invaild, please check PC setting or socket comm\n",
			__FUNCTION__, cap_fmt->width, cap_fmt->height);
		return CAP_ERR_INVALID_PARAMS;
	}

	memset(encode_param, 0, sizeof(encode_param_t));
	encode_param->src_width = cap_fmt->width;
	encode_param->src_height = cap_fmt->height;
#if 0
	encode_param->dst_width = cap_fmt->width;
	encode_param->dst_height = cap_fmt->height;
#else
	if (type == SOCK_CMD_VENCODE_ENCPP_YUV) {
		encode_param->dst_width = cap_fmt->width;
		encode_param->dst_height = cap_fmt->height;
	} else {
		if (vencoder_tuning_param->base_cfg.DstPicWidth < cap_fmt->width ||
			vencoder_tuning_param->base_cfg.DstPicHeight < cap_fmt->height) {
			encode_param->dst_width = cap_fmt->width;
			encode_param->dst_height = cap_fmt->height;
		} else {
		    encode_param->dst_width = vencoder_tuning_param->base_cfg.DstPicWidth;
			encode_param->dst_height = vencoder_tuning_param->base_cfg.DstPicHeight;
		}
	}
#endif
	printf("encode_param width=%u height=%u\n", encode_param->src_width, encode_param->src_height);
	encode_param->frame_rate = cap_fmt->fps;
	//encode_param->maxKeyFrame = 30;
	encode_param->maxKeyFrame = vencoder_tuning_param->base_cfg.MaxKeyInterval;
	encode_param->encode_frame_num = 1;

	encode_param->bit_rate = vencoder_tuning_param->base_cfg.mBitRate;

	encode_param->qp_min = vencoder_tuning_param->QPcontrol_cfg.Min_I_Qp;
	encode_param->qp_max = vencoder_tuning_param->QPcontrol_cfg.Max_I_Qp;
	encode_param->Pqp_min = vencoder_tuning_param->QPcontrol_cfg.Min_P_Qp;
	encode_param->Pqp_max = vencoder_tuning_param->QPcontrol_cfg.Max_P_Qp;
	encode_param->mInitQp = vencoder_tuning_param->QPcontrol_cfg.InitQp;
	encode_param->bFastEncFlag = vencoder_tuning_param->base_cfg.mFastEncFlag;
	encode_param->mbPintraEnable = vencoder_tuning_param->base_cfg.mbPIntraEnable;
	encode_param->gray = vencoder_tuning_param->base_cfg.GrayEn;
	encode_param->mirror = vencoder_tuning_param->base_cfg.MirrorEn;
	encode_param->Rotate = vencoder_tuning_param->base_cfg.Rotate;
	encode_param->n3DNR = vencoder_tuning_param->d3d_cfg.d3d_en;
	encode_param->SbmBufSize = vencoder_tuning_param->base_cfg.SbmBufSize;
	encode_param->MaxReEncodeTimes = vencoder_tuning_param->super_frame_cfg.MaxReEncodeTimes;
	if (type == SOCK_CMD_VENCODE_ENCPP_YUV)
		encode_param->encode_format = VENC_CODEC_H264;
	else
		encode_param->encode_format = vencoder_tuning_param->base_cfg.EncodeFormat;

	if (cap_fmt->format == V4L2_PIX_FMT_NV12M) {
		LOG("set_vencode_config: cap_fmt->format: NV12M\n");
		encode_param->picture_format = VENC_PIXEL_YUV420SP;
	} else if (cap_fmt->format == V4L2_PIX_FMT_NV21M) {
		LOG("set_vencode_config: cap_fmt->format: NV21M\n");
		encode_param->picture_format = VENC_PIXEL_YVU420SP;
	} else if (cap_fmt->format == V4L2_PIX_FMT_LBC_1_0X) {
		LOG("set_vencode_config: cap_fmt->format: V4L2_PIX_FMT_LBC_1_0X\n");
		encode_param->picture_format = VENC_PIXEL_LBC_AW;
	} else if (cap_fmt->format == V4L2_PIX_FMT_LBC_1_5X) {
		LOG("set_vencode_config: cap_fmt->format: V4L2_PIX_FMT_LBC_1_5X\n");
		encode_param->picture_format = VENC_PIXEL_LBC_AW;
		encode_param->bLbcLossyComEnFlag1_5x = 1;
	} else if (cap_fmt->format == V4L2_PIX_FMT_LBC_2_0X) {
		LOG("set_vencode_config: cap_fmt->format: V4L2_PIX_FMT_LBC_2_0X\n");
		encode_param->picture_format = VENC_PIXEL_LBC_AW;
		encode_param->bLbcLossyComEnFlag2x = 1;
	} else if (cap_fmt->format == V4L2_PIX_FMT_LBC_2_5X) {
		LOG("set_vencode_config: cap_fmt->format: V4L2_PIX_FMT_LBC_2_5X\n");
		encode_param->picture_format = VENC_PIXEL_LBC_AW;
		encode_param->bLbcLossyComEnFlag2_5x = 1;
	} else {
		LOG("set_vencode_config: cap_fmt->format: unknow or unknow support\n");
		encode_param->picture_format = VENC_PIXEL_YUV420SP;
	}

#ifdef INPUTSOURCE_FILE
	unsigned int nAlignW, nAlignH;

	encode_param->encode_frame_num = 30;
	memset(&encode_param->bufferParam, 0 ,sizeof(VencAllocateBufferParam));
	//* ve require 16-align
	nAlignW = (encode_param->src_width + 15)& ~15;
	nAlignH = (encode_param->src_height + 15)& ~15;
	encode_param->bufferParam.nSizeY = nAlignW*nAlignH;
	encode_param->bufferParam.nSizeC = nAlignW*nAlignH/2;
	encode_param->bufferParam.nBufferNum = 1;
	/******** end set bufferParam param********/

	encode_param->picture_format = VENC_PIXEL_YUV420P;
	encode_param->src_size = encode_param->src_width * encode_param->src_height;
	encode_param->dts_size = encode_param->src_size;
	strcpy((char*)encode_param->input_path,  "/tmp/1080.yuv");
	strcpy((char*)encode_param->output_path, "/tmp/1080p.h264");
	encode_param->in_file = fopen(encode_param->input_path, "r");
	if(encode_param->in_file == NULL)
	{
		printf("open in_file fail\n");
		return CAP_ERR_INVALID_PARAMS;
	}

	encode_param->out_file = fopen(encode_param->output_path, "wb");
	if(encode_param->out_file == NULL)
	{
		printf("open out_file fail\n");
		fclose(encode_param->in_file);
		return CAP_ERR_INVALID_PARAMS;
	}
#endif

	encode_param->debug_gdc_en = vencoder_tuning_param->base_cfg.GdcEn;
	return CAP_ERR_NONE;
}

int get_capture_vencode_buffer(capture_format *cap_fmt, encode_param_t *encode_param, int type)
{
	int i;
	int ret = CAP_ERR_NONE;
	capture_params *cap_handle = NULL;
	unsigned char *buffer = NULL;
	VencInputBuffer *inputBuffer = &encode_param->inputBuffer;
	VencOutputBuffer *outputBuffer = &encode_param->outputBuffer;
	unsigned char *uptr = NULL;

	if (cap_fmt == NULL) {
		LOG("%s: cap_fmt is null\n", __FUNCTION__);
		return CAP_ERR_INVALID_PARAMS;
	}
	buffer = cap_fmt->buffer;
	if (VALID_VIDEO_SEL(cap_fmt->channel)) {
		if (type == SOCK_CMD_VENCODE_PPSSPS) {
			ret = EncoderStart(encode_param, NULL, NULL, VENCODE_CMD_HEAD_PPSSPS);
			if (ret) {
				LOG("%s: detect encode error!!@%d\n", __FUNCTION__, __LINE__);
				ret = CAP_ERR_VENCODE_PPSSPS;
			} else {
				//Encoder data save in sps_pps_data
				cap_fmt->length = encode_param->sps_pps_data.nLength;
				memcpy(buffer, encode_param->sps_pps_data.pBuffer, cap_fmt->length);
				ret = CAP_ERR_NONE;
			}
		} else if (type == SOCK_CMD_VENCODE_STREAM) {
			pthread_mutex_lock(&g_cap_locker);
			cap_handle = g_cap_handle + cap_fmt->channel;
			ret = start_video(cap_handle, cap_fmt);
			if (CAP_ERR_NONE == ret) {
				pthread_mutex_lock(&cap_handle->locker);
				ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich,
									  &cap_handle->priv_cap.vi_frame_info,
									  cap_handle->priv_cap.timeout);
				cap_handle->frame_count++;
				if (ret < 0) {
					ret = CAP_ERR_GET_FRAME;
					LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				} else {
					inputBuffer->pAddrVirY = (unsigned char*)cap_handle->priv_cap.vi_frame_info.pVirAddr[0];
					inputBuffer->pAddrVirC = (unsigned char*)cap_handle->priv_cap.vi_frame_info.pVirAddr[1];
#if (ISP_VERSION == 600 || ISP_VERSION == 603)
					inputBuffer->pAddrPhyY = (unsigned char*)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[0];
					inputBuffer->pAddrPhyC = (unsigned char*)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[1];
#else
					inputBuffer->pAddrPhyY = (unsigned long)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[0];
					inputBuffer->pAddrPhyC = (unsigned long)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[1];
#endif
					ret = EncoderStart(encode_param, inputBuffer, outputBuffer, VENCODE_CMD_STREAM);
					if (ret) {
						LOG("%s: detect video encode error!!@%d\n", __FUNCTION__, __LINE__);
						ret = CAP_ERR_VENCODE_STREAM;
					} else {
						//Encoder data save in outputBuffer
						cap_fmt->length = outputBuffer->nSize0 + outputBuffer->nSize1;
						memcpy(buffer, outputBuffer->pData0, outputBuffer->nSize0);
						if (outputBuffer->nSize1) {
							buffer += outputBuffer->nSize0;
							memcpy(buffer, outputBuffer->pData1, outputBuffer->nSize1);
						}
						//when we do the job with outputBuffer, should call this function to free it
						ret = EncoderFreeOutputBuffer(&encode_param->outputBuffer);
						if (ret) {
							LOG("%s free video encode outputBuffer faile\n", __FUNCTION__);
							ret = CAP_ERR_VENCODE_FREEBUFFER;
						}
					}
					release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} else {
				LOG("%s: start_video fail\n", __FUNCTION__);
			}
			pthread_mutex_unlock(&g_cap_locker);
		} else if (type == SOCK_CMD_VENCODE_ENCPP_YUV) {
			pthread_mutex_lock(&g_cap_locker);
			cap_handle = g_cap_handle + cap_fmt->channel;
			ret = start_video(cap_handle, cap_fmt);
			if (CAP_ERR_NONE == ret) {
				pthread_mutex_lock(&cap_handle->locker);
				ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich,
									  &cap_handle->priv_cap.vi_frame_info,
									  cap_handle->priv_cap.timeout);
				cap_handle->frame_count++;
				if (ret < 0) {
					ret = CAP_ERR_GET_FRAME;
					LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				} else {
					inputBuffer->pAddrVirY = (unsigned char*)cap_handle->priv_cap.vi_frame_info.pVirAddr[0];
					inputBuffer->pAddrVirC = (unsigned char*)cap_handle->priv_cap.vi_frame_info.pVirAddr[1];
#if (ISP_VERSION == 600 || ISP_VERSION == 603)
					inputBuffer->pAddrPhyY = (unsigned char*)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[0];
					inputBuffer->pAddrPhyC = (unsigned char*)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[1];
#else
					inputBuffer->pAddrPhyY = (unsigned long)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[0];
					inputBuffer->pAddrPhyC = (unsigned long)cap_handle->priv_cap.vi_frame_info.u32PhyAddr[1];
#endif
					ret = EncoderStart(encode_param, inputBuffer, outputBuffer, VENCODE_CMD_STREAM);
					if (ret) {
						LOG("%s: detect video encode error!!@%d\n", __FUNCTION__, __LINE__);
						ret = CAP_ERR_VENCODE_STREAM;
					} else {
						if (cap_fmt->height % 16 == 0) {
							cap_fmt->length = cap_fmt->width * cap_fmt->height * 3 / 2;
						} else {
							cap_fmt->length = cap_fmt->width * ((cap_fmt->height / 16) + 1) * 16 * 3 / 2;
						}
						EncoderGetWbYuv(encode_param, buffer, cap_fmt->length);
						if(cap_fmt->height % 16) { // height ALIGN
							uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
							for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
								*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
							}
						}
						//when we do the job with outputBuffer, should call this function to free it
						ret = EncoderFreeOutputBuffer(&encode_param->outputBuffer);
						if (ret) {
							LOG("%s free video encode outputBuffer faile\n", __FUNCTION__);
							ret = CAP_ERR_VENCODE_FREEBUFFER;
						}
					}
					release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} else {
				LOG("%s: start_video fail\n", __FUNCTION__);
			}
			pthread_mutex_unlock(&g_cap_locker);
		}
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;

}
#endif

int get_capture_buffer_transfer(capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	unsigned char *buffer = NULL;
	capture_params *cap_handle = NULL;
	int i = 0;
	unsigned char *uptr = NULL;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		ret = start_video(cap_handle, cap_fmt);
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		if (CAP_ERR_NONE == ret) {
			// get frame
			//LOG("%s: ready to get frame(channel %d)\n", __FUNCTION__, cap_fmt->channel);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			pthread_mutex_lock(&cap_handle->locker);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			cap_fmt->length = 0;
			buffer = cap_fmt->buffer;
			for(i=0; i<cap_fmt->framecount; i++){//save cap_fmt->framecount frames
				ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info, cap_handle->priv_cap.timeout);
				//LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				cap_handle->frame_count++;
				if (ret < 0) {
					ret = CAP_ERR_GET_FRAME;
					LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				} else {
					if (cap_handle->frame_count % 50 == 0) {
						LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
					}
					if( (cap_fmt->length +  cap_fmt->width * cap_fmt->height*2) <(1 << 29)){
						for (ret = 0; ret < cap_fmt->planes_count; ret++) {
							memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[ret],
								cap_handle->priv_cap.vi_frame_info.u32Stride[ret]);
							buffer += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
							cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
						}
						if((cap_fmt->format != V4L2_PIX_FMT_SBGGR8) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG8) &&
							(cap_fmt->format != V4L2_PIX_FMT_SGRBG8) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB8) &&
							(cap_fmt->format != V4L2_PIX_FMT_SBGGR10) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG10) &&
							(cap_fmt->format != V4L2_PIX_FMT_SGRBG10) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB10) &&
							(cap_fmt->format != V4L2_PIX_FMT_SBGGR12) && (cap_fmt->format != V4L2_PIX_FMT_SGBRG12) &&
							(cap_fmt->format != V4L2_PIX_FMT_SGRBG12) && (cap_fmt->format != V4L2_PIX_FMT_SRGGB12) &&
							(cap_fmt->format != V4L2_PIX_FMT_LBC_2_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_2_5X) &&
							(cap_fmt->format != V4L2_PIX_FMT_LBC_1_0X) && (cap_fmt->format != V4L2_PIX_FMT_LBC_1_5X)) {
							if(cap_fmt->height % 16) { // height ALIGN
								uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
								for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
									*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
								}
							}
						}
						//if (cap_fmt->length < cap_fmt->width * cap_fmt->height) {
						//	LOG("%s: %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt->length, cap_fmt->width, cap_fmt->height);
						//	ret = CAP_ERR_GET_FRAME;
						//} else {
							ret = CAP_ERR_NONE;
						//}
					}
					release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
				}
			}
			pthread_mutex_unlock(&cap_handle->locker);
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;
}

int start_raw_flow(capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	capture_params *cap_handle = NULL;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		// check format
		if (!(cap_fmt->format == V4L2_PIX_FMT_SBGGR8 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG8 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG8 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB8 ||
			cap_fmt->format == V4L2_PIX_FMT_SBGGR10 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG10 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG10 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB10 ||
			cap_fmt->format == V4L2_PIX_FMT_SBGGR12 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG12 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG12 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB12)) {
			LOG("%s: Not valid bayer format\n", __FUNCTION__);
			return CAP_ERR_INVALID_PARAMS;
		}

		// start channel
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		ret = start_video(cap_handle, cap_fmt);
		if (CAP_ERR_NONE == ret) { // video on ok
			if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
				// init raw flow
				ret = init_raw_flow(cap_fmt, CAPTURE_RAW_FLOW_QUEUE_SIZE);
				if (ret != ERR_RAW_FLOW_NONE) {
					ret = CAP_ERR_START_RAW_FLOW;
				} else {
					pthread_mutex_lock(&cap_handle->locker);
					cap_handle->status |= CAP_RAW_FLOW_START;
					pthread_mutex_unlock(&cap_handle->locker);
					do {
						msleep(1);
						pthread_mutex_lock(&cap_handle->locker);
						if (CAP_RAW_FLOW_RUNNING & cap_handle->status) {
							LOG("%s: vich%d done\n", __FUNCTION__, cap_handle->priv_cap.vich);
							pthread_mutex_unlock(&cap_handle->locker);
							break;
						}
						pthread_mutex_unlock(&cap_handle->locker);
					} while (1);
					ret = CAP_ERR_NONE;
				}
			} else {
				LOG("%s: raw flow is already running(vich%d)\n", __FUNCTION__, cap_handle->priv_cap.vich);
			}
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}
	return ret;
}

int stop_raw_flow(int channel)
{
	capture_params *cap_handle = NULL;
	if (!VALID_VIDEO_SEL(channel)) {
		return CAP_ERR_INVALID_PARAMS;
	}

	pthread_mutex_lock(&g_cap_locker);
	cap_handle = g_cap_handle + channel;
	pthread_mutex_lock(&cap_handle->locker);
	cap_handle->status &= ~CAP_RAW_FLOW_START;
	pthread_mutex_unlock(&cap_handle->locker);
	do {
		msleep(1);
		pthread_mutex_lock(&cap_handle->locker);
		if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
			LOG("%s: vich%d done\n", __FUNCTION__, cap_handle->priv_cap.vich);
			pthread_mutex_unlock(&cap_handle->locker);
			break;
		}
		pthread_mutex_unlock(&cap_handle->locker);
	} while (1);
	pthread_mutex_unlock(&g_cap_locker);

	if (exit_raw_flow() != ERR_RAW_FLOW_NONE) {
		return CAP_ERR_STOP_RAW_FLOW;
	} else {
		return CAP_ERR_NONE;
	}
}

int get_raw_flow_frame(capture_format *cap_fmt)
{
	capture_params *cap_handle = NULL;
	if (!cap_fmt || !VALID_VIDEO_SEL(cap_fmt->channel)) {
		return CAP_ERR_INVALID_PARAMS;
	}

	pthread_mutex_lock(&g_cap_locker);
	cap_handle = g_cap_handle + cap_fmt->channel;
	pthread_mutex_lock(&cap_handle->locker);
	if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
		pthread_mutex_unlock(&cap_handle->locker);
		pthread_mutex_unlock(&g_cap_locker);
		return CAP_ERR_RAW_FLOW_NOT_RUN;
	}
	pthread_mutex_unlock(&cap_handle->locker);
	pthread_mutex_unlock(&g_cap_locker);

	if (dequeue_raw_flow(cap_fmt) != ERR_RAW_FLOW_NONE) {
		return CAP_ERR_GET_RAW_FLOW;
	} else {
		return CAP_ERR_NONE;
	}
}

void *do_save_raw_flow(void *params)
{
	FILE *fp = NULL;
	capture_format cap_fmt;
	int ret = 0, frames_count = 0;

	if (params) {
		fp = (FILE *)params;
		cap_fmt.buffer = (unsigned char *)malloc(1 << 24); // 16M
		do {
			ret = dequeue_raw_flow(&cap_fmt);
			if (ERR_RAW_FLOW_NONE == ret) {
				LOG("%s: %d\n", __FUNCTION__, __LINE__);
				fwrite(cap_fmt.buffer, cap_fmt.length, 1, fp);
				LOG("%s: %d\n", __FUNCTION__, __LINE__);
				//fflush(fp);
				frames_count++;
			} else {
				if (ERR_RAW_FLOW_QUEUE_EMPTY == ret) {
					continue;
				} else {
					break;
				}
			}
		} while (1);
		fclose(fp);
		fp = NULL;
		free(cap_fmt.buffer);
		cap_fmt.buffer = NULL;
		LOG("%s: save done(frames %d)\n", __FUNCTION__, frames_count);
	}
	return 0;
}

void save_raw_flow(const char *file_name)
{
	FILE *fp = NULL;

	if (file_name) {
		fp = fopen(file_name, "wb");
		if (fp) {
			add_work(&do_save_raw_flow, fp);
		} else {
			LOG("%s: failed to open %s\n", __FUNCTION__, file_name);
		}
	}
}

int video_set_flip(int vich, int hflip, int vflip)
{
	if (vich >= HW_VIDEO_DEVICE_NUM || g_media_dev == NULL || g_media_dev->video_dev[vich] == NULL) {
		LOG("%s: set flip fault.(video=%d, g_media_dev=%p)\n", __FUNCTION__, vich, g_media_dev);
		return -1;
	}
	video_set_control(g_media_dev->video_dev[vich], V4L2_CID_HFLIP, hflip);
	video_set_control(g_media_dev->video_dev[vich], V4L2_CID_VFLIP, vflip);
	return 0;
}

int video_get_flip(int vich, int *hflip, int *vflip)
{
	if (vich >= HW_VIDEO_DEVICE_NUM || g_media_dev == NULL || g_media_dev->video_dev[vich] == NULL) {
		LOG("%s: get flip fault.(video=%d, g_media_dev=%p)\n", __FUNCTION__, vich, g_media_dev);
		return -1;
	}
	video_get_control(g_media_dev->video_dev[vich], V4L2_CID_HFLIP, hflip);
	video_get_control(g_media_dev->video_dev[vich], V4L2_CID_VFLIP, vflip);
	return 0;
}


