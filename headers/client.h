#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

void  broadcast (t_data *data, char* msg);
char* getmsg    (int sock);

#endif