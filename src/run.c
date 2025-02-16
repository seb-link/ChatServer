#include "common.h"
#include "socket.h"
#include "run.h"

void run(void) {
  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket\n");
    return;
  }
  
  int new_sock = getconn();
  if (new_sock < 0) {
    printf("FATAL : Cloud not get connections !\n");
    return;
  }
  
  return;
}
