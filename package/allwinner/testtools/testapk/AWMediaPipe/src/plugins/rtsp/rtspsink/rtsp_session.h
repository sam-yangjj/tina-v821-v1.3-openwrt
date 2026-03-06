#ifndef RTSP_SESSION_H
#define RTSP_SESSION_H

#include "rtp_sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int client_fd;
    char client_ip[64];
    int client_port;
    int client_rtp_port;
    int client_rtcp_port;
    int state;
}RtspSession;

RtspSession* RtspSession_create(int client_fd, const char* client_ip, int client_port);
void RtspSession_destroy(RtspSession* session);


#endif // RTSP_SESSION_H
