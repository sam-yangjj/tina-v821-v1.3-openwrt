
/*
 ******************************************************************************
 *
 * snapshot.c
 *
 * Hawkview ISP - snapshot.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//#include "media/sunxi_camera_v2.h"

#include <isp.h>
#include <video.h>
#include <isp_dev.h>

#define MAX_VIDEO_NUM 	16
#define ISP_SERVER_STOP 0
#define MEDIA_DEVICE "/dev/media0"

struct hw_isp_media_dev *media = NULL;
#define ISP_FLASH_TEST 0
#define VIPP_OSD_TEST 0
#define DISP_USE 0

char path_name[30];
int save_cnt;
int test_cnt = 10;
int fps = 30;
int wdr_mode = 0;
int large_dma_merge = 0;

void *rgb_tmp_buf;
unsigned int rgb_tmp_buf_len;

struct vi_info {
	int Chn;
	int isp_id;
	int s32MilliSec;
	struct video_fmt vfmt;
	struct video_buffer buffer;
};

#pragma pack(push, 1)
typedef struct {
    unsigned short type;      // "BM"
    unsigned int size;        // 文件总大小
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;      // 像素数据偏移量
} BMPFileHeader;

typedef struct {
    unsigned int size;         // DIB header 大小
    int width;
    int height;
    unsigned short planes;     // 必须为1
    unsigned short bit_count;  // 每像素位数：24 for RGB24
    unsigned int compression;  // 无压缩
    unsigned int image_size;   // 图像大小
    int x_pixels_per_meter;
    int y_pixels_per_meter;
    unsigned int colors_used;
    unsigned int colors_important;
} BMPInfoHeader;
#pragma pack(pop)

static void terminate(int sig_no)
{
	printf("Got signal %d, exiting ...\n", sig_no);
	usleep(20*1000);
	exit(1);
}

static void install_sig_handler(void)
{
	signal(SIGBUS, terminate);
	signal(SIGFPE, terminate);
	signal(SIGHUP, terminate);
	signal(SIGILL, terminate);
	signal(SIGKILL, terminate);
	signal(SIGINT, terminate);
	signal(SIGIOT, terminate);
	signal(SIGPIPE, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGSEGV, terminate);
	signal(SIGSYS, terminate);
	signal(SIGTERM, terminate);
	signal(SIGTRAP, terminate);
	signal(SIGUSR1, terminate);
	signal(SIGUSR2, terminate);
}

static void isp_server_start(int isp_id)
{
	/*
	 * media_open must after AW_MPI_VI_InitCh, otherwise, media_pipeline_get_head
	 * will not get sensor entity, and then lead to failure of calling isp_sensor_get_configs,
	 * because of sensor link will not enable until s_input is called.
	 */

	isp_init(isp_id);
	isp_run(isp_id);
}

static void isp_server_stop(int isp_id)
{
	isp_stop(isp_id);
	isp_pthread_join(isp_id);
	isp_exit(isp_id);
}

static void isp_server_wait_to_exit(int isp_id)
{
	isp_pthread_join(isp_id);
	isp_exit(isp_id);
}

static void read_next_bin_file(struct video_fmt *vi_fmt)
{
	int length;
	int start_conut, end_count;

	if (vi_fmt->ptn_file == NULL) {
		return;
	}

	start_conut = 0;
	end_count = vi_fmt->ptn_count_total;

	if (*vi_fmt->ptn_count >= end_count) {
		rewind(vi_fmt->ptn_file);
		fseek(vi_fmt->ptn_file, start_conut * vi_fmt->ptn_cfg.ptn_size, SEEK_CUR);
		*vi_fmt->ptn_count = start_conut;
	}

	length = fread(vi_fmt->ptn_cfg.ptn_addr, 1, vi_fmt->ptn_cfg.ptn_size, vi_fmt->ptn_file);
	if (length != 0) {
		if (*vi_fmt->ptn_count % 10 == 0)
			printf("read ptn buf%d size is %d\n", *vi_fmt->ptn_count, length);
		(*vi_fmt->ptn_count)++;
	} else
		printf("read ptn buf%d size is %d\n", *vi_fmt->ptn_count, length);

}

