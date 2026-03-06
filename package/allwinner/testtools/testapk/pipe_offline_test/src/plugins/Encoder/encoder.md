# H.264 编码插件文档

## 概述

H.264编码插件是MediaPipeX框架的视频编码插件，基于硬件H.264编码器实现。该插件负责将原始视频帧（如YUV格式）编码为H.264压缩格式，为后续的存储或流媒体传输做准备。

## 主要功能

- **硬件H.264编码**：利用SoC内置的硬件编码器实现高效压缩
- **多格式输入支持**：支持NV12、NV21、YUV420P等输入格式
- **实时编码**：低延迟的实时视频编码能力
- **可配置参数**：支持码率、帧率、分辨率等参数配置
- **零拷贝处理**：基于DMA缓冲区的高效数据传输
- **Pull模式数据流**：主动从上游插件获取帧数据

## API接口

### 创建编码器

```c
#include "plugins/Encoder/encoder_plugin.h"

// 创建H.264编码器插件
Plugin* create_encoder_plugin(const char* name);

// 销毁编码器插件
void destroy_encoder_plugin(Plugin* plugin);

```

**参数说明**：
- `name`：插件实例名称（可为NULL）
> ⚠️ **注意**：`h264_input_format` **必须与上游插件输出格式完全一致**，否则编码将失败或产生错误图像。

## 编码参数配置

### 默认配置

当前插件使用以下默认编码参数：
- **码率**：2Mbps
- **输入格式**：YUV420P
- **编码标准**：H.264 Baseline Profile

### 支持的输入格式

| 插件支持格式 (`PIXEL_FORMAT_*`) | 对应底层编码器格式 (`VENC_PIXEL_*`) | 是否支持 | 说明                 |
| ------------------------- | -------------------------- | ---- | ------------------ |
| `PIXEL_FORMAT_NV12`       | `VENC_PIXEL_YUV420SP`      | ✅ 是  | 常见的YUV420半平面格式     |
| `PIXEL_FORMAT_NV21`       | `VENC_PIXEL_YVU420SP`      | ✅ 是  | Android常用格式，UV顺序不同 |
| `PIXEL_FORMAT_YUV420P`    | `VENC_PIXEL_YUV420P`       | ✅ 是  | 经典YUV 3平面格式        |
| `PIXEL_FORMAT_YV12`       | `VENC_PIXEL_YVU420P`       | ✅ 是  | 类似YUV420P，UV顺序相反   |
| `PIXEL_FORMAT_YUV422P`    | `VENC_PIXEL_YUV422P`       | ✅ 是  | 水平采样为4:2:2的平面格式    |
| `PIXEL_FORMAT_RGB565`     | 不支持映射                      | ❌ 否  | 暂未支持RGB565编码       |
| `PIXEL_FORMAT_RGB888`     | 不支持映射                      | ❌ 否  | 暂未直接支持RGB888       |
| `PIXEL_FORMAT_BGR888`     | 不支持映射                      | ❌ 否  | 暂未直接支持BGR888       |
| `PIXEL_FORMAT_RGBA8888`   | `VENC_PIXEL_RGBA`          | ✅ 是  | 支持带Alpha的RGBA格式    |
| `PIXEL_FORMAT_ARGB8888`   | `VENC_PIXEL_ARGB`          | ✅ 是  | Alpha在高位           |
| `PIXEL_FORMAT_ABGR8888`   | `VENC_PIXEL_ABGR`          | ✅ 是  | ABGR排列             |

## 使用示例

### 基本视频编码

