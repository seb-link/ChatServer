#include "common.h"
#include "socket.h"
#include "client.h"
#include "log.h"

uint8_t status_codes[] = {
  0,   // SUCCESS
  100, // WARNING
  200, // ERROR
  201  // KICK_ERROR
};

const size_t  banned_username_len =  3;
const char   *banned_username[]   =  {"FATAL", "ERROR", "WARN"};

/**
 * @brief Retrieves the username of a client from the socket.
 *
 * @param data Pointer to the server's data structure.
 * @param sock The socket descriptor for the client connection.
 * @return char* The cleaned username if successful, NULL otherwise.
 */
char *getusername(t_data* data, int sock) {

  char   *username  = NULL;
  bool   duplicate  = false;
  size_t len        = MAXNAMESIZE;

  username = getmsg(sock, &len); // New client username
  
  if (!username) {
    printf("Connection closed before username received\n");
    close(sock);
    removeClient(data, sock);
    log_msg(LOG_ERROR, "Client closed connection without sending a username");
    return NULL; // Failed
  }

  if (strlen(username) == 0) {
    printf("Empty username received\n");
    msgsend(sock, "ERROR: Username cannot be empty\n", Status_ERROR);
    log_msg(LOG_ERROR, "ERROR: Empty username received.");
    free(username);
    close(sock);
    removeClient(data, sock);
    return NULL; // Failed
  }

  for (size_t i = 0; i < banned_username_len; i++) {
    if (strcmp(username, banned_username[i]) == 0) {
      msgsend(sock, "ERROR: Invalid username", Status_ERROR);
      close(sock);
      return NULL;
    }
  }

  if (check_username(username)) {
    msgsend(sock, "ERROR: Username can only contain letters and numbers.", Status_ERROR);
    close(sock);
    return NULL;
  }

  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (data->clients[i]->u) {                                    // Is it used ?
      if (data->clients[i]->username) {                           // Is the username non-null ?
        if (strcmp(data->clients[i]->username, username) == 0) {  // Is is it a duplicate ?
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

  return username;
}

/**
 * @brief Removes a client from the server's client list.
 *
 * @param data Pointer to the server's data structure.
 * @param sock The socket descriptor of the client to remove.
 */
void removeClient(t_data *data, int sock) {
  
  pthread_mutex_lock(data->data_mutex);
  
  for (int i = 0; i < MAXCLIENTS; i++) {    
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

/**
 * @brief Broadcasts a message to all connected clients.
 *
 * @param data Pointer to the server's data structure.
 * @param msg The message to broadcast.
 * @param username The username of the sender.
 */
void broadcast(t_data *data, char *msg, char *username) {
  char *smsg = malloc(sizeof(username) + sizeof(msg) + 5); // message to send
  
  sprintf(smsg, "%s : %s", username, msg);
  pthread_mutex_lock(data->data_mutex);
  for (int i = 0; i<MAXCLIENTS; i++) {
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

/**
 * @brief Sends a message to a specific client.
 *
 * @param sock The socket descriptor of the client.
 * @param msg The message to send.
 * @param status_code The status code to send along with the message.
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int msgsend(const int sock, const char* msg, const Status status_code) {
  ssize_t status_code_status, msg_status = 0;

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

/**
 * @brief Receives a message from a client.
 *
 * @param sock The socket descriptor of the client.
 * @return char* The received message, or NULL on failure.
 */
char *getmsg(int sock, size_t *len) {
  char    *msg = NULL;
  ssize_t bytes_recv = 0;
  if ( !len ) {
    *len = BUFFSIZE;
  }
  if ( *len < 1 ) {
    return NULL;
  }

  msg = malloc(*len);
  if (!msg) {
      perror("malloc");
      fprintf(stderr, "malloc failed to allocate %zu bytes.\n", *len);
      return NULL;
  }

  bytes_recv = recv(sock, msg, *len - 1, 0);
  if (bytes_recv < 0) {
    perror("read");
    free(msg);
    return NULL;
  }
  
  if (bytes_recv == 0) {
    return 0;
  }

  msg[bytes_recv] = '\0';
  
  return msg;
}

/* Helper function */
int check_username(char *str) {
  for (; *str != '\0'; str++) {
      if (!isalnum((unsigned char)*str)) {
          return 1;
      }
  }
  return 0;
}

