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

/* Interacting functions */
int   msgsend        (const int sock, const char* msg, const Status status_code); 
char  *getmsg        (int sock,      size_t *len);
void  broadcast      (t_data *data,  char *msg, char *username);
char  *getusername   (t_data *data,  int sock);

/* Client management functions */
void  cleanup_client (t_data *data, int sock);

/* Helper functions */

#endif /* CHATSERVER_CLIENT_H */
