# V4L2 相机采集插件文档

## 概述

V4L2相机插件是MediaPipeX框架的视频输入源插件，基于Linux Video4Linux2 (V4L2) API实现。该插件负责从摄像头设备采集视频帧，并将其转换为标准的FrameBuffer格式供下游插件处理。

## 主要功能

- **多设备支持**：自动检测和配置可用的摄像头设备
- **多格式支持**：支持多种像素格式（YUV420、NV12、RGB等）
- **零拷贝DMA**：使用DMA缓冲区实现高效的数据传输
- **硬件加速**：支持ISP（图像信号处理器）硬件加速
- **线程安全**：内置线程安全的帧缓冲区管理

## API接口

### 创建插件实例

```c
#include "plugins/v4l2src/v4l2_camera_plugin.h"

Plugin* create_v4l2_camera_plugin();
```

**返回值**：
- 成功：插件实例指针
- 失败：NULL

## 使用示例

### 基本使用

```c
#include "plugins/v4l2src/v4l2_camera_plugin.h"

int main() {
    // 1. 创建相机插件
    Plugin* camera = create_v4l2_camera_plugin("CAM_NAME");
    if (!camera) {
        printf("创建相机插件失败\n");
        return -1;
    }

    V4L2CameraConfig cam_config = {cam_index, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    camera->init(camera, &cam_config);

    // 3. 启动采集
    if (camera->start(camera) != 0) {
        printf("启动相机失败\n");
        return -1;
    }

    // 4. 获取视频帧
    for (int i = 0; i < 100; i++) {
        FrameBuffer* frame = camera->pull(camera);
        if (frame) {
            frame_get(frame);
            printf("获取帧 %d: %dx%d, 格式=%d\n",
                   i, frame->width, frame->height, frame->format);

            // 处理帧数据...

            // 释放帧
            frame_put(frame);
        }
    }

    // 5. 停止采集
    camera->stop(camera);

    return 0;
}
```

## 配置参数

### 默认配置

- **设备路径**：自动检测 `/dev/video0`, `/dev/video1` 等
- **分辨率**：优先使用设备支持的最高分辨率
- **像素格式**：优先使用硬件原生格式
- **帧率**：30fps（如果设备支持）
- **缓冲区数量**：4个（V4L2_BUFFERS_COUNT）

### 支持的像素格式

| 格式 | 描述 | 硬件支持 |
|------|------|----------|
| NV12 | YUV420 半平面格式 | ✓ |
| NV21 | YVU420 半平面格式 | ✓ |
| YUV420P | YUV420 平面格式 | ✓ |
| RGB888 | RGB 24位格式 | ○ |
| ARGB8888 | ARGB 32位格式 | ○ |

*✓ = 硬件原生支持，○ = 软件转换支持*

## 性能特性

### 零拷贝DMA传输

```c
// 框架自动处理DMA缓冲区
FrameBuffer* frame = camera->pull(camera);
// frame->data 指向DMA缓冲区
// frame->fd 是DMA文件描述符
// 无需额外的内存拷贝
```

### 内存管理

```c
// 获取帧时自动增加引用计数
FrameBuffer* frame = camera->pull(camera);

// 手动增加引用计数（如果需要在多个地方使用）
frame_get(frame);

// 释放引用（引用计数为0时自动释放内存）
frame_put(frame);
```


## 兼容性

### 支持的平台
- **Allwinner平台**：MR536等


## 性能优化建议

1. **使用硬件原生格式**：避免不必要的格式转换
2. **合理设置分辨率**：根据实际需求选择合适的分辨率
3. **DMA缓冲区数量**：默认4个缓冲区通常足够，可根据需要调整
4. **ISP配置**：合理配置ISP参数以获得最佳图像质量

## 故障排除

### 检查清单

1. 确认摄像头设备正常工作
2. 检查设备节点权限（`/dev/videoX`）
3. 验证V4L2驱动正常加载
4. 确认DMA内存充足
5. 检查设备是否被其他程序占用

### 常用调试命令

```bash
# 列出所有视频设备
ls /dev/video*

```
