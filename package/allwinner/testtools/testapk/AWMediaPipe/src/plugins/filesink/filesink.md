# FileSink 文件写入插件文档

## 概述

FileSink插件是MediaPipeX框架的文件输出插件，专门用于将H.264编码的视频流写入文件。该插件作为流水线的终端节点（Sink），从上游插件获取编码后的视频数据并保存到指定的文件中，支持实时写入和统计功能。

## 主要功能

- **H.264文件写入**：将编码后的H.264视频流保存为文件
- **实时写入**：支持实时视频流的连续文件写入
- **Pull模式数据流**：主动从上游插件获取编码数据
- **统计功能**：提供写入帧数和字节数的统计信息
- **线程安全**：内置线程安全的文件写入机制
- **文件管理**：自动处理文件创建、写入和关闭

## API接口

### 创建和配置

```c
#include "plugins/filesink/filesink_plugin.h"

// 创建文件写入插件
Plugin* create_filesink_plugin(const char* name, const char* filename);

// 设置输出文件名
int filesink_set_filename(Plugin* self, const char* filename);

// 销毁插件（外部调用）
void destroy_filesink_plugin(Plugin* plugin);
```

**参数说明**：
- `name`：插件实例名称（可为NULL）
- `filename`：输出文件路径

## 使用示例

### 基本文件保存

```c
#include "plugins/filesink/filesink_plugin.h"

int main() {
    // 1. 创建文件写入插件
    Plugin* filesink = create_filesink_plugin("H264Writer", "output.h264");
    if (!filesink) {
        printf("创建文件写入插件失败\n");
        return -1;
    }

    // 2. 连接编码器作为数据源
    Plugin* encoder = create_encoder_plugin("H264");
    filesink->upstream = encoder;

    // 3. 初始化和启动
    encoder->init(encoder, NULL);
    filesink->init(filesink, NULL);

    encoder->start(encoder);
    filesink->start(filesink);

    printf("开始录制视频到文件: output.h264\n");

    // 4. 录制一段时间
    sleep(60);  // 录制60秒

    // 5. 停止录制
    filesink->stop(filesink);
    encoder->stop(encoder);

    // 6. 销毁插件
    destroy_filesink_plugin(filesink);

    printf("录制完成！\n");
    return 0;
}
```


### 多文件并行录制

```c
// 同时录制多个不同质量的文件
Plugin* encoder_hd = create_encoder_plugin("HD");     // 高清编码器
Plugin* encoder_sd = create_encoder_plugin("SD");     // 标清编码器

Plugin* filesink_hd = create_filesink_plugin("HD_Recorder", "hd_video.h264");
Plugin* filesink_sd = create_filesink_plugin("SD_Recorder", "sd_video.h264");

// 连接不同的编码器
filesink_hd->upstream = encoder_hd;  // 保存高清流
filesink_sd->upstream = encoder_sd;  // 保存标清流

// 同时启动两个录制任务
filesink_hd->start(filesink_hd);
filesink_sd->start(filesink_sd);
```

## 数据流处理

### Pull模式工作原理

```
FileSink线程循环：
1. 从upstream->pull()获取编码帧
   ↓
2. 检查帧数据有效性
   ↓
3. 写入数据到文件 (fwrite)
   ↓
4. 刷新文件缓冲区 (fflush)
   ↓
5. 更新统计信息
   ↓
6. 释放帧引用 (frame_put)
```

### 内存管理

```c
// FileSink插件自动管理文件写入
FrameBuffer* frame = upstream->pull(upstream);    // 从上游获取编码帧
frame_get(frame);
// ... 写入文件 ...
frame_put(frame);  // 释放帧引用
```

### 文件格式
可以自己制定，以h264格式为例：
FileSink插件直接将接收到的H.264数据写入文件，生成的文件具有以下特征：
- **格式**：原始H.264字节流（Elementary Stream）
- **扩展名**：通常使用`.h264`或`.264`
- **播放**：可直接用VLC、ffplay等播放器播放
- **转换**：可用ffmpeg转换为MP4等容器格式

## 错误处理

### 常见问题

**1. 文件创建失败**
```c
if (filesink->init(filesink, NULL) != 0) {
    printf("检查文件路径权限和磁盘空间\n");
    // 检查目录是否存在
    // 检查写入权限
    // 检查磁盘剩余空间
}
```

## 故障排除

### 检查清单

1. 确认输出目录存在且有写入权限
2. 检查磁盘剩余空间是否充足
3. 验证上游编码器正常输出数据
4. 确认文件路径格式正确
