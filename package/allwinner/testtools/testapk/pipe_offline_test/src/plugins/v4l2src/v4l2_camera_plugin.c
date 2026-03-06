// v4l2_camera_plugin.c
#include "v4l2_camera_plugin.h"
#include "../plugin_interface.h"
#include "../frame_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <pthread.h>
#include <linux/dma-buf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../../utils/log.h"
#include "../../utils/frame_check.h"
#if defined(__USE_VIN_ISP__)
#include "AWIspApi.h"
#include "sunxi_camera_v2.h"
#endif


// Replace IonMem with DMA heap fd and dma_buf fd array
#define V4L2_BUFFERS_COUNT  4
#define MAX_PLANES          2  // Assuming <= 2 planes (adjust as needed)

#define DEFAULT_WIDTH       2592
#define DEFAULT_HEIGHT      1944
#define DEFAULT_FPS         30

typedef struct {
    int dma_buf_fds[V4L2_BUFFERS_COUNT][MAX_PLANES];  // dma_buf fds per buffer/plane
    void* plane_start[V4L2_BUFFERS_COUNT][MAX_PLANES]; // mmap addresses
    size_t plane_lengths[V4L2_BUFFERS_COUNT][MAX_PLANES];
} DmaBufInfo;

typedef struct V4L2CameraPriv {
    int fd;
    int camera_index;
    int width;
    int height;
    int fps;
    PixelFormat format;

    int running;
    int num_planes;
    int sensor_type;
    int ispId;
    int ispGain;
    int ispExp;
    int sensorGain;
    int sensorExp;
    __u32	pixelformat;

    FrameQueue queue;
    DmaBufInfo dma_buf_info;

    int useIsp;
    int isOfflineTest;
#ifdef __USE_VIN_ISP__
	AWIspApi *ispPort;
#endif

    struct SunxiMemOpsS* memOps;
} V4L2CameraPriv;

static uint32_t pixel_format_to_v4l2_format(PixelFormat format) {
    switch (format) {
        case PIXEL_FORMAT_NV21:
            return V4L2_PIX_FMT_NV21;

        case PIXEL_FORMAT_NV12:
            return V4L2_PIX_FMT_NV12;

        case PIXEL_FORMAT_YV12:
            return V4L2_PIX_FMT_YVU420;  // YV12
        case PIXEL_FORMAT_YUV420P:
            return V4L2_PIX_FMT_YUV420;  // Also called I420
        case PIXEL_FORMAT_YUV422P:
            return V4L2_PIX_FMT_YUV422P;

        case PIXEL_FORMAT_RGB565:
            return V4L2_PIX_FMT_RGB565;

        case PIXEL_FORMAT_RGB888:
            return V4L2_PIX_FMT_RGB24;
        case PIXEL_FORMAT_BGR888:
            return V4L2_PIX_FMT_BGR24;

        case PIXEL_FORMAT_RGBA8888:
            return V4L2_PIX_FMT_ABGR32;  // Often interpreted this way in V4L2
        case PIXEL_FORMAT_ARGB8888:
            return V4L2_PIX_FMT_ARGB32;
        case PIXEL_FORMAT_ABGR8888:
            return V4L2_PIX_FMT_RGBA32;  // Swizzled

        case PIXEL_FORMAT_GRAY8:
            return V4L2_PIX_FMT_GREY;

        case PIXEL_FORMAT_H264:
            return V4L2_PIX_FMT_H264;

        default:
            return -1;  // Unsupported or unknown
    }
}

static int isOutputRawData(int format)
{
	if (format == V4L2_PIX_FMT_SBGGR8 ||
	    format == V4L2_PIX_FMT_SGBRG8 ||
	    format == V4L2_PIX_FMT_SGRBG8 ||
	    format == V4L2_PIX_FMT_SRGGB8 ||

	    format == V4L2_PIX_FMT_SBGGR10 ||
	    format == V4L2_PIX_FMT_SGBRG10 ||
	    format == V4L2_PIX_FMT_SGRBG10 ||
	    format == V4L2_PIX_FMT_SRGGB10 ||

	    format == V4L2_PIX_FMT_SBGGR12 ||
	    format == V4L2_PIX_FMT_SGBRG12 ||
	    format == V4L2_PIX_FMT_SGRBG12 ||
	    format == V4L2_PIX_FMT_SRGGB12)
		return 1;
	else
		return 0;
}