void yuv_to_rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
    int r_val = y + (1.402 * (v - 128));
    int g_val = y - (0.344 * (u - 128)) - (0.714 * (v - 128));
    int b_val = y + (1.772 * (u - 128));

    *r = (unsigned char)(r_val < 0 ? 0 : (r_val > 255 ? 255 : r_val));
    *g = (unsigned char)(g_val < 0 ? 0 : (g_val > 255 ? 255 : g_val));
    *b = (unsigned char)(b_val < 0 ? 0 : (b_val > 255 ? 255 : b_val));
}

int nv12_to_rgb24(int width, int height, struct video_buffer *nv21_buf, unsigned char *rgb_buf)
{
    const unsigned char *y_plane = nv21_buf->planes[0].mem;
    const unsigned char *uv_plane = nv21_buf->planes[1].mem;

    int rgb_index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int uv_index = ((y >> 1) * width + (x & ~1)); // 每两个像素共享一个 UV 对

            int u = uv_plane[uv_index];
            int v = uv_plane[uv_index + 1];

            int y_val = y_plane[y * width + x];

            unsigned char r, g, b;
            yuv_to_rgb(y_val, u, v, &r, &g, &b);

            rgb_buf[rgb_index++] = b;
            rgb_buf[rgb_index++] = g;
            rgb_buf[rgb_index++] = r;
        }
    }
    return 0;
}

int write_bmp(const char *filename, int width, int height, const unsigned char *rgb_data)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open output file");
        return -1;
    }

    int row_size = width * 3;
    int padding = (4 - (row_size % 4)) % 4;
    int padded_row_size = row_size + padding;
    int total_size = height * padded_row_size;

    BMPFileHeader file_header = {
        .type = 0x4D42,
        .size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + total_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
    };

    BMPInfoHeader info_header = {
        .size = sizeof(BMPInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bit_count = 24,
        .compression = 0,
        .image_size = total_size,
        .x_pixels_per_meter = 0,
        .y_pixels_per_meter = 0,
        .colors_used = 0,
        .colors_important = 0
    };

    fwrite(&file_header, sizeof(BMPFileHeader), 1, fp);
    fwrite(&info_header, sizeof(BMPInfoHeader), 1, fp);

    for (int y = height - 1; y >= 0; y--) { // BMP 存储顺序是倒的
        fwrite(rgb_data + y * row_size, row_size, 1, fp);
        for (int p = 0; p < padding; p++) {
            fputc(0, fp); // 补齐每行到4字节对齐
        }
    }

    ISP_PRINT("save file %s success!\n", filename);
    fclose(fp);
    return 0;
}

static void yuv_to_bmp_and_save(struct video_fmt *fmt, struct video_buffer *buffer)
{
    if (fmt->format.pixelformat != V4L2_PIX_FMT_NV12M) {
        ISP_ERR("only support V4L2_PIX_FMT_NV12M");
        return;
    }

    char bmp[128] = {0};
    uint64_t timestamp = (buffer->timestamp.tv_sec * 1000000) + buffer->timestamp.tv_usec;

    if (!rgb_tmp_buf) {
        rgb_tmp_buf_len = fmt->format.width * fmt->format.height * 3;
        rgb_tmp_buf = malloc(rgb_tmp_buf_len);
        if (!rgb_tmp_buf) {
            ISP_ERR("fatal error! malloc rgb tm buffer len %u fail!\n", rgb_tmp_buf_len);
            return;
        }
    }

    nv12_to_rgb24(fmt->format.width, fmt->format.height, buffer, rgb_tmp_buf);
    sprintf(bmp, "%s/%lu.bmp", path_name, timestamp);
    write_bmp(bmp, fmt->format.width, fmt->format.height, rgb_tmp_buf);
}

