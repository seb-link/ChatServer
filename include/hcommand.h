#ifndef COMMAND_H
#define COMMAND_H

#include "socket.h"

static char *commands[] = {"/exit","/stop","/kick"};

#define CMD_NUM sizeof(commands) / sizeof(commands[0]) // Number of commands

#define QUIT           99 // Close the program
#define CLI_EXIT       1  // Client exited
#define CMD_INVALID    2  // Command doesn't exist
#define CMD_SYNTAX_ERR 3  // Command syntax error
#define CMD_PERM_ERR   4  // Command permission error
#define KICK_NOTFOUND  5  // Kick function : user not found 

int parcmd(char** msg, t_data *data);

#endif // COMMAND_H
