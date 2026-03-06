# GDC 几何畸变校正插件文档

## 概述

GDC（Geometry Distortion Correction）插件是MediaPipeX框架的几何畸变校正插件，基于硬件GDC单元实现。该插件主要用于校正摄像头镜头造成的几何畸变，包括桶形畸变、枕形畸变以及透视变换等，为后续的图像处理提供更加准确的视觉数据。

> ⚠️ 使用本插件前，请确保将与输入分辨率匹配的 LUT 文件放置于 `/mnt/UDISK` 目录下。该文件为必需资源，用于驱动硬件进行正确的几何校正。


## 主要功能

- **镜头畸变校正**：校正广角镜头的桶形畸变和枕形畸变
- **透视变换**：实现图像的透视校正和几何变换
- **硬件加速**：利用专用GDC硬件单元实现高效处理
- **实时处理**：支持实时视频流的畸变校正
- **多种校正模式**：支持不同类型的几何变换算法
- **零拷贝处理**：基于DMA缓冲区的高效数据传输

## API接口

### 创建和配置

在初始化插件之前，用户需要根据输入分辨率准备好 LUT（查找表）文件，并将其放置到 `/mnt/UDISK` 目录下。GDC 插件将在初始化时自动加载该文件用于畸变校正。

例如，若输入分辨率为 1280x720，则必须提供与该分辨率对应的 `test_x.txt`,`test_y.txt` 文件。文件生成方式请查看gdc的对应文档

#### GDC配置模式说明

`GDCConfig` 结构中的 `mode` 字段用于指定几何校正类型，实际会映射到 GDC 硬件内部对应的处理模式。以下是支持的用户模式与其对应的内部 Warp 类型：
| `UserWarpMode` 枚举值       | 说明          | 内部映射 (`eWarpType`) |
| ------------------------ | ----------- | ------------------ |
| `USER_WARP_DEFAULT`      | 默认LDC畸变校正   | `Warp_LDC`         |
| `USER_WARP_WIDE`         | 鱼眼转广角       | `Warp_Fish2Wide`   |
| `USER_WARP_ULTRA_WIDE`   | 超广角矫正       | `Warp_LDC_Pro`     |
| `USER_WARP_PANORAMA_180` | 180度全景模式    | `Warp_Pano180`     |
| `USER_WARP_PANORAMA_360` | 360度全景模式    | `Warp_Pano360`     |
| `USER_WARP_TOP_DOWN`     | 正上俯视图       | `Warp_Normal`      |
| `USER_WARP_PERSPECTIVE`  | 透视矫正模式      | `Warp_Perspective` |
| `USER_WARP_BIRDSEYE`     | 鸟瞰图模式       | `Warp_BirdsEye`    |
| `USER_WARP_CUSTOM`       | 使用自定义LUT文件  | `Warp_User`        |


```c
#include "plugins/gdc/gdc_plugin.h"

// 创建GDC插件实例
Plugin* create_gdc_plugin(const char* name);

// 销毁GDC插件（外部调用）
void destroy_gdc_plugin(Plugin* plugin);
```

## 使用示例

### 基本畸变校正

```c
#include "plugins/gdc/gdc_plugin.h"

int main() {
    // 1. 创建GDC插件
    Plugin* gdc = create_gdc_plugin("LensCorrection");
    if (!gdc) {
        printf("创建GDC插件失败\n");
        return -1;
    }

    // 2. 初始化
    GDCConfig gdc_config = {USER_WARP_CUSTOM, CAM_WIDTH, CAM_HEIGHT, CAM_WIDTH, CAM_HEIGHT};
    gdc->init(gdc, &gdc_config);

    // 3. 连接上游数据源（如相机）
    Plugin* camera = create_v4l2_camera_plugin();
    V4L2CameraConfig cam_config = {cam_index, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    camera->init(camera, &cam_config);
    // 链接上游插件
    plugin_link_many(camera, g2d, NULL);

    // 4.启动
    plugin_chain_start(camera);

    // 5. 获取校正后的图像
    for (int i = 0; i < 100; i++) {
        FrameBuffer* corrected_frame = gdc->pull(gdc);
        if (corrected_frame) {
            printf("校正帧 %d: %dx%d\n",
                   i, corrected_frame->width, corrected_frame->height);

            // 处理校正后的图像
            // process_corrected_image(corrected_frame);

            frame_put(corrected_frame);
        }
    }

    // 6. 停止处理
    plugin_chain_stop(camera); //传入参数是pipeline的source

    // 7. 销毁插件
    destroy_gdc_plugin(gdc);

    return 0;
}
```

