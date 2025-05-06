#include "common.h"
#include "client.h"
#include "log.h"

uint8_t status_codes[] = {
  0,   // SUCCESS
  100, // WARNING
  200, // ERROR
  201  // KICK_ERROR
};

char *trim_whitespace(char *str);

const size_t  banned_username_len =  3;
const char   *banned_username[]   =  {"FATAL", "ERROR", "WARN"};

// Get Client username
char *getusername(t_data* data, int sock) {

  char *username   = NULL;
  char *clean_name = NULL;
  bool duplicate   = false;

  username = malloc(MAXNAMSIZE);
  username = getmsg(sock); // New client username
  
  if(!username) {
    printf("Connection closed before username received\n");
    close(sock);
    removeClient(data, sock);
    log_msg(LOG_ERROR, "Client closed connection without sending a username");
    return NULL; // Failed
  }

  clean_name = trim_whitespace(username);
  if(strlen(clean_name) == 0) {
    printf("Empty username received\n");
    msgsend(sock, "ERROR: Username cannot be empty\n", Status_ERROR);
    free(username);
    close(sock);
    removeClient(data, sock);
    return NULL; // Failed
  }

  for (size_t i = 0; i < banned_username_len; i++) {
    if (strcmp(clean_name, banned_username[i]) == 0) {
      msgsend(sock, "ERROR: Invalid username", Status_ERROR);
      close(sock);
      return NULL;
    }
  }

  if (check_username(clean_name)) {
    msgsend(sock, "ERROR: Username can only contain letters and numbers.", Status_ERROR);
    close(sock);
    return NULL;
  }

  pthread_mutex_lock(data->data_mutex);
  for(int i = 0; i < MAXCLIENT; i++) {
    if(data->clients[i]->u) {                                  // Is it used ?
      if (data->clients[i]->username) {                        // Is the username non-null ?
        if (!strcmp(data->clients[i]->username, clean_name)) { // Is is it a duplicate ?
          duplicate = true;
          break;
        }
      }
    }
  }

  pthread_mutex_unlock(data->data_mutex);

  if (duplicate) {
    msgsend(sock, "ERROR: Username already taken\n", Status_ERROR);
    free(username);
    close(sock);
    removeClient(data, sock);
    return NULL; // Failed
  }

  return clean_name;
}

void removeClient(t_data *data, int sock) {
  
  pthread_mutex_lock(data->data_mutex);
  
  for (int i = 0; i < MAXCLIENT; i++) {    
    if (data->clients[i]->sock == sock && data->clients[i]->u == true) {
    
      data->clients[i]->u = false;
      if (data->clients[i]->username) {
        printf("Client %s Disconnected !\n", data->clients[i]->username);
        free(data->clients[i]->username);
        data->clients[i]->username = NULL;
      }

    }
  }

  pthread_mutex_unlock(data->data_mutex);

  return;
}

void broadcast(t_data *data, char *msg, char *username) {
  char *smsg = malloc(sizeof(username) + sizeof(msg) + 5); // message to send
  
  sprintf(smsg, "%s : %s", username, msg);
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i<MAXCLIENT; i++) {
    if ( data->clients[i]->u && data->clients[i]->sock > 0 ) {
      if (msgsend(data->clients[i]->sock, smsg, Status_SUCCESS) != 0) {
        removeClient(data, data->clients[i]->sock);
      }
    }
  }
  pthread_mutex_unlock(data->data_mutex);
  
  free(smsg);
  return;
}

int msgsend(int sock, char* msg, Status status_code) {
  int status_code_status, msg_status = 0;

  status_code_status = send(sock, &status_codes[status_code], sizeof(uint8_t), 0);
  if (status_code_status < 0) {
    perror("send");
    return EXIT_FAILURE;
  }

  msg_status = send(sock, msg, strlen(msg), 0);
  
  if (msg_status < 0) {
    perror("send");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

char *getmsg(int sock) {
  char    *msg;
  ssize_t recv_status = 0;

  msg = malloc(BUFFSIZE);        
  if (!msg) {
      perror("malloc");
      return NULL;
  }

  recv_status = recv(sock, msg, BUFFSIZE - 1,0);
  if (recv_status < 0) {
    perror("read");
    free(msg);
    return NULL;
  }
  
  if (recv_status == 0) {
    return 0;
  }

  msg[recv_status] = '\0';
  
  return msg;
}

int check_username(char *str) {
  for (; *str != '\0'; str++) {
      if (!isalnum((unsigned char)*str)) {
          return 1;
      }
  }
  return 0;
}

// Removes whitespaces (isspace from ctypes)
char* trim_whitespace(char* str) { 
  while(isspace((unsigned char)*str)) str++;
  if(*str == '\0') return str;
  char* end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  *(end+1) = '\0';
  return str;
}
// //
