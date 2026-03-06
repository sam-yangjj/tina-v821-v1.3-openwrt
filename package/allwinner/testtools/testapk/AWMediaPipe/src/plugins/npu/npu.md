# YOLOv5 AI推理插件文档

## 概述

YOLOv5插件是MediaPipeX框架的AI推理插件，基于NPU（神经网络处理器）硬件加速实现。该插件提供实时目标检测功能，支持多线程并行推理，能够高效地在视频流中检测和识别各种物体。

> ⚠️ 程序在图像上绘制文字标签时，需要加载字体文件。请确保所需字体文件（如 DejaVuSans.ttf）与可执行文件放在同一目录，否则文字标签将无法正常显示。

## 主要功能

- **实时目标检测**：基于YOLOv5算法的高精度目标检测
- **多线程推理**：支持可配置的推理线程池，提高处理吞吐量
- **硬件加速**：充分利用NPU硬件加速，降低CPU负载
- **结果可视化**：自动在原图上绘制检测框和标签
- **零拷贝处理**：基于DMA缓冲区的高效数据传输

## 仅支持以下格式输入
| 项目         | 要求值                             | 说明                               |
|--------------|------------------------------------|------------------------------------|
| **分辨率**    | **640×640**                        | 模型固定输入大小，不支持自动缩放      |
| **格式**      | **RGB888（`PIXEL_FORMAT_RGB888`）** | 每像素3字节，顺序为 R-G-B             |
| **颜色空间**  | RGB                                | 不支持 YUV 或其他格式                |
| **通道顺序**  | R-G-B                              | 非 BGR 或 RGBA                      |



## API接口

### 创建插件

```c
#include "plugins/npu/yolov5_plugin.h"

// 创建YOLOv5推理插件
Plugin* create_yolov5_plugin(const char* name, int thread_count, const char* model_path);
```

**参数说明**：
- `name`：插件实例名称（可为NULL）
- `thread_count`：推理线程数量（建议1-4个）
- `model_path`：YOLOv5模型文件路径（.nbg格式）

## 使用示例

### 基本目标检测

```c
#include "plugins/npu/yolov5_plugin.h"

int main() {
    // 1. 创建并初始化插件
    Plugin* cam = create_v4l2_camera_plugin();
    if (!cam) {
        fprintf(stderr, "Failed to create V4L2 plugin\n");
        return;
    }

    V4L2CameraConfig cam_config;
    cam_config.camea_index = 0;
    cam_config.fps = 30;
    cam_config.width = 2592;
    cam_config.height = 1944;
    cam_config.pixel_format = PIXEL_FORMAT_YUV420P;
    if (cam->init(cam, &cam_config) != 0) {
        fprintf(stderr, "Failed to init V4L2 camera\n");
        destroy_v4l2_camera_plugin(cam);
        return;
    }

    Plugin* gdc = create_gdc_plugin("gdc");
    if (!gdc) {
        fprintf(stderr, "Failed to create GDC plugin\n");
        destroy_v4l2_camera_plugin(cam);
        return;
    }

    Plugin* g2d = create_g2d_plugin("g2d");
    g2d_set_output_format(g2d_2, PIXEL_FORMAT_RGB888);
    g2d_set_output_size(g2d_2, 640, 640);

    Plugin* yolo = create_yolov5_plugin("yolo", 5, "./model/yolov5.nb");
    if (yolo->init(yolo, NULL) != 0) {
        fprintf(stderr, "Failed to init YOLO plugin\n");
        goto cleanup;
    }

    plugin_link_many(cam, g2d, yolo, NULL);
    plugin_chain_start(cam);
    // 4. 获取检测结果
    for (int i = 0; i < 100; i++) {
        FrameBuffer* frame = yolo->pull(yolo);
        if (frame) {
            frame_get(frame);
            printf("检测帧 %d: %dx%d, 包含检测结果\n",
                   i, frame->width, frame->height);
            // frame中的图像已包含绘制的检测框
            frame_put(frame);
        }
    }

    // 5. 停止推理
    plugin_chain_stop(pipeline->cam);

    destroy_yolov5_plugin(yolo);
    destroy_g2d_plugin(g2d);
    destroy_v4l2_camera_plugin(cam);

    return 0;
}
```


## 模型支持

### 支持的模型格式

- **NBG格式**：.ngb格式
- **输入尺寸**：640x640（标准YOLO输入）
- **输入格式**：RGB888
- **模型类型**：YOLOv5s, YOLOv5m, YOLOv5l等


### 多线程架构

```
主线程：从上游拉取帧 → 转换为推理任务 → 分发到线程池
   ↓
推理线程池：
   Thread 1: [模型实例1] → 推理 → 绘制结果
   Thread 2: [模型实例2] → 推理 → 绘制结果
   ...
   ↓
结果队列：收集处理完成的帧 → 输出给下游
```

## 配置选项

### 线程数量选择

```c
// 根据性能需求选择合适的线程数
Plugin* yolo_fast = create_yolov5_plugin("Fast", 1, model);     // 低延迟
Plugin* yolo_balanced = create_yolov5_plugin("Balanced", 2, model); // 平衡
Plugin* yolo_throughput = create_yolov5_plugin("Throughput", 4, model); // 高吞吐
```

## 检测结果处理

### 自动可视化

插件会自动在原图上绘制：
- **检测框**：红色矩形框
- **类别标签**：物体类别名称
- **置信度**：检测置信度分数
- **坐标信息**：框的精确位置

### 检测参数

- **置信度阈值**：默认0.25（可在模型中配置）
- **NMS阈值**：默认0.45（非极大值抑制）
- **最大检测数**：默认100个物体

## 高级用法

### 多模型并行

```c
// 同时运行多个不同的检测模型
Plugin* yolo_person = create_yolov5_plugin("Person", 1, "/opt/person_det.nbg");
Plugin* yolo_vehicle = create_yolov5_plugin("Vehicle", 1, "/opt/vehicle_det.nbg");

```

## 错误处理

### 常见问题

**1. 模型加载失败**
```c
Plugin* yolo = create_yolov5_plugin("YOLO", 2, "/wrong/path/model.nbg");
if (!yolo) {
    printf("检查模型文件路径是否正确\n");
}
```

**2. NPU初始化失败**
```c
if (yolo->init(yolo, NULL) != 0) {
    printf("NPU硬件初始化失败，检查驱动\n");
    // 检查 /dev/vipcore 设备
}
```

**3. 内存不足**
```c
// 减少线程数或使用更小的模型
Plugin* yolo = create_yolov5_plugin("YOLO", 1, "/opt/yolov5s.nbg");
```

### 调试方法

```bash
# 检查NPU设备
ls -l /dev/vipcore
```

## 性能优化

### 建议配置

1. **输入预处理**：确保输入为640x640的RGB图像
2. **线程数量**：建议4-5个线程
3. **模型选择**：实时应用优先使用YOLOv5s
4. **内存管理**：及时释放不需要的帧引用


## 故障排除

### 检查清单

1. 确认NPU硬件和驱动正常
2. 验证模型文件格式和路径
3. 检查输入图像格式（必须是640x640 RGB）
4. 确认系统内存充足
5. 验证推理库版本兼容性