#ifdef __USE_VIN_ISP__
static int getSensorType(int fd)
{
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (fd == 0)
		return 0xFF000000;

	memset((void *)&ctrl, 0, sizeof(struct v4l2_control));
	memset((void *)&qc_ctrl, 0, sizeof(struct v4l2_queryctrl));
	ctrl.id = V4L2_CID_SENSOR_TYPE;
	qc_ctrl.id = V4L2_CID_SENSOR_TYPE;

	if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &qc_ctrl)) {
		perror(" query sensor type ctrl failed\n");
		return -1;
	}

	ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
	if (ret < 0) {
		printf(" get sensor type failed,errno(%d)\n", errno);
		return ret;
	}

	return ctrl.value;
}
#endif

#ifdef __USE_VIN_ISP__
static int getSensorGain(int fd)
{
        int ret = -1;
        struct v4l2_control ctrl;

        if (fd == 0)
                return 0xFF000000;

        memset((void *)&ctrl, 0, sizeof(struct v4l2_control));
        ctrl.id = V4L2_CID_GAIN;
        ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
        if(ret < 0){
                return ret;
        }
        return ctrl.value;
}
#endif

#ifdef __USE_VIN_ISP__
static int getSensorExp(int fd)
{
        int ret = -1;
        struct v4l2_control ctrl;

        if (fd == 0)
                return 0xFF000000;

        memset((void *)&ctrl, 0, sizeof(struct v4l2_control));
        ctrl.id = V4L2_CID_EXPOSURE;
        ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
        if(ret < 0){
                return ret;
        }
        return ctrl.value;
}
#endif

static void *read_bin_file(char *path, int *length)
{
	FILE *pfile;
	void *buf;
	int i;

	pfile = fopen(path, "rb");
	if (pfile == NULL) {
		printf("open %s fail\n", path);
		return NULL;
	}
	printf("open %s ok\n", path);
	fseek(pfile, 0, SEEK_END);
	*length = ftell(pfile);
	buf = (void *)malloc((*length + 1) * sizeof(char));
	rewind(pfile); *length = fread(buf, 1, *length, pfile);
	printf("%s size = %d, buf_addr %p\n", path, *length, buf);
	fclose(pfile);
	return buf;
}

