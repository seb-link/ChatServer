#include "common.h"
#include "command.h"

int parcmd(char** msg) {
  char *current;
  current = strtok(*msg," ");
  if (!in(commands, CMD_NUM, *msg)) return CMD_INVALID;
  // No args commands
  if (strcmp(current, "/exit")) return CLI_EXIT;
  if (strcmp(current, "/stop")) return QUIT;
  current = strtok(*msg," ");
  return EXIT_SUCCESS;  
}