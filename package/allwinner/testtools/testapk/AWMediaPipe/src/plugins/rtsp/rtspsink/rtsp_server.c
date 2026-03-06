#include "rtsp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rtsp_session.h"

RtspServer* RtspServer_create(const char* ip, int port) {
    RtspServer* server = (RtspServer*)malloc(sizeof(RtspServer));
    if (!server) return NULL;
    strncpy(server->ip, ip, sizeof(server->ip)-1);
    server->ip[sizeof(server->ip)-1] = '\0';
    server->port = port;
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("socket");
        free(server);
        return NULL;
    }
    int on = 1;
    setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (bind(server->listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server->listen_fd);
        free(server);
        return NULL;
    }
    if (listen(server->listen_fd, 10) < 0) {
        perror("listen");
        close(server->listen_fd);
        free(server);
        return NULL;
    }
    return server;
}

void RtspServer_destroy(RtspServer* server) {
    if (!server) return;
    if (server->listen_fd > 0) close(server->listen_fd);
    free(server);
}