```c
#include "plugins/Encoder/encoder_plugin.h"

int main() {
    // 1. 创建编码器插件
    Plugin* encoder = create_encoder_plugin("H264Encoder");
    if (!encoder) {
        printf("创建编码器插件失败\n");
        return -1;
    }

    encConfig encoder_config = {0};
    encoder_config.h264_input_width = 640;
    encoder_config.h264_input_height = 480;
    encoder_config.h264_input_fps = 30;
    encoder_config.h264_bitrate = 2 * 1024 * 1024;
    //h264_input_format 必须与上游插件输出格式完全一致，否则编码将失败或产生错误图像。
    encoder_config.h264_input_format = PIXEL_FORMAT_YUV420P;
    if (encoder->init(encoder, &encoder_config) != 0) {
        LOG_ERROR("h264 encoder init fail");
    }


    // 2. 连接上游数据源（如g2d插件）,注意h264_input_format必须和上游插件输出图像一致
    Plugin* g2d = create_g2d_plugin("g2d");
    g2d_set_output_format(g2d, PIXEL_FORMAT_YUV420P);
    g2d_set_output_size(g2d, 640, 480);


    Plugin* camera = create_v4l2_camera_plugin();
    V4L2CameraConfig cam_config = {cam_index, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    // 4. 初始化和启动
    camera->init(camera, &cam_config);

    plugin_link_many(camera, g2d, encoder, NULL); //一定要以NULL结尾

    plugin_chain_start(camera);

    // 4. 获取编码后的H.264数据
    for (int i = 0; i < 300; i++) {  // 编码10秒@30fps
        FrameBuffer* encoded_frame = encoder->pull(encoder);
        if (encoded_frame) {
            frame_get(encoded_frame);
            printf("编码帧 %d: 大小=%d字节, PTS=%lu\n",
                   i, encoded_frame->size, encoded_frame->pts);

            frame_put(encoded_frame);
        }
    }

    // 5. 停止编码
    plugin_chain_stop(camera);


    //6. 释放资源
    destroy_encoder_plugin(encoder);
    destroy_g2d_plugin(g2d);
    destroy_v4l2_camera_plugin(camera);

    return 0;
}
```

## 数据流处理

### Pull模式工作原理

```
编码器线程循环：
1. 从upstream->pull()获取原始帧
   ↓
2. 转换为VencInputBuffer格式
   ↓
3. 调用硬件编码器进行编码
   ↓
4. 创建包含H.264数据的FrameBuffer
   ↓
5. 推入输出队列供下游pull()获取
```

### 内存管理

```c
// ... 编码处理 ...
FrameBuffer* output = encoder->pull(encoder);    // 获取编码帧

// 正确的内存管理
frame_put(input);   // 释放输入帧引用
frame_put(output);  // 释放输出帧引用
```

### 编码输出格式

编码器输出的FrameBuffer具有以下特征：
- **format**：PIXEL_FORMAT_H264（自定义格式）
- **data**：指向H.264压缩数据
- **size**：压缩数据的字节数
- **pts**：保持与输入帧相同的时间戳
- **width/height**：编码分辨率

## 并发使用说明

编码插件支持多个实例并发运行（如输出不同码率的视频流），但每个实例需绑定独立的输入流。编码器内部线程不支持跨实例共享，需确保上下文隔离。

```c
// 创建多个编码器实例
Plugin* encoder1 = create_encoder_plugin("encoder1");
Plugin* encoder2 = create_encoder_plugin("encoder2");
```

### 调试方法
```c
// 检查编码器状态
if (!encoder->pull(encoder)) {
    printf("编码器无输出，检查输入数据\n");
}
```

## 性能优化


### 系统优化

```c
// 优化编码性能的系统配置
// 1. 提高编码器线程优先级
// 2. 确保足够的DMA内存
// 3. 监控CPU和内存使用率
// 4. 合理配置编码参数
```

## 兼容性

### 硬件要求

- **H.264编码器**：SoC内置硬件编码单元
- **内存要求**：足够的DMA连续内存
- **驱动支持**：相应的硬件编码驱动

### 平台支持

- **Allwinner MR536**：完全支持

## 故障排除

### 检查清单

1. 确认H.264硬件编码器正常工作
2. 检查输入数据格式是否支持
3. 验证DMA内存充足
4. 确认编码参数在合理范围内
5. 检查upstream连接是否正确

