#include "common.h"
#include "hcommand.h"
#include "socket.h"
#include "client.h"
#include "log.h"

int kick(char *username,t_data *data) {
  char *msg = malloc(BUFFSIZE);
  if (!msg) {
    perror("malloc");
    log_msg(LOG_FATAL, "Cloud not allocate memory using malloc when calling the kick function");
    quit(data);
  }
  sprintf(msg, "%s just got kicked !", username);
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i < MAXCLIENT; i++) {
    if (data->clients[i]->u && strcmp(data->clients[i]->username, username) == 0) {
      msgsend(data->clients[i]->sock, "You were kicked", Status_ERROR_KICKED);
      close(data->clients[i]->sock);
      broadcast(data,msg,username);
      return 0;
    }
  }
  return KICK_NOTFOUND;
}

int parcmd(char** msg,t_data *data) {
  char *current;
  char *command;
  current = strtok(*msg," ");
  if (!in(commands, CMD_NUM, *msg)) return CMD_INVALID;
  // No args commands
  if (!strcmp(current, "/exit")) return CLI_EXIT;
  if (!strcmp(current, "/stop")) return QUIT;
  if (!strcmp(current, "/kick")) command = "kick";
  current = strtok(*msg," ");
  // first arg
  if (!strcmp(command, "kick")) return kick(current, data);
  return EXIT_FAILURE;  
}