static int v4l2_camera_init(Plugin* self, void* v4l2_config) {
    V4L2CameraPriv* priv = (V4L2CameraPriv*)malloc(sizeof(V4L2CameraPriv));
    if (!priv) return -1;
    memset(priv, 0, sizeof(*priv));

    V4L2CameraConfig* config = (V4L2CameraConfig*)v4l2_config;
    unsigned char camera_path[16];

    priv->useIsp = config->useIsp;
    priv->isOfflineTest = config->isOfflineTest;
    priv->width = config && config->width > 0 ? config->width : DEFAULT_WIDTH;
    priv->height = config && config->height > 0 ? config->height : DEFAULT_HEIGHT;
    priv->fps = config && config->fps > 0 ? config->fps : DEFAULT_FPS;

    PixelFormat pf = config ? config->pixel_format : PIXEL_FORMAT_NONE;
    if (pf <= PIXEL_FORMAT_UNKNOWN || pf >= PIXEL_FORMAT_MAX) {
        pf = PIXEL_FORMAT_YUV420P;
    }
    priv->format = pf;

    priv->pixelformat = pixel_format_to_v4l2_format(pf);
    priv->camera_index = config->camea_index;
    sprintf(camera_path, "%s%d", "/dev/video", priv->camera_index);
    priv->fd = open(camera_path, O_RDWR | O_NONBLOCK);
    if (priv->fd < 0) {
        printf("Cannot open %s",camera_path);
        free(priv);
        return -1;
    }

    struct v4l2_capability cap = {0};
    ioctl(priv->fd, VIDIOC_QUERYCAP, &cap);
    if (!(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        perror("Video device does not support multi-planar capture!\n");
        return -1;
    }



    #ifdef __USE_VIN_ISP__
    if(priv->useIsp){
        /* detect sensor type */
        priv->sensor_type = getSensorType(priv->fd);
        if (priv->sensor_type == V4L2_SENSOR_TYPE_RAW) {
            priv->ispPort = CreateAWIspApi();
            printf(" raw sensor use vin isp\n");
        }
    }
    #endif


    struct v4l2_input inp;
    memset((void *)&inp, 0, sizeof(inp));
	inp.index = 0;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(priv->fd, VIDIOC_S_INPUT, &inp) < 0) {
		perror("VIDIOC_S_INPUT failed! s_input");
		close(priv->fd);
		return -1;
	}

    if(priv->isOfflineTest){
        void *ptn_vaddr = NULL;
        struct vin_pattern_config ptn_cfg;
        int buf_len = 0;
        ptn_vaddr = read_bin_file("/tmp/source_RGGB10.raw", &buf_len);
        memset(&ptn_cfg, 0, sizeof(ptn_cfg));
        ptn_cfg.ptn_en = 1;
        ptn_cfg.ptn_addr = ptn_vaddr;
        ptn_cfg.ptn_size = buf_len;
        ptn_cfg.ptn_w = 640;
        ptn_cfg.ptn_h = 480;
        ptn_cfg.ptn_fmt = V4L2_PIX_FMT_SRGGB10;
        ptn_cfg.ptn_type = 1;
        if(ioctl(priv->fd, VIDIOC_VIN_PTN_CFG, &ptn_cfg) < 0){
            perror("VIDIOC_VIN_PTN_CFG failed!");
            close(priv->fd);
            return -1;
        }
        if (ptn_vaddr)
            free(ptn_vaddr);
    }

    // Set format (use multiplanar capture)
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.width = priv->width;
    fmt.fmt.pix_mp.height = priv->height;
    fmt.fmt.pix_mp.pixelformat = priv->pixelformat; // format with planes
    fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;

    if (ioctl(priv->fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("Set format failed");
        close(priv->fd);
        free(priv);
        return -1;
    }

    // 获取实际格式
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if(ioctl(priv->fd, VIDIOC_G_FMT, &fmt) < 0){
        perror("Get format failed");
        close(priv->fd);
        free(priv);
        return -1;
    }

    priv->pixelformat = fmt.fmt.pix_mp.pixelformat;
    priv->num_planes = fmt.fmt.pix_mp.num_planes;
    priv->height = fmt.fmt.pix_mp.height;
    priv->width = fmt.fmt.pix_mp.width;
    if (priv->num_planes > MAX_PLANES) {
        fprintf(stderr, "num_planes %d > MAX_PLANES %d, adjust MAX_PLANES\n",
                priv->num_planes, MAX_PLANES);
        close(priv->fd);
        free(priv);
        return -1;
    }

    // Request buffers, but specify memory = DMABUF
    struct v4l2_requestbuffers req = {0};
    req.count = V4L2_BUFFERS_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_DMABUF;

    if (ioctl(priv->fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("Request buffers failed");
        close(priv->fd);
        free(priv);
        return -1;
    }
    if (req.count < V4L2_BUFFERS_COUNT) {
        fprintf(stderr, "Driver allocated fewer buffers than requested\n");
    }

    // Open dma_heap device once
    priv->memOps = GetMemAdapterOpsS();
    SunxiMemOpen(priv->memOps);

    // Query each buffer to get plane lengths, allocate dma_bufs, mmap
    for (int i = 0; i < V4L2_BUFFERS_COUNT; i++) {
        struct v4l2_buffer buf = {0};
        struct v4l2_plane planes[MAX_PLANES] = {0};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;
        buf.length = priv->num_planes;
        buf.m.planes = planes;

        if (ioctl(priv->fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("Query buffer failed");
            SunxiMemClose(priv->memOps);
            close(priv->fd);
            free(priv);
            return -1;
        }

        for (int p = 0; p < priv->num_planes; p++) {
            priv->dma_buf_info.plane_lengths[i][p] = planes[p].length;
            priv->dma_buf_info.plane_start[i][p] = SunxiMemPalloc(priv->memOps,planes[p].length);
            // allocate dma_buf fd from dma_heap for each plane
            priv->dma_buf_info.dma_buf_fds[i][p] = SunxiMemGetBufferFd(priv->memOps,priv->dma_buf_info.plane_start[i][p]);
            if (priv->dma_buf_info.dma_buf_fds[i][p] < 0) {
                fprintf(stderr, "Failed alloc dma_buf for buf %d plane %d\n", i, p);
                SunxiMemClose(priv->memOps);
                close(priv->fd);
                free(priv);
                return -1;
            }

        }
    }

    // Queue all buffers with dma_buf fds
    for (int i = 0; i < V4L2_BUFFERS_COUNT; i++) {
        struct v4l2_buffer buf = {0};
        struct v4l2_plane planes[MAX_PLANES] = {0};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;
        buf.length = priv->num_planes;
        buf.m.planes = planes;

        for (int p = 0; p < priv->num_planes; p++) {
            planes[p].m.fd = priv->dma_buf_info.dma_buf_fds[i][p];
        }

        if (ioctl(priv->fd, VIDIOC_QBUF, &buf) < 0) {
            perror("Queue buffer failed");
            SunxiMemClose(priv->memOps);
            close(priv->fd);
            free(priv);
            return -1;
        }
    }

    frame_queue_init(&priv->queue);
    self->priv = priv;
    printf("v4l2_cam_init end\n");
    return 0;
}

static void v4l2_frame_release(FrameBuffer* frame, void* user_data) {
    V4L2CameraPriv* priv = (V4L2CameraPriv*)user_data;

    // 检查设备是否还在运行，只有在运行时才返回buffer给驱动
    if (priv && priv->running && priv->fd > 0) {
        struct v4l2_buffer buf = {0};
        struct v4l2_plane planes[MAX_PLANES] = {0};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = frame->buf_index;
        buf.length = priv->num_planes;
        buf.field = V4L2_FIELD_NONE;
        buf.m.planes = planes;

        for (int p = 0; p < priv->num_planes; p++) {
            buf.m.planes[p].m.fd = priv->dma_buf_info.dma_buf_fds[frame->buf_index][p];
        }

        if (ioctl(priv->fd, VIDIOC_QBUF, &buf) < 0) {
            // 只在调试模式下打印错误，避免正常停止时的误报
            if (priv->running) {
                perror("VIDIOC_QBUF in release failed");
            }
        }
    }

    pthread_mutex_destroy(&frame->ref_mutex);
    free(frame); // 或者返回到 frame pool
}

void update_fps_v4l2() {
    static int frame_count = 0;
    static time_t last_time = 0;

    frame_count++;

    time_t current_time = time(NULL);
    if (current_time != last_time) {
        LOG_INFO("FPS: %d", frame_count);
        frame_count = 0;
        last_time = current_time;
    }
}

static int save_data(const unsigned char *rgb, size_t size, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERROR("fail to open %s",filename);
        return -1;
    }

    size_t written = fwrite(rgb, 1, size, fp);
    fclose(fp);

    if (written != size) {
        LOG_ERROR("write error, expect to write %zu bytes, actual write %zu bytes", size, written);
        return -1;
    }

    return 0;
}
static void* v4l2_capture_thread(void* arg) {
    Plugin* self = (Plugin*)arg;
    V4L2CameraPriv* priv = (V4L2CameraPriv*)self->priv;
    int fd = priv->fd;
    int num_planes = priv->num_planes;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("Start streaming failed");
        return NULL;
    }

    #ifdef __USE_VIN_ISP__
	/* setting ISP */
    if(priv->useIsp){
        if (priv->sensor_type == V4L2_SENSOR_TYPE_RAW
            && !isOutputRawData(priv->pixelformat)) {
            priv->ispId = -1;
            priv->ispId = priv->ispPort->ispGetIspId(priv->camera_index);
            if (priv->ispId >= 0){
                printf("----ready to get ispExp and ispGain\n");
                priv->ispPort->ispStart(priv->ispId);
                priv->ispGain = priv->ispPort->ispGetIspGain(priv->ispId);
                priv->ispExp = priv->ispPort->ispGetIspExp(priv->ispId);
            }
        }
    }
    #endif
    static int index = 0;
    uint8_t* ref_frame = NULL;
    while (priv->running) {

        #ifdef __USE_VIN_ISP__
        if(priv->useIsp){
            priv->sensorGain = getSensorGain(priv->fd);
            priv->sensorExp = getSensorExp(priv->fd);
        }
        #endif

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };

        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            perror("select failed");
            break;
        }
        if (r == 0) {
            fprintf(stderr, "select timeout\n");
            continue;
        }

        struct v4l2_buffer buf = {0};
        struct v4l2_plane planes[MAX_PLANES] = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.length = num_planes;
        buf.m.planes = planes;

        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN)
                continue;
            perror("VIDIOC_DQBUF failed");
            break;
        }

        // Create FrameBuffer with zero copy: data points to mmap'd dma_buf plane 0 start
        FrameBuffer* frame = (FrameBuffer*)malloc(sizeof(FrameBuffer));
        if (!frame) {
            fprintf(stderr, "Failed to allocate frame\n");
            ioctl(fd, VIDIOC_QBUF, &buf); // requeue
            break;
        }

        frame->memOps = priv->memOps;
        frame->size = buf.m.planes[0].bytesused;
        //frame->data = SunxiMemPalloc(priv->memOps,buf.m.planes[0].bytesused);
        //SunxiMemCopy(priv->memOps,frame->data,priv->dma_buf_info.plane_start[buf.index][0],frame->size);
        //frame->fd = SunxiMemGetBufferFd(priv->memOps,frame->data);
        frame->width = priv->width;
        frame->height = priv->height;
        frame->format = priv->format;
        frame->data = priv->dma_buf_info.plane_start[buf.index][0];
        frame->Vir_C_addr = frame->data + frame->width * frame->width;
        frame->Phy_Y_addr = SunxiMemGetPhysicAddressCpu(frame->memOps, frame->data);
        frame->fd = priv->dma_buf_info.dma_buf_fds[buf.index][0];
        frame->pts = buf.timestamp.tv_sec * 1000000LL + buf.timestamp.tv_usec;
        frame->ref_count = 0;
        frame->release = v4l2_frame_release;
        frame->user_data = priv;
        frame->buf_index = buf.index;

        pthread_mutex_init(&frame->ref_mutex, NULL);

        if(index == 1){
            ref_frame = init_checker(frame->size);
        }

        if(check_frame(frame->data, ref_frame, frame->size, index++)){
            printf("[ERROR]第 %d 帧与基准帧不一致, %s 输出异常\n", index, self->name);
        }else{
            printf("%s: 第 %d 帧与基准帧一致\n", self->name, index);
        }

        if(frame_queue_push(&priv->queue, frame, &priv->running)){
            static int num = 0;
            v4l2_frame_release(frame, priv);
            LOG_INFO("lost frame num=%d", frame->width, frame->height, frame->fd, num++);
        }
        else{
            //LOG_INFO("v4l2 push frame: %dx%d, fd=%d, index=%d", frame->width, frame->height, frame->fd, index++);
        }
        //frame_queue_push_force(&priv->queue, frame);
    }

    LOG_INFO("VIDIOC_STREAMOFF\n");
    if (ioctl(priv->fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("Start streaming failed");
        return NULL;
    }
    LOG_INFO("VIDIOC_STREAMOFF success\n");
    LOG_INFO("v4l2 capture thread end\n");

    release_checker(ref_frame);
    return NULL;
}

