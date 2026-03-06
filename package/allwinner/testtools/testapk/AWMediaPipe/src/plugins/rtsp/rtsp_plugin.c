/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "rtsp_plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../frame_queue.h"
#include "../utils/log.h"
#include "rtspsink/rtsp_server.h"

// RTSP插件私有数据结构
typedef struct {
    RtspServer* rtsp_server;
    RtspSession* session;
    RtpSender* sender;
    int fps;
    pthread_t server_thread;
    pthread_t data_thread;
    int running;

    // 服务器配置
    char server_ip[64];
    int server_port;

} RtspPluginPriv;


static int parse_request_line(const char* line, char* method, char* url, char* version) {
    return sscanf(line, "%s %s %s", method, url, version);
}

static int parse_cseq(const char* line, int* cseq) {
    return sscanf(line, "CSeq: %d", cseq);
}

// static int parse_transport(const char* line, int* rtp_port) {
//     // 查找 client_port=xxxx-xxxx
//     const char* p = strstr(line, "client_port=");
//     if (p) {
//         return sscanf(p, "client_port=%d", rtp_port);
//     }
//     return 0;
// }

static void parse_transport(const char* line, int* rtp_port, int* rtcp_port) {
    *rtp_port = 0;
    *rtcp_port = 0;
    const char* p = strstr(line, "client_port=");
    if (p) {
        p += strlen("client_port=");
        sscanf(p, "%d-%d", rtp_port, rtcp_port);
    }
}



void update_fps_rtsp() {
    static int frame_count = 0;
    static time_t last_time = 0;

    frame_count++;

    time_t current_time = time(NULL);
    if (current_time != last_time) {
        LOG_INFO("FPS: %d\n", frame_count);
        frame_count = 0;
        last_time = current_time;
    }
}

// 数据处理线程，从上游拉取编码数据并推入H264FrameBuffer
static void* rtsp_data_thread(void* arg) {
    Plugin* self = (Plugin*)arg;
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;

    RtpSender* sender = RtpSender_create(priv->session->client_ip, priv->session->client_rtp_port);
    if (!sender) {
        LOG_ERROR("Failed to create RTP sender");
        return NULL;
    }
    int isFirst = 1;

    while (priv->running) {
        // 检查是否有上游插件
        if (!self->upstream || !self->upstream->pull) {
            usleep(10 * 1000);
            continue;
        }

        FrameBuffer* frame = self->upstream->pull(self->upstream);
        if (!frame) {
            usleep(1000);
            continue;
        }
        frame_get(frame);

        LOG_INFO("RTSP received encoded frame: size=%d, pts=%lu, index=%d\n", frame->size, frame->pts, frame->buf_index);

        // if(((isFirst && frame->format == PIXEL_FORMAT_H264) || frame->nFlag) && frame->sps_pps_data)
        // {
        //     RtpSender_send_frame(sender,frame->sps_pps_data,frame->sps_pps_data_len, 30);
        //     isFirst = 0;
        // }
        if((isFirst && frame->format == PIXEL_FORMAT_H264)  && frame->sps_pps_data)
        {
            RtpSender_send_frame(sender,frame->sps_pps_data,frame->sps_pps_data_len, priv->fps);
            isFirst = 0;
        }
        update_fps_rtsp();
        RtpSender_send_frame(sender,frame->data,frame->size, 25);
        LOG_INFO("Frame pushed to RTSP buffer\n");

        // 释放帧
        frame_put(frame);
    }

    return NULL;
}

