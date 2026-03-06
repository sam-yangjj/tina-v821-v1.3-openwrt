#include "rtsp_session.h"
#include <pthread.h>

RtspSession* RtspSession_create(int client_fd, const char* client_ip, int client_port) {
    RtspSession* session = (RtspSession*)malloc(sizeof(RtspSession));
    if (!session) return NULL;
    session->client_fd = client_fd;
    strncpy(session->client_ip, client_ip, sizeof(session->client_ip)-1);
    session->client_ip[sizeof(session->client_ip)-1] = '\0';
    session->client_port = client_port;
    session->client_rtp_port = 0;
    session->state = 0;
    return session;
}
void dump_hex(const uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    if (len % 16 != 0)
        printf("\n");
}
#if 0
uint8_t* merge_h264_data(VencOutputBuffer* venc_buf, int* total_size) {
    int size0 = venc_buf->nSize0;
    int size1 = venc_buf->nSize1;

    *total_size = size0 + size1;

    if (*total_size == 0) {
        return NULL;
    }

    uint8_t* merged_data = (uint8_t*)malloc(*total_size);
    if (!merged_data) {
        perror("malloc for merged h264 data failed");
        return NULL;
    }

    // 拷贝数据部分
    if (size0 > 0 && venc_buf->pData0)
        memcpy(merged_data, venc_buf->pData0, size0);

    if (size1 > 0 && venc_buf->pData1)
        memcpy(merged_data + size0, venc_buf->pData1, size1);

    return merged_data;  // 使用完需要 free()
}
#endif
#if 0
void* rtp_streaming_thread(void* arg) {
    RtspSession* session = (RtspSession*)arg;

    RtpSender* sender = RtpSender_create(session->client_ip, session->client_rtp_port);
    if (!sender) {
        printf("[RTSP] Failed to create RTP sender\n");
        return NULL;
    }
    printf("[RTSP] Start streaming to %s:%d\n", session->client_ip, session->client_rtp_port);

    EncodedFrame frame;
    int frame_size;
    int flag = 1;
    VencHeaderData sps_pps_data;

    while (session->state == 2) {
        if (pop_encoded_frame_no_copy(session->frame_buffer, &frame, 1000) == 0) {
            frame_size = frame.venc_buf->nSize0 + frame.venc_buf->nSize1;
            printf("[RTSP] Read H264 frame size = %d\n", frame_size);

            printf("[RTSP] pts = %d ===================\n",frame.venc_buf->nPts);

            if (flag) {
                // 首次发送 SPS/PPS
                VideoEncGetParameter(frame.encoder, VENC_IndexParamH264SPSPPS, &sps_pps_data);
                RtpSender_send_frame(sender, sps_pps_data.pBuffer, sps_pps_data.nLength);
                flag = 0;
            }

            // 可选：关键帧前发送 SPS/PPS
            if (frame.venc_buf->nFlag) {
                RtpSender_send_frame(sender, sps_pps_data.pBuffer, sps_pps_data.nLength);
            }

            if (frame.venc_buf->nSize1 > 0) {
                uint8_t* merged_frame = merge_h264_data(&frame.venc_buf, frame_size);
                RtpSender_send_frame(sender, merged_frame, frame_size);
            }else
                RtpSender_send_frame(sender, frame.venc_buf->pData0, frame.venc_buf->nSize0);

            frame_unref(&frame);
            if(frame.sink_refcount == 0)
            {
                FreeOneBitStreamFrame(frame.encoder, frame.venc_buf);
                buffer_pool_release(session->h264OutBufPool, frame.bufIndex);
            }
            // printf("=================== FreeOneBitStreamFrame ID = %d ================\n",frame.venc_buf->nID);
        } else {
            printf("[RTSP] Failed to get frame\n");
            usleep(1000);
        }
    }

    printf("[RTSP] Streaming thread exit\n");
    RtpSender_destroy(sender);
    return NULL;
}
#endif

#if 0
void RtspSession_handle(RtspSession* session) {
    char buf[2048];
    char method[32], url[128], version[32];
    int cseq = 0;
    int64_t expected_frame_id = 0;
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
            const char* sdp =
                "v=0\r\n"
                "o=- 0 0 IN IP4 127.0.0.1\r\n"
                "s=No Name\r\n"
                "t=0 0\r\n"
                "a=tool:libavformat 58.29.100\r\n"
                "m=video 0 RTP/AVP 96\r\n"
                "c=IN IP4 0.0.0.0\r\n"
                "a=rtpmap:96 H264/90000\r\n"
                "a=control:trackID=0\r\n";
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

            #if 0
            uint8_t frame[500000];
            int frame_size;
            while ((frame_size = H264Reader_get_frame(reader, frame, sizeof(frame))) > 0) {
                printf("[RTSP] Read H264 frame, size=%d\n", frame_size);
                RtpSender_send_frame(sender, frame, frame_size);
                usleep(40000); // 25fps
            }
            #endif


            pthread_t rtp_thread;
            if (pthread_create(&rtp_thread, NULL, rtp_streaming_thread, session) != 0) {
                perror("Failed to create RTP streaming thread");
            } else {
                printf("RTP streaming thread started\n");
            }
            pthread_join(rtp_thread, NULL);

            #if 1
            EncodedFrame frame;
            int64_t pts;

            int frame_size;

            VencHeaderData sps_pps_data;
            int flag = 1;
            while (1) {
                if (pop_encoded_frame_no_copy(session->frame_buffer, &frame, 1000) == 0) {
                    frame_size = frame.venc_buf->nSize0 + frame.venc_buf->nSize1;
                    printf("[RTSP] Read H264 frame size = %d\n",frame_size);

                    if(flag)
                    {
                        VideoEncGetParameter(frame.encoder, VENC_IndexParamH264SPSPPS, &sps_pps_data);
                        RtpSender_send_frame(sender,sps_pps_data.pBuffer,sps_pps_data.nLength);
                        flag = 0;
                    }
                    // 关键帧前先推 SPS/PPS
                    if (frame.venc_buf->nFlag) {
                        //RtpSender_send_frame(sender,sps_pps_data.pBuffer,sps_pps_data.nLength);
                    }
                    RtpSender_send_frame(sender, frame.venc_buf->pData0, frame.venc_buf->nSize0);
                    if (frame.venc_buf->nSize1)
                        printf("pData1, size = %u\n", frame.venc_buf->nSize1);
                        dump_hex(frame.venc_buf->pData1, frame.venc_buf->nSize1);
                        RtpSender_send_frame(sender, frame.venc_buf->pData1, frame.venc_buf->nSize1);

                    FreeOneBitStreamFrame(frame.encoder, &frame.venc_buf);
                } else {
                    printf("[RTSP] Read H264 frame failed\n");
                }
            }

            #endif
            RtpSender_destroy(sender);

           // H264Reader_destroy(reader);
            break;
        }
    }
}
#endif
void RtspSession_destroy(RtspSession* session) {
    if (!session) return;
    if (session->client_fd > 0) close(session->client_fd);
    free(session);
}
