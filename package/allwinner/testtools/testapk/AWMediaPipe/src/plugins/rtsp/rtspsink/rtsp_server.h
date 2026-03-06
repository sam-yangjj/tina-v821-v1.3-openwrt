#ifndef RTSP_SERVER_H
#define RTSP_SERVER_H

#include "rtsp_session.h"

struct RtspServer {
    int listen_fd;
    char ip[64];
    int port;
};
typedef struct RtspServer RtspServer;


RtspServer* RtspServer_create(const char* ip, int port);
void RtspServer_destroy(RtspServer* server);

#endif // RTSP_SERVER_H