#if 0
void RtspSession_handle(Plugin* self) {
    char buf[2048];
    char method[32], url[128], version[32];
    int cseq = 0;
    int64_t expected_frame_id = 0;

    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;
    RtspSession* session = priv->session;
    while (1) {
        ssize_t n = recv(session->client_fd, buf, sizeof(buf)-1, 0);
        if (n <= 0) {
            printf("[RTSP] recv() <= 0, break\n");
            break;
        }
        buf[n] = '\0';
        printf("[RTSP] Received request:\n%s\n", buf);
        // 只简单处理第一行和CSeq、Transport
        char* line = strtok(buf, "\r\n");
        method[0] = url[0] = version[0] = '\0';
        cseq = 0;
        while (line) {
            if (strstr(line, "OPTIONS") || strstr(line, "DESCRIBE") || strstr(line, "SETUP") || strstr(line, "PLAY")) {
                parse_request_line(line, method, url, version);
                printf("[RTSP] method line: %s\n", line);
            } else if (strstr(line, "CSeq")) {
                parse_cseq(line, &cseq);
                printf("[RTSP] CSeq line: %s\n", line);
            } else if (strstr(line, "Transport")) {
                parse_transport(line, &session->client_rtp_port);
                printf("[RTSP] Transport line: %s, parsed client_rtp_port=%d\n", line, session->client_rtp_port);
            }
            line = strtok(NULL, "\r\n");
        }
        printf("[RTSP] method=%s, cseq=%d, client_rtp_port=%d, state=%d\n", method, cseq, session->client_rtp_port, session->state);
        char response[2048];
        if (strcmp(method, "OPTIONS") == 0) {
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nPublic: OPTIONS, DESCRIBE, SETUP, PLAY\r\n\r\n", cseq);
            printf("[RTSP] Respond OPTIONS\n");
        } else if (strcmp(method, "DESCRIBE") == 0) {
            // 更标准的SDP内容
            char sdp[512];
            sprintf(sdp,"v=0\r\n"
                "o=- 0 0 IN IP4 %s\r\n"
                "s=No Name\r\n"
                "t=0 0\r\n"
                "a=tool:libavformat 58.29.100\r\n"
                "m=video 0 RTP/AVP 96\r\n"
                "c=IN IP4 %s\r\n"
                "a=rtpmap:96 H264/90000\r\n"
                "a=control:trackID=0\r\n",
                priv->session->client_ip,
                priv->session->client_ip
            );
            // const char* sdp =
            //     "v=0\r\n"
            //     "o=- 0 0 IN IP4 127.0.0.1\r\n"
            //     "s=No Name\r\n"
            //     "t=0 0\r\n"
            //     "a=tool:libavformat 58.29.100\r\n"
            //     "m=video 0 RTP/AVP 96\r\n"
            //     "c=IN IP4 0.0.0.0\r\n"
            //     "a=rtpmap:96 H264/90000\r\n"
            //     "a=control:trackID=0\r\n";
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nContent-Base: %s\r\nContent-type: application/sdp\r\nContent-length: %zu\r\n\r\n%s",
                cseq, url, strlen(sdp), sdp);
            printf("[RTSP] Respond DESCRIBE\n");
        } else if (strcmp(method, "SETUP") == 0) {
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d;server_port=55532-55533\r\nSession: 66334873\r\n\r\n",
                cseq, session->client_rtp_port, session->client_rtp_port+1);
            session->state = 1;
            printf("[RTSP] Respond SETUP, client_rtp_port=%d\n", session->client_rtp_port);
        } else if (strcmp(method, "PLAY") == 0) {
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nRange: npt=0.000-\r\nSession: 66334873; timeout=10\r\n\r\n", cseq);
            session->state = 2;
            printf("[RTSP] Respond PLAY, start streaming\n");
        } else {
            snprintf(response, sizeof(response), "RTSP/1.0 400 Bad Request\r\n\r\n");
            printf("[RTSP] Unknown method: %s\n", method);
        }
        send(session->client_fd, response, strlen(response), 0);
        if (session->state == 2) {
            #if 0
            printf("[RTSP] Enter streaming loop, open H264 file\n");
            H264Reader* reader = H264Reader_create("/tmp/output.h264");
            if (!reader) {
                printf("[RTSP] Failed to open H264 file ../data/test.h264\n");
                break;
            }
            #endif
            RtpSender* sender = RtpSender_create(session->client_ip, session->client_rtp_port);
            if (!sender) {
                printf("[RTSP] Failed to create RTP sender\n");
                //H264Reader_destroy(reader);
                break;
            }
            printf("[RTSP] Start streaming to %s:%d\n", session->client_ip, session->client_rtp_port);

            pthread_t rtp_thread;
            if (pthread_create(&rtp_thread, NULL, rtsp_data_thread, self) != 0) {
                perror("Failed to create RTP streaming thread");
            } else {
                printf("RTP streaming thread started\n");
            }
            pthread_join(rtp_thread, NULL);

            RtpSender_destroy(sender);

           // H264Reader_destroy(reader);
            break;
        }
    }
}
#endif

