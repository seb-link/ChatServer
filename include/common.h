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

typedef struct {
  char *username;
  int sock;
  bool u;
} Client;

typedef struct s_data {
  pthread_mutex_t  *data_mutex;
  pthread_mutex_t  *server_mutex;
  Client           clients[MAXCLIENTS];
  bool             reqshut;
} t_data;

#include "socket.h"

extern t_data data;

int   in               (char *arr[], ssize_t size, const char *target);
void  quit             (void);
char  *strdupli        (const char *s);
void  print_hex        (const unsigned char *data, size_t len);
char  *remove_newlines (const char *str);

#endif /* CHATSERVER_COMMON_H */

