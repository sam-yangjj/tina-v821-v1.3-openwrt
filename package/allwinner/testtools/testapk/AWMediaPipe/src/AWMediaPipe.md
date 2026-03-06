- # AWMediaPipe 图像处理流水线系统

## 1. 项目概览

**AWMediaPipe** 是一个基于插件架构构建的高性能图像处理流水线系统，主要面向 Allwinner 平台，支持多路视频采集、格式转换、图像校正、AI 推理、视频编码与输出等功能。

本系统的典型功能包括：

- **多路摄像头采集**（V4L2）

- **图像畸变校正**（GDC）

- **图像格式转换与缩放**（G2D）

- **NPU 目标检测**（YOLOv5）

- **视频编码**（H.264）

- **图像拼接与保存**

- **RTSP 网络推流**

- **统一的内存池管理（OutputBufferPool）**

**适用场景**：

- 视觉 AI 应用（行人检测、物体识别）

- 多摄像头拼接

- 低延迟视频传输

- 视频监控与存档

---

## 编译与运行

### 编译

```bash
# 编译 Pipeline

# 修改Makefile中的MODE变量

MODE = main_pipeline  #表示选择main_pipeline.c作为主程序编译

MODE = merge_pipeline #表示选择merge_pipeline.c作为主程序编译
```

编译完成后会生成AWMediaPipe可执行文件，将可执行文件push到板端

### 运行程序

**注意**：

1.如果选择**main_pipeline.c**作为主程序编译，则在执行前需要将以下文件push到板端的指定位置：

- 模型文件`src/files/yolov5.nb`放到板端可执行文件目录下的 model 目录下如: `/app/model`

- 字体文件`src/files/DejaVuSans.ttf` 放到板端可执行文件目录下如: `/app`

- gdc校正需要的的lut文件（使用2560*1440的像素格式）`src/files/2560_1440/test_x.txt` 和 `src/files/2560_1440/test_y.txt` 放到 `/mnt/UDISK` 目录下

- 视频捕获数据会以h264格式保存为 `/tmp/output.h264`

2.如果选择**merge_pipeline.c**作为主程序编译, 则需要确保/dev/video1 和/dev/video5 两个摄像头节点(需要配置设备树)可以同时出 640*480 PIXEL_FORMAT_YUV420P格式的数据，并且将gdc校正文件`src/files/640_480/test_x.txt` 和 `src/files/640_480/test_y.txt` 放到 `/mnt/UDISK `目录下


## 2. 源码结构

```
src

├── main_pipeline.c           # 单路完整 Pipeline 主程序

├── merge_pipeline.c          # 双路合并 Pipeline 主程序

├── plugins                   # 插件目录

│   ├── v4l2src                # 摄像头采集插件

│   ├── gdc                    # 畸变校正插件

│   ├── g2d                    # 缩放与格式转换插件

│   ├── Encoder                # H.264 编码插件

│   ├── npu/yolov5              # YOLOv5 推理插件

│   ├── rtsp                   # RTSP 推流插件

│   ├── filesink               # 文件保存插件

│   ├── tee                    # Tee 分流插件

│   ├── buffer_pool.c/.h       # 内存池管理

│   ├── frame_queue.c/.h       # 帧队列

│   └── plugin_interface.*     # 插件接口定义

├── utils

│   ├── log.c / log.h          # 日志系统

├── test_plugins.c             # 插件测试

└── README.md                  # 项目说明文档
```

**插件与源码对应关系**：

| 插件功能          | 源码路径                                   |

| ------------- | -------------------------------------- |

| 摄像头采集 (V4L2)  | `plugins/v4l2src/v4l2_camera_plugin.c` |

| 畸变校正 (GDC)    | `plugins/gdc/gdc_plugin.c`             |

| 缩放/格式转换 (G2D) | `plugins/g2d/g2d_plugin.c`             |

| 目标检测 (YOLOv5) | `plugins/npu/yolov5/yolov5_plugin.c`   |

| 视频编码 (H.264)  | `plugins/Encoder/encoder_plugin.c`     |

| 文件保存          | `plugins/filesink/filesink_plugin.c`   |

| RTSP 推流       | `plugins/rtsp/rtsp_plugin.c`           |

| 数据分发 (Tee)    | `plugins/tee/tee_plugin.c`             |

---

## 3. 插件架构

### 3.1 插件接口规范

所有插件需实现统一的接口定义（位于 `plugin_interface.h`）：

```c
typedef struct Plugin {

    int (*init)(struct Plugin* self, void* config);

    int (*start)(struct Plugin* self);

    int (*stop)(struct Plugin* self);

    FrameBuffer* (*pull)(struct Plugin* self);

    void (*push)(struct Plugin* self, FrameBuffer* fb);

    void (*destroy)(struct Plugin* self);

} Plugin;
```