static void *loop_cap(void *pArg)
{
	struct vi_info *privCap = (struct vi_info *)pArg;
	struct isp_video_device *video = NULL;
	struct buffers_pool *pool = NULL;
	isp_image_params_t isp_image_params;
	int i = 0;

	printf("current vi channel is = %d\n", privCap->Chn);

	if (privCap->Chn >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[privCap->Chn]) {
		ISP_ERR("vi channel number %d is invalid!\n", privCap->Chn);
		return NULL;
	} else {
		video = media->video_dev[privCap->Chn];
	}

	pool = buffers_pool_new(video);
	if (NULL == pool)
		return NULL;

	if (video_req_buffers(video, pool) < 0)
		return NULL;
	video_get_fmt(video, &privCap->vfmt);
	for (i = 0; i < privCap->vfmt.nbufs; i++)
		video_queue_buffer(video, i);

	if (video_stream_on(video) < 0)
		return NULL;

	i = 0;
	memset(&isp_image_params, 0, sizeof isp_image_params);
	while (i < save_cnt) {
		if (video_wait_buffer(video, privCap->s32MilliSec) < 0) {
			if (privCap->vfmt.ptn_en) {
				read_next_bin_file(&privCap->vfmt);
				video_set_next_ptn(video, &privCap->vfmt);
			}
			continue;
		}

		if (privCap->vfmt.ptn_en) {
			read_next_bin_file(&privCap->vfmt);
			video_set_next_ptn(video, &privCap->vfmt);
		}

		if (video_dequeue_buffer(video, &privCap->buffer) < 0)
			goto disablech;

		i++;
        yuv_to_bmp_and_save(&privCap->vfmt, &privCap->buffer);

		if (video_queue_buffer(video, privCap->buffer.index) < 0)
			goto disablech;
    }
disablech:
	if (privCap->vfmt.ptn_en) {
		fclose(privCap->vfmt.ptn_file);
		if (privCap->vfmt.ptn_cfg.ptn_addr) {
			free(privCap->vfmt.ptn_cfg.ptn_addr);
			privCap->vfmt.ptn_cfg.ptn_addr = NULL;
		}
	}
	if (video_stream_off(video) < 0)
		return NULL;
	if (video_free_buffers(video) < 0)
		return NULL;
	buffers_pool_delete(video);
	goto vi_exit;
vi_exit:
	return NULL;
}

static void *read_bin_file(struct video_fmt *vi_fmt, char *path, int length)
{
	void *raw_data = NULL;
	long int total_length;

	if (length == 0)
		return NULL;

	vi_fmt->ptn_file = fopen(path, "rb");
	if (vi_fmt->ptn_file == NULL) {
		printf("open %s fail\n", path);
		return NULL;
	}
	printf("open %s ok\n", path);
	fseek(vi_fmt->ptn_file, 0, SEEK_END);
	total_length = ftell(vi_fmt->ptn_file);
	if (total_length <= 0) {
		printf("maybe raw buf is too large, get buf length err\n");
		fclose(vi_fmt->ptn_file);
		return NULL;
	}
	rewind(vi_fmt->ptn_file);
	vi_fmt->ptn_count_total = total_length/length;
	printf("%s total_length is %ld, raw conut is %d\n", path, total_length, vi_fmt->ptn_count_total);

	raw_data = (void *)malloc((length + 1) * sizeof(char));
	if (!raw_data) {
		printf("malloc raw buf err, size is %d\n", length + 1);
		fclose(vi_fmt->ptn_file);
		return NULL;
	}

	length = fread(raw_data, 1, length, vi_fmt->ptn_file);
	if (length == 0) {
		printf("read ptn size is %d\n", length);
		fclose(vi_fmt->ptn_file);
		free(raw_data);
		return NULL;
	}
	printf("buf_addr %p, size = %d\n", raw_data, length);

	return raw_data;
}

