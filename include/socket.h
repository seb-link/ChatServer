#ifndef CHATSERVER_SOCKET_H
#define CHATSERVER_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "common.h"

#define BUFFSIZE 1024

static int                server_fd;
static int                opt;
static struct sockaddr_in address;
static socklen_t          addrlen;

typedef struct {
  char *username;
  int sock;
  bool u;
} Client;

typedef struct s_data {
  pthread_mutex_t  *data_mutex;
  pthread_mutex_t  *server_mutex;
  Client           (*clients)[MAXCLIENT];
  bool             reqshut;
} t_data;

int   init           (void);
char *getmsg         (int sock);
int   getconn        (t_data *socks);

#endif /* CHATSERVER_SOCKET_H */