插件可通过 `plugin_link_many()` 按顺序连接：

```c
plugin_link_many(p1, p2, p3, NULL);
```

---

### 3.2 内存与线程模型（零拷贝 DMA）

##### FrameBuffer（`plugins/plugin_interface.h`）

- **关键字段**

  `data`, `fd`, `width`, `height`, `size`, `format`, `pts`, `ref_count`, `release`, `user_data`, `memOps`

- **生命周期**

  - 获取：`frame_get()`

  - 归还：`frame_put()`（当引用计数归零时触发 `release` 或默认释放逻辑）

- **内存对齐**

  - 使用 `ALIGN_16B` 进行 16 字节对齐，确保 DMA 访问效率

---

#### 缓冲复用

- **输出缓冲池**（`plugins/buffer_pool.h`）

  提供缓冲对象的统一管理与复用接口：

  - `output_pool_init()`

  - `output_pool_get()`

  - `output_pool_release()`

  - `output_pool_destroy()`

- **帧队列**（`plugins/frame_queue.h`）

  - 基于有界阻塞队列实现（`FRAME_QUEUE_MAX_SIZE`）

  - 保证在生产者-消费者模型中安全传递帧数据

---

### 线程与拉流机制

- 各插件在 `start()` 调用后创建处理线程

- 线程循环从上游插件的 `pull()` 主动取帧

- 处理完成后将结果投递到自身的 `queue_out`

- 下游插件通过继续调用 `pull()` 实现流水线数据传递

## 4. 插件模块详解

### 4.1 v4l2_camera_plugin

**功能**：从摄像头采集图像帧（支持 V4L2），详见src/plugins/v4l2src/v4l2_camera.md

```c
Plugin* cam = create_v4l2_camera_plugin("cam0");



V4L2CameraConfig cam_cfg = {0, 640, 480, 30, PIXEL_FORMAT_YUV420P, 1};



cam->init(cam, &cam_cfg);
```

### 4.2 gdc_plugin

**注意**：使用gdc插件之前必须将对应像素格式的 lut_x.txt 和 lut_y.txt 文件放在/mnt/UDISK目录下

**功能**：图像畸变校正，详见src/plugins/gdc/gdc.md

```c
Plugin* gdc = create_gdc_plugin("gdc0");



GDCConfig gdc_cfg = {USER_WARP_CUSTOM, 640, 480, 640, 480};



gdc->init(gdc, &gdc_cfg);
```

### 4.3 g2d_plugin

**功能**：图像格式转换与缩放 ，详见src/plugins/g2d/g2d.md

```c
Plugin* g2d = create_g2d_plugin("g2d0");



g2d_set_output_format(g2d, PIXEL_FORMAT_RGB888);



g2d_set_output_size(g2d, 640, 640);
```

### 4.4 yolov5_plugin

**注意**：用于 YOLO 推理的上游插件的输出格式**必须**为：

- 输出格式：`PIXEL_FORMAT_RGB888`

- 输出分辨率：`640x640`

- 模型文件位置：程序运行前必须将模型文件放到指定的目录下，模型文件路径由create_yolov5_plugin的第3个参数决定

- 字体文件位置：程序运行前必须将字体文件src/plugins/npu/DejaVuSans.ttf放在可执行文件目录下

**功能**：YOLOv5 目标检测（NPU 加速），详见src/plugins/npu/npu.md

```c
Plugin* yolo = create_yolov5_plugin("yolo", 5, "./model/yolov5.nb");



yolo->init(yolo, NULL);
```

### 4.5 encoder_plugin

**功能**：H.264 视频编码，详见src/plugins/Encoder/encoder.md

```c
Plugin* encoder = create_encoder_plugin("h264enc");



encConfig cfg = {640, 480, 30, 2*1024*1024, PIXEL_FORMAT_YUV420P};



encoder->init(encoder, &cfg);
```

### 4.6 filesink_plugin

**功能**：保存输出到文件，文件路径由create_filesink_plugin的第二个参数指定，详见src/plugins/filesink/filesink.md

```c
Plugin* sink = create_filesink_plugin("filesink", "/tmp/output.h264");



sink->init(sink, NULL);
```

### 4.7 rtspsink_plugin

**功能**：rtsp实时推流，详见src/plugins/rtsp/rtsp.md
**注意**：目前的rtsp视频流需要使用ffmpeg进行拉流播放，暂时不支持vlc播放，播放命令示例：`ffplay -i rtsp://192.168.1.1：8554`，拉流端和板端必须处于同一个局域网内，确保可以互相ping通！
**参数设置**：参数1可任意设置，参数2：板端的IPv4地址，参数3：端口号，参数4：视频流帧率（实际帧率）
```c
Plugin* sink = create_rtsp_plugin("rtsp", "192.168.1.1", 8554, 30);



sink->init(sink, NULL);
```

