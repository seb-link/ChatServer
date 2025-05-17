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

/* Server's thread-shared data structure */
typedef struct s_data {
  pthread_mutex_t  *data_mutex;         /* Data mutex : used when accessing clients */
  pthread_mutex_t  *server_mutex;       /* Server mutex : used when getting connections */
  pthread_mutex_t  *log_mutex;          /* Log mutex : used when logging */
  Client           clients[MAXCLIENTS]; /* clients : Array of all connected clients */
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

