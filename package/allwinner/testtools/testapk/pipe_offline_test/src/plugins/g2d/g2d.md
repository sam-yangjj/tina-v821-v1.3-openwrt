# G2D 图形处理插件文档

## 概述

G2D插件是MediaPipeX框架的2D图形处理插件，基于硬件G2D（Graphics 2D）加速单元实现。该插件提供高性能的图像缩放、旋转、格式转换和裁剪功能，广泛用于视频预处理和后处理场景。

## 主要功能

- **图像缩放**：支持任意比例的图像缩放，包括放大和缩小（会自动进行16位对齐）
- **图像旋转**：支持0°、90°、180°、270°四个角度的旋转
- **格式转换**：支持多种像素格式之间的转换
- **图像裁剪**：支持任意矩形区域的裁剪
- **硬件加速**：充分利用G2D硬件加速单元，性能优异
- **零拷贝操作**：基于DMA缓冲区的零拷贝处理

> ⚠️ 注意：G2D 插件一次只能执行一种类型的处理（缩放 / 裁剪 / 旋转），只有缩放和图像格式转换可以同时进行，其他功能如需组合使用，请创建多个 G2D 实例，按顺序链接处理。


## 配置参数详解

### 支持的像素格式

| 枚举值                     | 类型          | 说明                 |
| ----------------------- | ----------- | ------------------ |
| `PIXEL_FORMAT_NV12`     | YUV（半平面）    | 常见的摄像头输出格式         |
| `PIXEL_FORMAT_NV21`     | YUV（半平面）    | 与 NV12 通道顺序不同      |
| `PIXEL_FORMAT_YUV420P`  | YUV（平面）     | 三平面格式，通用           |
| `PIXEL_FORMAT_YV12`     | YUV（平面）     | 类似 YUV420P，UV 顺序相反 |
| `PIXEL_FORMAT_YUV422P`  | YUV（平面）     | 水平 4:2:2 采样，色彩更丰富  |
| `PIXEL_FORMAT_RGB565`   | RGB         | 占用内存小，16位色         |
| `PIXEL_FORMAT_RGB888`   | RGB         | 标准24位 RGB 格式       |
| `PIXEL_FORMAT_BGR888`   | RGB         | 通道顺序为 BGR          |
| `PIXEL_FORMAT_RGBA8888` | RGB + Alpha | 包含透明度通道            |
| `PIXEL_FORMAT_ARGB8888` | RGB + Alpha | Alpha 通道在最高位       |
| `PIXEL_FORMAT_ABGR8888` | RGB + Alpha | ABGR 排列格式          |

> ⚠️ 注意：需要确保G2D上游插件的像素格式是上述所支持的格式

### 旋转支持

| 角度 | 支持 | 性能影响 |
|------|------|----------|
| 0° | ✓ | 无 |
| 90° | ✓ | 最小 |
| 180° | ✓ | 最小 |
| 270° | ✓ | 最小 |
| 其他角度 | ✗ | - |


## API接口

### 创建和配置

```c
#include "plugins/g2d/g2d_plugin.h"

// 创建G2D插件实例
Plugin* g2d_create(const char* name);

// 设置输出格式
int g2d_set_output_format(Plugin* self, PixelFormat fmt);

// 设置输出尺寸
int g2d_set_output_size(Plugin* self, int width, int height);

// 设置裁剪区域
int g2d_set_crop_region(Plugin* self, int crop_x, int crop_y,
                        int crop_width, int crop_height);

// 设置旋转角度
int g2d_set_rotation(Plugin* self, int angle);

// 销毁G2D插件
void destroy_g2d_plugin(Plugin* plugin);

```

## 使用示例

### 基本图像缩放

