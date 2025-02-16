#include "common.h"
#include "socket.h"
#include "run.h"

void run(void) {
  Client clients[MAXCLIENT];
  Client null;
  null.u = false;
  null.sock = 0;
  
  for(int i = 0; i< MAXCLIENT; i++) {
    clients[i] = null;
  }

  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket\n");
    return;
  }
  // TODO : Add multi-threading system to handle multiple clients.
  int new_sock = getconn();
  if (new_sock < 0) {
    perror("accept");
    printf("FATAL : Cloud not get connections !\n");
    return;
  }
  
  return;
}
