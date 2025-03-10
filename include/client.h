#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

#define MAXNAMSIZE 20

extern const char   *banned_username[];
extern const size_t banned_username_len;

typedef enum {
  Status_SUCCESS,     // No problem
  Status_WARNING,     // Minor problem occured
  Status_ERROR,       // Connection ending error occured
  Status_ERROR_KICKED // Resulting of the kick command
} Status;

char* getmsg         (int sock);
void  broadcast      (t_data *data, char* msg, char* username);
char* getusername    (t_data* data, int sock);
void  removeClient   (t_data *data, int sock);
int   check_username (char *str);
int   msgsend        (int sock, char* msg, Status status_code); 

#endif
