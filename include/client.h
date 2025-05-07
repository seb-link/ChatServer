#ifndef CHATSERVER_CLIENT_H
#define CHATSERVER_CLIENT_H

#include "socket.h"

#define MAXNAMESIZE 20

extern const char   *banned_username[];
extern const size_t banned_username_len;

typedef enum {
  Status_SUCCESS,     // No problem
  Status_WARNING,     // Minor problem occured
  Status_ERROR,       // Connection ending error occured
  Status_ERROR_KICKED // Resulting of the kick command
} Status;

void  broadcast      (t_data *data, char *msg, char *username);
char  *getmsg        (int sock, size_t *len);
char  *getusername   (t_data *data, int sock);
void  removeClient   (t_data *data, int sock);
int   check_username (char *str);
int   msgsend        (const int sock, const char* msg, const Status status_code); 

#endif /* CHATSERVER_CLIENT_H */
