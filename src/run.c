#include "common.h"
#include "socket.h"
#include "run.h"

void* threadTarget(void* data);

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
  
  data.clients = &Pclients;

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

  return;
}


void* threadTarget(void* data) {
  int exitcode = 0;
  bool running = 1;
  data = (t_data*) data;
  while (true) {
    if (exitcode != 0) break;
    int new_sock = (int) getconn(data);
    if (new_sock < 0) {
      perror("accept");
      printf("FATAL : Cloud not get connections !\n");
      exitcode = 1;
    }

    printf("Got connection !\n");
    /*
    char* msg = malloc(sizeof(char) * BUFFSIZE);
    while (running) {
      msg = getmsg(new_sock);
      if (strcmp(msg,EXITMSG)) {
        free(msg);
        close(new_sock);
        printf("Client Exited.\n");
        break;
      }
      printf("Client : %s\n",msg);  
      free(msg);
    */
    close(new_sock);
  }
  return (void*) exitcode;
}