#if 1
// static void parse_transport(const char* line, int* rtp_port, int* rtcp_port) {
//     sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d", rtp_port, rtcp_port);
// }

void RtspSession_handle(Plugin* self) {
    char buf[2048];
    char method[32], url[128], version[32];
    int cseq = 0;

    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;
    RtspSession* session = priv->session;

    while (1) {
        ssize_t n = recv(session->client_fd, buf, sizeof(buf)-1, 0);
        if (n <= 0) {
            printf("[RTSP] recv() <= 0, break\n");
            break;
        }
        buf[n] = '\0';
        printf("[RTSP] Received request:\n%s\n", buf);

        // 解析请求
        char* line = strtok(buf, "\r\n");
        method[0] = url[0] = version[0] = '\0';
        cseq = 0;
        while (line) {
            if (strstr(line, "OPTIONS") || strstr(line, "DESCRIBE") || strstr(line, "SETUP") || strstr(line, "PLAY")) {
                parse_request_line(line, method, url, version);
            } else if (strstr(line, "CSeq")) {
                parse_cseq(line, &cseq);
            } else if (strstr(line, "Transport")) {
                parse_transport(line, &session->client_rtp_port, &session->client_rtcp_port);
                printf("[RTSP] Transport: client RTP=%d, RTCP=%d\n", session->client_rtp_port, session->client_rtcp_port);
            }
            line = strtok(NULL, "\r\n");
        }

        char response[2048];
        if (strcmp(method, "OPTIONS") == 0) {
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nPublic: OPTIONS, DESCRIBE, SETUP, PLAY\r\n\r\n", cseq);
        } else if (strcmp(method, "DESCRIBE") == 0) {
            // 注意：替换 c= 行中的 IP 地址为服务器实际地址
        char sdp[1024];
        snprintf(sdp, sizeof(sdp),
            "v=0\r\n"
            "o=- 0 0 IN IP4 %s\r\n"
            "s=No Name\r\n"
            "t=0 0\r\n"
            "a=tool:MyRTSPServer\r\n"
            "m=video 0 RTP/AVP 96\r\n"
            "c=IN IP4 %s\r\n"
            "a=rtpmap:96 H264/90000\r\n"
            "a=control:trackID=0\r\n",
            priv->session->client_ip,
            priv->session->client_ip   // SDP中的c=行
        );


            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nContent-Base: %s\r\nContent-type: application/sdp\r\nContent-length: %zu\r\n\r\n%s",
                cseq, url, strlen(sdp), sdp);
        } else if (strcmp(method, "SETUP") == 0) {
            session->state = 1;
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
                "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=55532-55533\r\n"
                "Session: 66334873\r\n\r\n",
                cseq, session->client_rtp_port, session->client_rtcp_port);
        } else if (strcmp(method, "PLAY") == 0) {
            session->state = 2;
            snprintf(response, sizeof(response),
                "RTSP/1.0 200 OK\r\nCSeq: %d\r\nRange: npt=0.000-\r\nSession: 66334873; timeout=10\r\n\r\n", cseq);

            // 启动 RTP 推流线程
            RtpSender* sender = RtpSender_create(session->client_ip, session->client_rtp_port);
            if (!sender) {
                printf("[RTSP] Failed to create RTP sender\n");
                break;
            }
            //priv->sender = sender; // 如果你想让 rtsp_data_thread 访问它

            pthread_t rtp_thread;
            if (pthread_create(&rtp_thread, NULL, rtsp_data_thread, self) != 0) {
                perror("Failed to create RTP streaming thread");
            } else {
                pthread_detach(rtp_thread); // 不阻塞主线程
                printf("[RTSP] RTP streaming thread started\n");
            }
        } else {
            snprintf(response, sizeof(response), "RTSP/1.0 400 Bad Request\r\n\r\n");
        }

        // 发送响应
        send(session->client_fd, response, strlen(response), 0);
    }
}