int main_test(int ch_num, int width, int height, int out_fmt)
{
	pthread_t thid[MAX_VIDEO_NUM];
	int ret, i, ch = -1;
	struct vi_info privCap[MAX_VIDEO_NUM];
	int buf_len = 0;
	void *ptn_vaddr = NULL;

	if (media == NULL) {
		media = isp_md_open(MEDIA_DEVICE);
		if (media == NULL) {
			ISP_PRINT("unable to open media device %s\n", MEDIA_DEVICE);
			return -1;
		}
	} else {
		ISP_PRINT("mpi_vi already init\n");
	}

	media_dev_init();

	if (ch_num > MAX_VIDEO_NUM)
		ch_num = MAX_VIDEO_NUM;

	if (ch_num == 0 || ch_num == 1 || ch_num == 2 || ch_num == 3) {
		ch = ch_num;
		ch_num = 4;
	} else if (ch_num == 4 || ch_num == 8 || ch_num == 12) {
		ch = ch_num;
		ch_num = 13;
	}

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		memset(&privCap[i], 0, sizeof(struct vi_info));
		/*Set Dev ID and Chn ID*/
		privCap[i].Chn = i;
		privCap[i].s32MilliSec = 2000;

		privCap[i].vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		privCap[i].vfmt.memtype = V4L2_MEMORY_MMAP;
		switch (out_fmt) {
		case 0: privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_SBGGR8; break;
		case 1: privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420M; break;
		case 2: privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420; break;
		case 3: privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_NV12M; break;
		default: privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420M; break;
		}
		privCap[i].vfmt.format.field = V4L2_FIELD_NONE;
		privCap[i].vfmt.format.width = width;
		privCap[i].vfmt.format.height = height;
		privCap[i].vfmt.nbufs = 5;
		privCap[i].vfmt.nplanes = 3;
		privCap[i].vfmt.fps = fps;
		privCap[i].vfmt.capturemode = V4L2_MODE_VIDEO;
		privCap[i].vfmt.use_current_win = 0;
		privCap[i].vfmt.wdr_mode = wdr_mode;
		privCap[i].vfmt.pixel_num = MIPI_ONE_PIXEL;
		privCap[i].vfmt.tdm_speed_down_en = 0;
		privCap[i].vfmt.large_dma_merge_en = large_dma_merge;
		privCap[i].vfmt.tdm_rxbuf_cnt = 0;
		privCap[i].vfmt.ptn_en = 0;

		if (privCap[i].vfmt.ptn_en) {
			memset(&privCap[i].vfmt.ptn_cfg, 0, sizeof(privCap[i].vfmt.ptn_cfg));
			privCap[i].vfmt.ptn_cfg.ptn_fmt = V4L2_PIX_FMT_SRGGB10;
			switch (privCap[i].vfmt.ptn_cfg.ptn_fmt) {
			case V4L2_PIX_FMT_SBGGR8:
			case V4L2_PIX_FMT_SGBRG8:
			case V4L2_PIX_FMT_SGRBG8:
			case V4L2_PIX_FMT_SRGGB8:
				buf_len = width * height;
				break;
			case V4L2_PIX_FMT_SBGGR10:
			case V4L2_PIX_FMT_SGBRG10:
			case V4L2_PIX_FMT_SGRBG10:
			case V4L2_PIX_FMT_SRGGB10:
			case V4L2_PIX_FMT_SBGGR12:
			case V4L2_PIX_FMT_SGBRG12:
			case V4L2_PIX_FMT_SGRBG12:
			case V4L2_PIX_FMT_SRGGB12:
				buf_len = width * height * 2;
				break;
			}
			ptn_vaddr = read_bin_file(&privCap[i].vfmt, "./1080p_raw10_20.bin", buf_len);
			if (!ptn_vaddr) {
				privCap[i].vfmt.ptn_en = 0;
			} else {
				privCap[i].vfmt.ptn_cfg.ptn_en = 1;
				privCap[i].vfmt.ptn_cfg.ptn_addr = ptn_vaddr;
				privCap[i].vfmt.ptn_cfg.ptn_size = buf_len;
				privCap[i].vfmt.ptn_cfg.ptn_w = width;
				privCap[i].vfmt.ptn_cfg.ptn_h = height;
				privCap[i].vfmt.ptn_cfg.ptn_type = 1;
			}
		}

		if (isp_video_open(media, i) < 0) {
			printf("isp_video_open vi%d failed\n", i);
			return -1;
		}

		if (video_set_fmt(media->video_dev[i], &privCap[i].vfmt) < 0) {
			printf("video_set_fmt failed\n");
			return -1;
		}

		video_get_fmt(media->video_dev[i], &privCap[i].vfmt);

		//video_set_ldci_mode(media->video_dev[i], 1);

		privCap[i].isp_id = video_to_isp_id(media->video_dev[i]);
		if (privCap[i].isp_id == -1)
			continue;

		/*Call isp server*/
		if (privCap[i].vfmt.large_dma_merge_en) {
			printf("isp%d server start!!!\n", 0);
			isp_set_stitch_mode(privCap[i].isp_id, STITCH_2IN1_LINEAR);
			isp_set_sync((0x1 << 0) | (0x1 << privCap[i].isp_id));
			isp_server_start(0);
		}
		printf("isp%d server start!!!\n", privCap[i].isp_id);
		isp_server_start(privCap[i].isp_id);

		/*Call Video Thread*/
		ret = pthread_create(&thid[i], NULL, loop_cap, (void *)&privCap[i]);
		if (ret < 0) {
			printf("pthread_create loop_cap Chn[%d] failed.\n", i);
			continue;
		}
	}

