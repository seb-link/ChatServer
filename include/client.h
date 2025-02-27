#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

#define MAXNAMSIZE 20

const extern char   *banned_username[];
const extern size_t banned_username_len;

char* getmsg         (int sock);
void  broadcast      (t_data *data, char* msg, char* username);
char* getusername    (t_data* data, int sock);
void  removeClient   (t_data *data, int sock);
int   check_username (char *str);

#endif