static int v4l2_camera_start(Plugin* self) {
    V4L2CameraPriv* priv = (V4L2CameraPriv*)self->priv;
    if (!priv) return -1;
    priv->running = 1;

    if (pthread_create(&self->thread_id, NULL, v4l2_capture_thread, self) != 0) {
        perror("Failed to create capture thread");
        return -1;
    }

    return 0;
}

static int v4l2_camera_stop(Plugin* self) {
    V4L2CameraPriv* priv = (V4L2CameraPriv*)self->priv;
    if (!priv) return -1;

    // 1. 首先停止线程运行标志
    priv->running = 0;
    //pthread_cond_broadcast(&priv->queue.cond_nonempty);
    // pthread_cond_broadcast(&priv->queue.cond_nonfull);
    // 2. 等待capture线程结束

    if (self->thread_id) {
        pthread_join(self->thread_id, NULL);
        self->thread_id = 0;
    }
    return 0;
}

static int v4l2_camera_process(Plugin* self, FrameBuffer* frame) {
    // priv plugin does not accept input frames
    return -1;
}

static int v4l2_camera_connect(Plugin* self, Plugin* downstream) {
    if (!self) return -1;
    self->downstream = downstream;
    return 0;
}

FrameBuffer* v4l2_camera_pull_frame(Plugin* plugin) {
    if (!plugin || !plugin->priv) return NULL;
    V4L2CameraPriv* priv = (V4L2CameraPriv*)plugin->priv;
    return frame_queue_pop(&priv->queue);
}


