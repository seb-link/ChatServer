#include "common.h"
#include "hcommand.h"

int kick(char *username) {}

int parcmd(char** msg) {
  char *current;
  char *command;
  current = strtok(*msg," ");
  if (!in(commands, CMD_NUM, *msg)) return CMD_INVALID;
  // No args commands
  if (strcmp(current, "/exit")) return CLI_EXIT;
  if (strcmp(current, "/stop")) return QUIT;
  if (strcmp(current, "/kick")) command = "kick";
  current = strtok(*msg," ");
  // first arg
  if (strcmp(command, "kick")) return kick(current);
  return EXIT_SUCCESS;  
}
