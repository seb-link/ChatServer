#include "common.h"
#include "socket.h"

// Copied from "https://www.geeksforgeeks.org/socket-programming-cc/" and adapted to my case
int init(void) {
  opt = 1;
  addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    return EXIT_FAILURE;
  }

  if (setsockopt(server_fd, SOL_SOCKET,
                SO_REUSEADDR, &opt,
                sizeof(opt))) {
    perror("setsockopt");
    return EXIT_FAILURE;
  }
	
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr*) &address,
             sizeof(address)) < 0) {
  perror("bind failed");
  return EXIT_FAILURE;
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    return EXIT_FAILURE;   
  }

  return 0;
}

int getconn(t_data *socks) {
  if (socks == NULL) {
    fprintf(stderr, "Error: NULL data passed to getconn\n");
    return -EXIT_FAILURE;
  }
 
  bool alloc = false;
  int new_sock = 0;
  
  /* Listen for connection (one thread at a time) */
  pthread_mutex_lock(socks->server_mutex);
  new_sock = accept(server_fd, (struct sockaddr*)&address, &addrlen);
  pthread_mutex_unlock(socks->server_mutex);

  if (new_sock < 0) {
    perror("accept");
    return -EXIT_FAILURE;
  }

  if (socks->reqshut == true) {
    pthread_exit(0);
  }

  pthread_mutex_lock(socks->data_mutex);
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (socks->clients[i]->u == false) {
      socks->clients[i]->u = true;
      socks->clients[i]->sock = new_sock;
      socks->clients[i]->username = NULL;
      alloc = true;
      break;
    }
  }
  pthread_mutex_unlock(socks->data_mutex);

  if (alloc == 0) {
    close(new_sock);
  }

  return new_sock;
}