---

## 5. 帧缓冲管理

### 5.1 内存池管理（OutputBufferPool）

`buffer_pool.h` 提供了统一的帧缓冲管理机制，避免频繁申请释放内存带来的性能开销。

```c
#define MAX_OUTPUT_BUFFERS 5



typedef struct {

    pthread_mutex_t lock;

    FrameBuffer* buffers[MAX_OUTPUT_BUFFERS];

    int available[MAX_OUTPUT_BUFFERS];

    int count;

    int width;

    int height;

    int format;

    int initialized;

    struct SunxiMemOpsS* mem_ops;

} OutputBufferPool;
```

核心函数：

- `output_pool_init()` 初始化缓冲池

- `output_pool_get()` 获取可用缓冲

- `output_pool_release()` 释放缓冲

- `output_pool_destroy()` 销毁缓冲池

---

### Frame Queue 模块说明

`frame_queue.c` 提供了一个**线程安全的帧队列**实现，主要用于多线程环境下的帧数据传递（例如视频采集线程与图像处理线程之间的数据交换）。

该模块支持：

- 阻塞推入/弹出

- 超时推入/弹出

- 强制推入（丢弃最老数据）

- 安全清空队列

- 引用计数管理帧资源

- 多线程并发安全

#### 数据结构

##### FrameQueue

```c
typedef struct {

    FrameBuffer* queue[FRAME_QUEUE_MAX_SIZE];

    int head;       // 队头索引

    int tail;       // 队尾索引

    int size;       // 当前队列大小

    pthread_mutex_t mutex;

    pthread_cond_t cond_nonempty; // 队列非空条件变量

    pthread_cond_t cond_nonfull;  // 队列未满条件变量

} FrameQueue;
```

---

#### API说明

- 初始化和销毁：初始化会设置队列索引为 0，分配并初始化互斥锁和条件变量。销毁会释放队列中残留的帧，并销毁互斥锁与条件变量。

```c
int frame_queue_init(FrameQueue* fq);



void frame_queue_destroy(FrameQueue* fq);
```

- 阻塞推入：当队列已满时阻塞等待，直到有空位或 `running_flag` 变为 0。

```c
int frame_queue_push(FrameQueue* fq, FrameBuffer* frame, int* running_flag);
```

- 超时推入：在队列满的情况下等待 `timeout_ms` 毫秒，超时返回 `-2`。

```c
int frame_queue_push_timeout(FrameQueue* fq, FrameBuffer* frame, int timeout_ms);
```

- 强制推入： 队列满时丢弃最老的帧并插入新帧。

```c
int frame_queue_push_force(FrameQueue* fq, FrameBuffer* frame);
```

- 阻塞弹出：当队列为空时阻塞等待，直到有新帧。

```c
FrameBuffer* frame_queue_pop(FrameQueue* fq);
```

- 超时弹出：当队列为空时等待 `timeout_ms` 毫秒，超时返回 `NULL`。

```c
FrameBuffer* frame_queue_pop_timeout(FrameQueue* fq, int timeout_ms);
```

- 清空队列：释放所有帧资源，将队列重置为空。

```c
void frame_queue_clear(FrameQueue* fq);
```

- ## 注意事项

1. `frame_queue_push` 和 `frame_queue_pop` 会阻塞，请在必要时使用超时版本。

2. 使用 `frame_queue_push_force` 可能会丢帧，请在能容忍数据丢失的场景下使用。

3. `FrameBuffer` 内存由生产者分配，消费者取到后需适时 `frame_put()` 或 `free_frame_buffer()`。

4. 销毁队列前应确保所有生产者与消费者线程已经退出。

## 6. Pipeline 连接示例

### 6.1 单路 NPU 推理 Pipeline（代码见src/main_pipeline.c）

```
Camera (V4L2)

     ↓

G2D (2592*1944 → 2560*1440  YUV420P)

     ↓

    GDC

     ↓

G2D (2560*1440 YUV420P → 640*640 RGB888)

     ↓

YOLOv5 (NPU 推理)

     ↓

G2D (640*640 RGB888 → 640*480 YUV420P)

     ↓

Encoder (H.264)

     ↓

Filesink (/tmp/img/output.h264)
```

### 6.2 双路图像合成 Pipeline（代码见src/merge_pipeline.c）

```
Camera1 → GDC → G2D → Pull →

                              Merge → PNG

Camera2 → GDC → G2D → Pull →
```

---

## 7. 插件销毁顺序建议

- 销毁顺序应是pipeline插件的倒序，例如：

```c
destroy_filesink_plugin(...);



destroy_encoder(...);



destroy_g2d_plugin(...);



destroy_yolov5_plugin(...);



destroy_v4l2_camera_plugin(...);
```

---
