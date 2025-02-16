#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>

static int server_fd;
static int opt;
static struct sockaddr_in address;
static socklen_t addrlen;

int init(void);
#endif