Plugin* create_v4l2_camera_plugin(const char* name) {

    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    if (!plugin) return NULL;

    plugin->name = name ? strdup(name) : "v4l2_cam";
    plugin->init = v4l2_camera_init;
    plugin->start = v4l2_camera_start;
    plugin->stop = v4l2_camera_stop;
    plugin->process = v4l2_camera_process;
    plugin->pull = v4l2_camera_pull_frame;

    return plugin;
}


static void v4l2_camera_plugin_destroy(Plugin* plugin)
{
    LOG_INFO("v4l2_camera_plugin_destroy enter");
    V4L2CameraPriv* priv = (V4L2CameraPriv*)plugin->priv;
    if (priv) {
        // 确保设备已经停止
        if (priv->running) {
            plugin->stop(plugin);
        }

        #ifdef __USE_VIN_ISP__
        /* stop ISP */
        if(priv->useIsp){
            if (priv->sensor_type == V4L2_SENSOR_TYPE_RAW) {
                if (priv->ispId >= 0 && !isOutputRawData(priv->pixelformat))
                    priv->ispPort->ispStop(priv->ispId);

                DestroyAWIspApi(priv->ispPort);
                priv->ispPort = NULL;
            }
        }
        #endif

        // 1. 首先销毁frame队列（释放队列中可能引用DMA缓冲区的frame）
        frame_queue_destroy(&priv->queue);

        // 2. 然后释放DMA缓冲区（此时没有frame在引用它们）
        for (int i = 0; i < V4L2_BUFFERS_COUNT; i++) {
            for (int p = 0; p < priv->num_planes; p++) {
                if (priv->dma_buf_info.plane_start[i][p]) {
                    SunxiMemPfree(priv->memOps, priv->dma_buf_info.plane_start[i][p]);
                    priv->dma_buf_info.plane_start[i][p] = NULL;
                }
                if (priv->dma_buf_info.dma_buf_fds[i][p] > 0) {
                    close(priv->dma_buf_info.dma_buf_fds[i][p]);
                    priv->dma_buf_info.dma_buf_fds[i][p] = -1;
                }
            }
        }

        // 3. 关闭设备文件描述符
        if (priv->fd > 0) {
            close(priv->fd);
            priv->fd = -1;
        }

        // 4. 最后清理内存操作接口
        if (priv->memOps) {
            SunxiMemClose(priv->memOps);
            priv->memOps = NULL;
        }

        // 5. 释放私有数据结构
        free(priv);
        plugin->priv = NULL;
    }
    LOG_INFO("v4l2_camera_plugin_destroy\n");

}

void destroy_v4l2_camera_plugin(Plugin* plugin)
{
    if (plugin) {
        if (plugin->priv) {
            v4l2_camera_plugin_destroy(plugin);
        }
        if (plugin->name) {
            free(plugin->name);
            plugin->name = NULL;
        }
        free(plugin);
    }
}