#if !ISP_SERVER_STOP
	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		printf("isp%d server wait to exit!!!\n", privCap[i].isp_id);
		isp_server_wait_to_exit(privCap[i].isp_id);
		if (privCap[i].vfmt.large_dma_merge_en) {
			printf("isp%d server wait to exit!!!\n", 0);
			isp_server_wait_to_exit(0);
		}
	}
#endif
	media_dev_exit();

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		printf("video%d wait to exit!!!\n", i);
		pthread_join(thid[i], NULL);
	}

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		isp_video_close(media, i);
	}

	if (media)
		isp_md_close(media);
	media = NULL;

	return 0;
}

int main(int argc __attribute__((__unused__)), char *argv[] __attribute__((__unused__)))
{
	int ret = 0, n = 0;
	int ch_num = 0;
	int width = 640;
	int height = 480;
	int out_fmt = 1;

	//install_sig_handler();

	memset(path_name, 0, sizeof (path_name));

	if (argc == 1) {
		sprintf(path_name, "/mnt/sdcard");
	} else if (argc == 2) {
		sprintf(path_name, "/mnt/sdcard");
		ch_num = atoi(argv[1]);
	} else if (argc == 4) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "/mnt/sdcard");
	} else if (argc == 5) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
	} else if (argc == 6) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
	} else if (argc == 7) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
	} else if (argc == 8) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
	} else if (argc == 9) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
		fps = atoi(argv[8]);
	} else if (argc == 10) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
		fps = atoi(argv[8]);
		wdr_mode = atoi(argv[9]);
	} else if (argc == 11) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
		fps = atoi(argv[8]);
		wdr_mode = atoi(argv[9]);
		large_dma_merge = atoi(argv[10]);
	} else {
		printf("please select the ch_num: 1~4 ......\n");
		scanf("%d", &ch_num);

		printf("please input the resolution: width height......\n");
		scanf("%d %d", &width, &height);

		printf("please input the frame saving path......\n");
		scanf("%15s", path_name);

		printf("please input the test out_fmt: 0~3......\n");
		scanf("%d", &out_fmt);

		printf("please input the save_cnt: >=1......\n");
		scanf("%d", &save_cnt);
	}

	for (n = 0; n < test_cnt; n++) {
		ret = main_test(ch_num, width, height, out_fmt);
		printf("cameraisp test %d, return %d\n", n + 1, ret);
	}

	return ret;
}

