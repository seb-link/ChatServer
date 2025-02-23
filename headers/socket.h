#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFSIZE 1024

static int                server_fd;
static int                opt;
static struct sockaddr_in address;
static socklen_t          addrlen;

typedef struct {
  int sock;
  bool u;
} Client;

typedef struct s_data {
  pthread_mutex_t* data_mutex;
  pthread_mutex_t* server_mutex;
  Client (*clients)[MAXCLIENT];
} t_data;

int   init           (void);
void* getconn        (void* data);
char* getmsg         (int sock);
#endif

