#ifndef CHATSERVER_SOCKET_H
#define CHATSERVER_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"

#define BUFFSIZE 1024

#define ERROR_SERVER_FULL  -1
#define ERROR_ACCEPT_ERROR -2 
#define ERROR_NULL_DATA    -3 

static int                server_fd;
static int                opt;
static struct sockaddr_in address;
static socklen_t          addrlen;

int   init           (void);
int   getconn        (void);

#endif /* CHATSERVER_SOCKET_H */
