#include "common.h"
#include "socket.h"
#include "run.h"

void run(void) {
  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket\n");
    return;
  }
  
  return;
}
