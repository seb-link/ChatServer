#include "common.h"
#include "client.h"


void removeClient(t_data *data, int sock) {
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i < MAXCLIENT; i++) {
    if (data->clients[i]->sock == sock && data->clients[i]->u == true) {
      data->clients[i]->u = false;
    }
  }
  pthread_mutex_unlock(data->data_mutex);
}

void broadcast(t_data *data, char* msg) {
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i<MAXCLIENT; i++) {
    if (data->clients[i]->u == true) {
      send(data->clients[i]->sock, msg, strlen(msg), 0); // idk why 0
    }
  }
  pthread_mutex_unlock(data->data_mutex);
  return;
}

char *getmsg(int sock) {
  char *msg = malloc(BUFFSIZE);
  if (!msg) {
      perror("malloc");
      return NULL;
  }
  ssize_t s = recv(sock, msg, BUFFSIZE - 1,0);
  if (s < 0) {
      perror("read");
      free(msg);
      return NULL;
  } else if (msg == 0) {
    return 0;
  }
  msg[s] = '\0';
  return msg;
}

