#include "common.h"
#include "client.h"
#include "log.h"

// Helper function to convert binary to hex string
void print_hex(const unsigned char *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
      printf("%02x", data[i]);
  }
  printf("\n");
}

/* Helper function */
int in(char* arr[], ssize_t size, const char* target) {
  for (ssize_t i = 0; i < size; i++) {
    if (arr[i] != NULL && strcmp(arr[i], target) == 0) {
      return i;
    }
  }
  return -1;
}

/* Make the server stop */
void quit(t_data *data) {
  log_close();
  data->reqshut = true;
  for (int i = 0; i< MAXCLIENTS; i++) {
    if (data->clients[i]->u == true) {
      (void) msgsend(data->clients[i]->sock, "ERROR : The server is shutting down", Status_ERROR);
      close(data->clients[i]->sock);
    }
    // Not technically required but idc
    data->clients[i]->u    = false; 
    data->clients[i]->sock = 0;
  }

  pthread_mutex_destroy(data->data_mutex);
  pthread_mutex_destroy(data->server_mutex);
  exit(0);
}

/* Helper function */
char *strdupli(const char *c) {
    char *dup = malloc(strlen(c) + 1);

    if (!dup){
      perror("malloc");
      return NULL;
    }else {
      strcpy(dup,c); // Copies c -> dup
    }
    return dup; // Note: Should be freed
}
