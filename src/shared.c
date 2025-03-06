#include "common.h"

// Helper function to convert binary to hex string
void print_hex(const unsigned char *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
      printf("%02x", data[i]);
  }
  printf("\n");
}

int in(char* arr[], ssize_t size, const char* target) {
  for (ssize_t i = 0; i < size; i++) {
    if (arr[i] != NULL && strcmp(arr[i], target) == 0) {
      return 1; 
    }
  }
  return 0;
}

void quit(t_data* data) {
  data->reqshut = true;
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i< MAXCLIENT; i++) {
    if (data->clients[i]->u == true) {
      send(data->clients[i]->sock, "ERROR : The server is shutting down", BUFFSIZE, 0);
      close(data->clients[i]->sock);
    }
    // Not technically required but idc
    data->clients[i]->u    = false; 
    data->clients[i]->sock = 0;
  }
  pthread_mutex_unlock(data->data_mutex);
  pthread_mutex_destroy(data->data_mutex);
  pthread_mutex_destroy(data->server_mutex);
  exit(0);
}

char *strdupli(const char *c)
{
    char *dup = malloc(strlen(c) + 1);

    if (dup != NULL) {
       strcpy(dup, c);
    }else{
      perror("malloc");
    }
    return dup;
}
