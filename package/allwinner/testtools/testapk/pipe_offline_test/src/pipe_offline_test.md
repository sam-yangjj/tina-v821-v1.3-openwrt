- # pipe_offline_test 离线自动化测试

## 1. 项目概览

**pipe_offline_test** 是一个基于插件架构构建的离线自动化测试系统，主要面向 Allwinner 平台，支持无摄像头测试


## 运行

执行之前需要先修改设备树和对应配置，生成对应的虚拟摄像头节点，具体参考文档[https://alidocs.dingtalk.com/i/nodes/Obva6QBXJw9wZym9hly2y95kWn4qY5Pr?utm_scene=team_space]()

**注意**：执行程序之前需要将以下文件放到板端对应位置

- gdc校正需要的的lut文件（使用640*480的像素格式）`src/files/640_480/test_x.txt` 和 `src/files/640_480/test_y.txt` 放到 `/mnt/UDISK` 目录下

- raw数据文件 `src/files/source_RGGB10.raw` 放到 `/tmp` 目录下

- 执行程序，所有模块会将接收到的第一帧作为参考帧，后面的视频帧都会和参考帧进行比较，并且将比较结果实时打印在串口终端，如下

```
第 996 帧与基准帧不一致, g2d 输出异常
第 996 帧与基准帧不一致, jpegenc 输出异常
第 997 帧与基准帧不一致, v4l2cam 输出异常
第 997 帧与基准帧不一致, gdc 输出异常
```

## 2. 源码结构

```
src

├── pipe_offline_test.c        # 测试主程序

├── plugins                    # 插件目录

│   ├── v4l2src                # 摄像头采集插件

│   ├── gdc                    # 畸变校正插件

│   ├── g2d                    # 缩放与格式转换插件

│   ├── Encoder                # H.264 编码插件

│   ├── buffer_pool.c/.h       # 内存池管理

│   ├── frame_queue.c/.h       # 帧队列

│   └── plugin_interface.c/.h     # 插件接口定义

├── utils

│   ├── log.c / log.h          # 日志系统

│   ├── frame_check.c / frame_check.h         # 实时图像比较

└── pipe_offline_test.md                  # 说明文档
```

### 线程与拉流机制

- 各插件在 `start()` 调用后创建处理线程

- 线程循环从上游插件的 `pull()` 主动取帧

- 处理完成后将结果投递到自身的 `queue_out`

- 下游插件通过继续调用 `pull()` 实现流水线数据传递

## 3. 插件模块详解

### 3.1 v4l2_camera_plugin

**功能**：从摄像头采集图像帧（支持 V4L2），详见src/plugins/v4l2src/v4l2_camera.md

```c
Plugin* cam = create_v4l2_camera_plugin("cam0");

V4L2CameraConfig cam_cfg = {0, 640, 480, 30, PIXEL_FORMAT_YUV420P, 1};

cam->init(cam, &cam_cfg);
```

### 3.2 gdc_plugin

**注意**：使用gdc插件之前必须将对应像素格式的 lut_x.txt 和 lut_y.txt 文件放在/mnt/UDISK目录下

**功能**：图像畸变校正，详见src/plugins/gdc/gdc.md

```c
Plugin* gdc = create_gdc_plugin("gdc0");

GDCConfig gdc_cfg = {USER_WARP_CUSTOM, 640, 480, 640, 480};

gdc->init(gdc, &gdc_cfg);
```

### 3.3 g2d_plugin

**功能**：图像格式转换与缩放 ，详见src/plugins/g2d/g2d.md

```c
Plugin* g2d = create_g2d_plugin("g2d0");


g2d_set_output_format(g2d, PIXEL_FORMAT_RGB888);


g2d_set_output_size(g2d, 640, 640);
```

### 3.4 encoder_plugin

**功能**：H.264 视频编码，详见src/plugins/Encoder/encoder.md

```c
Plugin* encoder = create_encoder_plugin("h264enc");

encConfig cfg = {640, 480, 30, 2*1024*1024, PIXEL_FORMAT_YUV420P};

encoder->init(encoder, &cfg);
```

## 4. 帧缓冲管理

### 4.1 内存池管理（OutputBufferPool）

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

## 5. pipe_offline_test 连接示例

### 5.1 单路离线自动化测试（代码见src/pipe_offline_test.c）

```
Camera (V4L2) (640*480 YUV420P)
     ↓
    GDC
     ↓
    G2D (rotate 180)
     ↓
  Encoder (JPEG)
```
