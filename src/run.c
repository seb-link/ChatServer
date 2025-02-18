#include "common.h"
#include "socket.h"
#include "run.h"

void run(void) {
  bool running = true;
  /* Not implemented
  Client clients[MAXCLIENT];
  Client null;
  null.u = false;
  null.sock = 0;
  
  for(int i = 0; i< MAXCLIENT; i++) {
    clients[i] = null;
  }
  */

  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket\n");
    return;
  }

  printf("Listening of port : %d...\n",PORT);
  // TODO : Add multi-threading system to handle multiple clients.
  // int new_sock = getconn();
  int new_sock = legacy_getconn();
  if (new_sock < 0) {
    perror("accept");
    printf("FATAL : Cloud not get connections !\n");
    return;
  }

  printf("Got connection !\n");
  
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
  }

  return;
}
