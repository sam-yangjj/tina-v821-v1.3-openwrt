#include "rtp_sender.h"
#include "rtp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../utils/log.h"

RtpSender* RtpSender_create(const char* client_ip, int client_port) {
    RtpSender* sender = (RtpSender*)malloc(sizeof(RtpSender));
    if (!sender) return NULL;
    sender->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender->sockfd < 0) {
        perror("socket");
        free(sender);
        return NULL;
    }
    strncpy(sender->client_ip, client_ip, sizeof(sender->client_ip)-1);
    sender->client_ip[sizeof(sender->client_ip)-1] = '\0';
    sender->client_port = client_port;
    sender->seq = 0;
    sender->timestamp = 0;
    sender->ssrc = 0x88923423;
    return sender;
}

#if 0
int RtpSender_send_frame(RtpSender* sender, const uint8_t* frame, int size) {
    printf("[RTP] Send to %s:%d, size=%d\n", sender->client_ip, sender->client_port, size);
    struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(RTP_HEADER_SIZE + size);
    if (!rtpPacket) return -1;
    rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
                  sender->seq, sender->timestamp, sender->ssrc);
    memcpy(rtpPacket->payload, frame, size);
    int ret = rtpSendPacketOverUdp(sender->sockfd, sender->client_ip, sender->client_port, rtpPacket, size);
    sender->seq++;
    sender->timestamp += 90000/25;
    free(rtpPacket);
    return ret;
}
#endif

int RtpSender_send_frame(RtpSender* sender, const uint8_t* frame, int size, int fps) {
    // 跳过起始码
    int offset = 0;
    if (frame[0] == 0x00 && frame[1] == 0x00) {
        if (frame[2] == 0x01) offset = 3;
        else if (frame[2] == 0x00 && frame[3] == 0x01) offset = 4;
    }
    const uint8_t* nal = frame + offset;
    int nal_size = size - offset;

    uint8_t nal_type = nal[0] & 0x1F;
    uint8_t nri = nal[0] & 0x60;

    // RTP max payload size
    const int max_payload = 1400;

    if (nal_size <= max_payload) {
        // === Single RTP Packet ===
        int packet_size = RTP_HEADER_SIZE + nal_size;
        uint8_t* packet = malloc(packet_size);
        if (!packet) return -1;

        rtpHeaderInit((struct RtpPacket*)packet, 0, 0, 1, RTP_VESION, RTP_PAYLOAD_TYPE_H264,
                      1, sender->seq++, sender->timestamp, sender->ssrc);
        memcpy(packet + RTP_HEADER_SIZE, nal, nal_size);

        int ret = rtpSendPacketOverUdp(sender->sockfd, sender->client_ip, sender->client_port, (struct RtpPacket*)packet, nal_size);
        free(packet);
        sender->timestamp += 90000 / fps;
        return ret;
    }

    // === FU-A fragmentation ===
    int payload_left = nal_size - 1; // skip nal[0]
    const uint8_t* payload = nal + 1;
    int ret = 0;

    while (payload_left > 0) {
        int frag_size = payload_left > max_payload - 2 ? max_payload - 2 : payload_left;
        int packet_size = RTP_HEADER_SIZE + frag_size + 2;
        uint8_t* packet = malloc(packet_size);
        if (!packet) return -1;

        int marker = (frag_size == payload_left); // Last fragment

        // RTP header
        rtpHeaderInit((struct RtpPacket*)packet, 0, 0, marker, RTP_VESION, RTP_PAYLOAD_TYPE_H264,
                      marker, sender->seq++, sender->timestamp, sender->ssrc);

        // FU indicator
        uint8_t fu_indicator = nri | 28;
        // FU header
        uint8_t fu_header = 0;
        fu_header |= nal_type;
        if (payload == nal + 1) fu_header |= 0x80; // Start bit
        if (marker) fu_header |= 0x40;             // End bit

        uint8_t* p = packet + RTP_HEADER_SIZE;
        p[0] = fu_indicator;
        p[1] = fu_header;
        memcpy(p + 2, payload, frag_size);

        ret = rtpSendPacketOverUdp(sender->sockfd, sender->client_ip, sender->client_port, (struct RtpPacket*)packet, frag_size + 2);
        free(packet);

        payload += frag_size;
        payload_left -= frag_size;
    }

    sender->timestamp += 90000 / fps;
    return ret;
}


void RtpSender_destroy(RtpSender* sender) {
    if (!sender) return;
    if (sender->sockfd > 0) close(sender->sockfd);
    free(sender);
}
