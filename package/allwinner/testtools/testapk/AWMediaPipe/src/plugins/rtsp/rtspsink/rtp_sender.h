#ifndef RTP_SENDER_H
#define RTP_SENDER_H

#include <stdint.h>

typedef struct RtpSender {
    int sockfd;
    char client_ip[64];
    int client_port;
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
} RtpSender;

RtpSender* RtpSender_create(const char* client_ip, int client_port);
int RtpSender_send_frame(RtpSender* sender, const uint8_t* frame, int size, int fps);
void RtpSender_destroy(RtpSender* sender);

#endif // RTP_SENDER_H