```c
#include "plugins/g2d/g2d_plugin.h"

int main() {
    // 1. 创建G2D插件
    Plugin* g2d = g2d_create("G2D");
    if (!g2d) {
        printf("创建G2D插件失败\n");
        return -1;
    }

    // 2. 配置输出参数(图像缩放和图像格式转换可以同时进行)
    g2d_set_output_size(g2d, 640, 640);          // 输出640x640
    g2d_set_output_format(g2d, PIXEL_FORMAT_RGB888); // RGB888格式

    //注意图像旋转和图像缩放、图像格式转换不能同时进行
    //g2d_set_rotation(g2d, 90);

    // 3. 连接上游数据源（如相机）
    Plugin* camera = create_v4l2_camera_plugin();
    V4L2CameraConfig cam_config = {cam_index, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    // 4. 初始化和启动（需要确保G2D上游插件的像素格式是g2d所支持的格式）
    camera->init(camera, &cam_config);
    plugin_link_many(camera, g2d, NULL); //一定要以NULL结尾

    //启动链路 参数必须是链路的第一个插件
    plugin_chain_start(camera);

    // 5. 处理图像(如果没有下游插件)
    for (int i = 0; i < 100; i++) {
        FrameBuffer* frame = g2d->pull(g2d);
        if (frame) {
            printf("处理帧 %d: %dx%d → 640x640\n",
                   i, frame->width, frame->height);
            frame_put(frame);
        }
    }

    // 6. 停止处理
    plugin_chain_stop(camera);

    //7. 释放资源（从后往前释放）
    destroy_g2d_plugin(g2d);
    destroy_v4l2_camera_plugin(camera);

    return 0;
}
```

### 图像裁剪

```c
// 裁剪图像中心区域并旋转90度
Plugin* g2d = g2d_create("G2D");

// 从原图(1920x1080)中裁剪中心800x600区域
g2d_set_crop_region(g2d, 560, 240, 800, 600);

```

### 格式转换预处理

```c
// 为AI推理准备RGB格式的方形图像
Plugin* camera = create_v4l2_camera_plugin();
Plugin* g2d = g2d_create("G2D");
Plugin* yolo = create_yolov5_plugin("YOLO", 2, "/path/to/model.nbg");

// 配置G2D：NV12 → RGB888，1920x1080 → 640x640
g2d_set_output_format(g2d, PIXEL_FORMAT_RGB888);
g2d_set_output_size(g2d, 640, 640);

```


## 高级用法

### 链式处理

```c
// 多级图像处理
Plugin* v4l2src = create_v4l2_camera_plugin();  // 输入格式
cam->init(v4l2src,cam_config);
Plugin* g2d1 = g2d_create("Crop");      // 第一级：裁剪
Plugin* g2d2 = g2d_create("Scale");     // 第二级：缩放
Plugin* g2d3 = g2d_create("Rotate");    // 第三级：旋转

// 配置各级处理参数(这三种图像处理不能使用同一个g2d插件 必须分开！！！)
g2d_set_crop_region(g2d1, 100, 100, 800, 600);
g2d_set_output_size(g2d2, 640, 480);
g2d_set_rotation(g2d3, 90);

// 连接处理链
plugin_link_many(v4l2src, g2d1, g2d2, g2d3, NULL); //注意，最后一个参数必须为NULL

//如果没有pipeline的结尾不是sink，则可以直接pull数据出来
FrameBuffer* fb = g2d3->pull(g2d3); //注意拉取数据的插件必须是pipeline的最后一个插件
frame_get(fb); //拿到数据后必须调用

frame_put(fb); //使用完成后必须释放
```


## 性能优化建议

1. **避免频繁参数修改**：每次修改参数都会重新配置硬件
2. **合理选择格式**：使用硬件原生支持的格式组合
3. **批量处理**：避免单帧处理，使用流水线模式
4. **内存对齐**：输入的图像分辨率会自动按16字节对齐

## 兼容性说明

### 硬件要求

- **G2D硬件单元**：必须存在G2D硬件加速单元（存在 /dev/g2d 节点）
- **驱动支持**：需要相应的G2D驱动程序
- **内存要求**：足够的DMA连续内存

### 平台支持

- **Allwinner MR536**：完全支持


## 故障排除

### 检查清单

1. 确认G2D硬件正常工作（`ls /dev/g2d`）
2. 检查驱动是否正确加载（`lsmod | grep g2d`）
3. 验证DMA内存充足
4. 确认输入参数在支持范围内

### 测试命令

```bash
# 检查G2D设备
ls -l /dev/g2d

# 查看G2D驱动状态
cat /proc/interrupts | grep g2d
```