### 多角度监控校正

```c
    // 为多个广角摄像头分别进行畸变校正
    Plugin* camera1 = create_v4l2_camera_plugin();  // 前置摄像头
    Plugin* camera2 = create_v4l2_camera_plugin();  // 后置摄像头

    V4L2CameraConfig cam_config1 = {cam_index1, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    camera1->init(camera1, &cam_config1);

    V4L2CameraConfig cam_config2 = {cam_index2, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    camera2->init(camera2, &cam_config2);

    Plugin* gdc1 = create_gdc_plugin("FrontCorrection");
    Plugin* gdc2 = create_gdc_plugin("RearCorrection");

    // 配置不同的校正参数
    GDCConfig gdc_config1 = {USER_WARP_CUSTOM, CAM_WIDTH, CAM_HEIGHT, CAM_WIDTH, CAM_HEIGHT};
    gdc1->init(gdc1, &gdc_config1);
    GDCConfig gdc_config2 = {USER_WARP_CUSTOM, CAM_WIDTH, CAM_HEIGHT, CAM_WIDTH, CAM_HEIGHT};
    gdc2->init(gdc2, &gdc_config2);

    // 连接各自的数据流
    plugin_link_many(camera1, gdc1, NULL);
    plugin_link_many(camera2, gdc2, NULL);

    plugin_chain_start(camera1);
    plugin_chain_start(camera2);

    // 后续可以将校正后的图像合成全景视图
    while (1) {
            FrameBuffer* fb1 = gdc1->pull(gdc1);
            FrameBuffer* fb2 = gdc2->pull(gdc2);

            if (!fb1 || !fb2) {
                printf("Failed to pull frame\n");
                continue;
            }

            frame_get(fb1);
            frame_get(fb2);

            int out_width, out_height;
            uint8_t* merged = merge_frames(fb1, fb2); //用户需自行实现 merge_frames() 以适配特定图像融合逻辑

            frame_put(fb1);
            frame_put(fb2);

        }
    plugin_chain_stop(camera1);
    plugin_chain_stop(camera2);
    destroy_gdc_plugin(gdc1);
    destroy_gdc_plugin(gdc2);
    destroy_v4l2_camera_plugin(camera1);
    destroy_v4l2_camera_plugin(camera2);
```

## 数据流处理

### Pull模式工作原理

```
GDC处理线程循环：
1. 从upstream->pull()获取原始图像
   ↓
2. 配置GDC输入缓冲区
   ↓
3. 调用硬件GDC进行畸变校正
   ↓
4. 创建包含校正后图像的FrameBuffer
   ↓
5. 推入输出队列供下游pull()获取
```

### 处理优化

1. **输入格式**：使用硬件原生支持的格式
2. **分辨率**：根据应用需求选择合适的输出分辨率
3. **校正模式**：选择适合的校正算法模式
4. **内存对齐**：确保数据按硬件要求对齐

### 系统优化

```c
 1. 确保足够的DMA内存
 2. 合理设置处理线程优先级
 3. 监控内存使用情况
```

## 标定和配置


### 硬件要求

- **GDC硬件单元**：专用的几何畸变校正硬件
- **内存要求**：足够的DMA连续内存
- **驱动支持**：相应的GDC硬件驱动

### 平台支持

- **Allwinner MR536**：完全支持
- **其他平台**：需要确认GDC硬件可用性

## 故障排除

### 检查清单

1. 确认GDC硬件和驱动正常工作
2. 检查输入图像格式是否支持
3. 验证校正参数是否正确
4. 确认DMA内存充足
5. 检查upstream连接是否正确