#endif
// RTSP服务器线程，处理客户端连接
static void* rtsp_server_thread(void* arg) {
    Plugin* self = (Plugin*)arg;
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;

    LOG_DEBUG("RTSP server starting on %s:%d\n", priv->server_ip, priv->server_port);
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(priv->rtsp_server->listen_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) {
            LOG_ERROR("accept");
            continue;
        }
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);
        LOG_DEBUG("[RTSP] Accept client: %s:%d\n", client_ip, client_port);
        RtspSession* session = RtspSession_create(client_fd, client_ip, client_port);
        priv->session = session;
        RtspSession_handle(self);
        RtspSession_destroy(session);
    }

    return NULL;
}

static int rtsp_plugin_init(Plugin* self, void* config) {
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;

    priv->running = 0;

    // 创建RTSP服务器
    priv->rtsp_server = RtspServer_create(priv->server_ip, priv->server_port);
    if (!priv->rtsp_server) {
        return -1;
    }

    printf("RTSP plugin initialized: %s:%d\n", priv->server_ip, priv->server_port);
    return 0;
}

static int rtsp_start(Plugin* self) {
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;
    priv->running = 1;

    // 启动数据处理线程
    // int ret1 = pthread_create(&priv->data_thread, NULL, rtsp_data_thread, self);

    // 启动RTSP服务器线程
    int ret = pthread_create(&priv->server_thread, NULL, rtsp_server_thread, self);

    return ret;
}

static int rtsp_stop(Plugin* self) {
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;
    priv->running = 0;

    // 等待线程结束
    pthread_join(priv->data_thread, NULL);
    pthread_join(priv->server_thread, NULL);

    return 0;
}

static int rtsp_process(Plugin* self, FrameBuffer* frame) {
    // 作为sink插件，通常不实现process，而是通过pull主动拉取
    return 0;
}

static int rtsp_push(Plugin* self, FrameBuffer* frame) {
    // sink插件不需要push
    return 0;
}

static FrameBuffer* rtsp_pull(Plugin* self) {
    // sink插件不输出数据
    return NULL;
}

static void rtsp_plugin_destroy(Plugin* self) {
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;

    if(priv->sender)
        RtpSender_destroy(priv->sender);
    if (priv->rtsp_server) {
        RtspServer_destroy(priv->rtsp_server);
    }
    if (priv->session)
    {
        RtspSession_destroy(priv->session);
    }



    free(priv);
}

// 设置服务器地址
int rtsp_set_server_address(Plugin* self, const char* ip, int port) {
    RtspPluginPriv* priv = (RtspPluginPriv*)self->priv;

    strncpy(priv->server_ip, ip, sizeof(priv->server_ip)-1);
    priv->server_ip[sizeof(priv->server_ip)-1] = '\0';
    priv->server_port = port;

    return 0;
}

Plugin* create_rtsp_plugin(const char* name, const char* ip, int port, int fps) {
    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    RtspPluginPriv* priv = (RtspPluginPriv*)calloc(1, sizeof(RtspPluginPriv));

    plugin->name = name ? strdup(name) : "RtspPlugin";
    plugin->init = rtsp_plugin_init;
    plugin->start = rtsp_start;
    plugin->stop = rtsp_stop;
    plugin->process = rtsp_process;
    plugin->push = rtsp_push;
    plugin->pull = rtsp_pull;
    plugin->priv = priv;

    // 初始化服务器配置
    if (ip) {
        strncpy(priv->server_ip, ip, sizeof(priv->server_ip)-1);
        priv->server_ip[sizeof(priv->server_ip)-1] = '\0';
    } else {
        strcpy(priv->server_ip, "0.0.0.0");
    }
    priv->server_port = port > 0 ? port : 554;
    priv->fps = fps > 0 ? fps : 30;

    return plugin;
}

// 独立的销毁函数，供外部调用
void destroy_rtsp_plugin(Plugin* plugin) {
    if (plugin && plugin->priv) {
        rtsp_plugin_destroy(plugin);
        if (plugin->name) {
            free(plugin->name);
            plugin->name = NULL;
        }
        free(plugin);
        plugin = NULL;
    }
}
