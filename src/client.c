#include "common.h"
#include "client.h"

char* trim_whitespace(char* str);
const size_t  banned_username_len =  1;
const char   *banned_username[]   =  {"ERROR"};

// Get Client username
char* getusername(t_data* data, int sock) {
  char *username = malloc(MAXNAMSIZE);
  username = getmsg(sock); // New client username
  if(!username) {
    printf("Connection closed before username received\n");
    close(sock);
    removeClient(data, sock);
    return NULL; // Failed
  }
  char* clean_name = trim_whitespace(username);
  if(strlen(clean_name) == 0) {
    printf("Empty username received\n");
    send(sock, "ERROR: Username cannot be empty\n", BUFFSIZE, 0);
    free(username);
    close(sock);
    removeClient(data, sock);
    return NULL; // Failed
  }
  for (size_t i = 0; i < banned_username_len; i++) {
    if (strcmp(clean_name, banned_username[i]) == 0) {
      send(sock, "ERROR: Invalid username", BUFFSIZE, 0);
      close(sock);
      return NULL;
    }
  }

  if (check_username(clean_name)) {
    send(sock, "ERROR: Username can only contain letters and numbers.", BUFFSIZE, 0);
    close(sock);
    return NULL;
  }
  
  bool duplicate = false;
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
    send(sock, "ERROR: Username already taken\n", 30, 0);
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
	free(data->clients[i]->username);
        data->clients[i]->username = NULL;
      }
    }
  }
  pthread_mutex_unlock(data->data_mutex);
}

void broadcast(t_data *data, char* msg,char* username) {
  char* smsg = malloc(sizeof(username) + sizeof(msg) + 5); // message to send
  sprintf(smsg, "%s : %s", username, msg);
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i<MAXCLIENT; i++) {
    if (data->clients[i]->u == true) {
      send(data->clients[i]->sock, smsg, strlen(msg), 0); // idk why 0
    }
  }
  pthread_mutex_unlock(data->data_mutex);
  return;
}

char *getmsg(int sock) {
  char *msg = malloc(BUFFSIZE);
  if (!msg) {
      perror("malloc");
      return NULL;
  }
  ssize_t s = recv(sock, msg, BUFFSIZE - 1,0);
  if (s < 0) {
      perror("read");
      free(msg);
      return NULL;
  } else if (msg == 0) {
    return 0;
  }
  msg[s] = '\0';
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
