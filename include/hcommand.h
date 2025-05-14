#ifndef CHATSERVER_COMMAND_H
#define CHATSERVER_COMMAND_H

#include "socket.h"

static char *commands[] = {"/exit","/stop","/kick"};

#define CMD_NUM sizeof(commands) / sizeof(commands[0]) // Number of commands

#define QUIT           99 // Close the program
#define CLI_EXIT       2  // Client exited
#define CMD_INVALID    3  // Command doesn't exist
#define CMD_SYNTAX_ERR 4  // Command syntax error
#define CMD_PERM_ERR   5  // Command permission error
#define KICK_NOTFOUND  6  // Kick function : user not found 

int parcmd(char *originalCmd, t_data *data);

#endif /* CHATSERVER_COMMAND_H */
