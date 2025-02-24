#include "common.h"
#include "command.h"
#include "socket.h"
#include "run.h"
#include "client.h"

void* threadTarget(void* data);
void quit(t_data* data);

void run(void) {
  bool running = true;
  
  t_data data;
  pthread_mutex_t data_mutex;
  pthread_mutex_t server_mutex;
  pthread_t threads[MAXCLIENT];
  Client clients[MAXCLIENT];
  Client (*Pclients)[MAXCLIENT] = &clients;
  Client null;
  null.u = false;
  null.sock = 0;
  
  pthread_mutex_init(&data_mutex, NULL);
  pthread_mutex_init(&server_mutex, NULL);
  data.data_mutex = &data_mutex;
  data.server_mutex = &server_mutex;
  
  data.clients = Pclients;

  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket\n");
    return;
  }

  for(int i = 0; i< MAXCLIENT; i++) {
    clients[i] = null;
    pthread_create(&threads[i], NULL, threadTarget, &data);
  }
  

  printf("Listening of port : %d...\n",PORT);
  for (int i = 0; i<MAXCLIENT; i++) {
    pthread_join(threads[i], 0);
  }

  quit(&data);

  return;
}


void* threadTarget(void* sdata) {
  int exitcode = 0;
  bool running = 1;
  t_data* data;
  data = (t_data*) sdata;
  while (true) {
    int new_sock = (int) getconn(data);
    if (new_sock < 0) {
      perror("accept");
      printf("FATAL : Cloud not get connections !\n");
      quit(data);
    }

    printf("Got connection !\n");
    char* msg = malloc(sizeof(char) * BUFFSIZE);
    if (!msg) {
      perror("malloc");
      quit(data);
    }
    char *username = malloc(MAXNAMSIZE);
    username = getmsg(server_fd); // New client username
    
    if (strcmp(username, "\n")) {
      printf("No username provided dropping connections...");
      close(new_sock); 
      continue; 
    }
    printf("Username is : \"%x\"\n",username);
    for (int i =0; i < MAXCLIENT; i++) {
      if (data->clients[i]->sock == new_sock) { 
	data->clients[i]->username = username;
	break;
      }
    }
    printf("New client : %s\n",username);
    while (running) {
      msg = getmsg(new_sock);
      if (msg != 0) {
        printf("Client : %s\n",msg);
        broadcast(data, msg);
        if (strcmp(&msg[0],"/")) {
          switch(parcmd(&msg)) {
            case CLI_EXIT:
              free(msg);
              close(new_sock);
              printf("Client Exited.\n");
              return 0;
              break;
            case QUIT:
              quit(data);
            default :
              break;
          }
        }
      }
      printf("No message received\n");
      free(msg);
    }
    close(new_sock);
  }
  return (void*) exitcode;
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
