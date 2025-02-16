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
                SO_REUSEADDR | SO_REUSEPORT, &opt,
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

  return 0;
}

int getconn(){
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    return -EXIT_FAILURE;   
  }

  int new_sock = accept(server_fd, (struct sockaddr*)&address, &addrlen);
  if (new_sock < 0) {
    perror("accept");
    return -EXIT_FAILURE;
  }
  return new_sock;
} 
