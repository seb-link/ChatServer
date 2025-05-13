#include <stdio.h>
#include <stdlib.h>

#include "run.h"

int main(int argc, char **argv) {
  if (argc > 3) {
    fprintf(stderr, "Too much arguments");
    return 1;
  }

  char *configFilePath = malloc(CONFIG_NAME_LENGTH);
  
  if (argc != 2) {
    configFilePath = "config.ini";
  } else {
    snprintf(configFilePath, CONFIG_NAME_LENGTH, "%s", argv[1]);
  }
  
  run(configFilePath);
  
  free(configFilePath);
  return 0;
}

