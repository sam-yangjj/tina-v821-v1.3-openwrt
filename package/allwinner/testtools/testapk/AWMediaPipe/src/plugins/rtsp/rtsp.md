# RTSP 流媒体插件文档

## 概述

RTSP插件是MediaPipeX框架的流媒体输出插件，基于标准RTSP/RTP协议实现。该插件作为视频处理管道的终端(sink)节点，将H.264编码的视频流通过网络提供给客户端，支持多客户端并发访问和低延迟直播。

## 主要功能

- **标准RTSP服务**：完全兼容RTSP 1.0协议规范
- **RTP传输**：支持RTP over UDP的实时传输
- **多客户端支持**：同时支持多个客户端连接
- **H.264流传输**：优化的H.264视频流传输

## API接口

### 创建和配置

```c
#include "plugins/rtsp/rtsp_plugin.h"

// 创建RTSP服务器插件
Plugin* create_rtsp_plugin(const char* name, const char* ip, int port, int fps);

// 设置服务器地址
int rtsp_set_server_address(Plugin* self, const char* ip, int port);
```

**参数说明**：
- `name`：插件实例名称（可为NULL）
- `ip`：服务器绑定IP地址（"0.0.0.0"表示所有接口）
- `port`：RTSP服务端口（标准端口554）
- `fps`：RTSP推流帧率，需要尽量和实际fps一致，否则会导致拉流端高延迟和卡顿

**注意**：拉流端和板端必须处于同一个局域网内，确保可以互相ping通！

## 使用示例

### 基本RTSP服务

```c
#include "plugins/rtsp/rtsp_plugin.h"

int main() {
    // 1. 创建RTSP服务器插件
    Plugin* rtsp = create_rtsp_plugin("RTSP", "0.0.0.0", 8554, 30);
    if (!rtsp) {
        printf("创建RTSP插件失败\n");
        return -1;
    }

    // 2. 连接H.264编码器作为数据源
    Plugin* encoder = create_encoder_plugin("H264");
    rtsp->upstream = encoder;

    // 3. 初始化和启动
    encoder->init(encoder, enconfig);
    rtsp->init(rtsp, NULL);

    encoder->start(encoder);
    rtsp->start(rtsp);

    printf("RTSP服务器启动成功！\n");
    printf("客户端可通过以下地址观看：\n");
    printf("rtsp://localhost:554/live\n");

    // 4. 服务器持续运行
    while (1) {
        sleep(1);
        // 检查服务器状态...
    }

    // 5. 停止服务
    rtsp->stop(rtsp);
    encoder->stop(encoder);

    return 0;
}
```


## 协议支持

### RTSP协议

- **版本**：RTSP 1.0
- **方法**：DESCRIBE, SETUP, PLAY, TEARDOWN
- **传输**：RTP over UDP
- **会话管理**：完整的会话生命周期管理

### RTP传输

- **载荷类型**：H.264 (Payload Type 96)
- **打包模式**：Single NAL Unit Mode
- **传输方式**：UDP单播
- **端口范围**：动态分配（通常从5004开始）

### SDP描述

```
v=0
o=- 0 0 IN IP4 192.168.1.100
s=MediaPipeX Live Stream
c=IN IP4 192.168.1.100
t=0 0
m=video 5004 RTP/AVP 96
a=rtpmap:96 H264/90000
a=fmtp:96 profile-level-id=42001e; sprop-parameter-sets=...
```

## 客户端兼容性


### 客户端连接示例

```bash
# 暂时只支持FFmpeg
# FFplay播放器
ffplay -i rtsp://192.168.1.100:554/live

# FFmpeg录制
ffmpeg -i rtsp://192.168.1.100:554/live -c copy output.mp4

```

**注意**：目前的rtsp视频流需要使用ffmpeg进行拉流播放，暂时不支持vlc播放，播放命令示例：`ffplay -i rtsp://127.0.0.1：8554`


## 错误处理

### 常见问题

**端口占用**
```bash
# 检查端口是否被占用
sudo netstat -tlnp | grep :8554
# 终止占用端口的进程
sudo kill -9 <PID>
```


## 故障排除

### 检查清单

1. 确认RTSP服务端口未被占用
2. 检查网络连接和防火墙设置
3. 验证编码器输出H.264流正常
4. 确认客户端支持RTSP协议
5. 检查系统资源是否充足
