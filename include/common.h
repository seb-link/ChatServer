#ifndef CHATSERVER_COMMON_H
#define CHATSERVER_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#define MAXCLIENTS 2
#define PORT      8090

#define DO_AUTH 1

typedef struct {
  char *username;
  int sock;
  bool u;
} Client;

typedef struct s_data {
  pthread_mutex_t  *data_mutex;
  pthread_mutex_t  *server_mutex;
  Client           (*clients)[MAXCLIENTS];
  bool             reqshut;
} t_data;

#include "socket.h"

int   in       (char *arr[], ssize_t size, const char *target);
void  quit     (t_data *data);
char *strdupli (const char *s);
void  print_hex(const unsigned char *data, size_t len);

#endif /* CHATSERVER_COMMON_H */
