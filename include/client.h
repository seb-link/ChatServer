#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

#define MAXNAMSIZE 20

char* getmsg      (int sock);
void  broadcast   (t_data *data, char* msg);
void  removeClient(t_data *data, int sock) ;

#endif
