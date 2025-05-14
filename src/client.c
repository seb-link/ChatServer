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

/* Private/local function constructors */
void  removeClient   (t_data *data, int sock);
int   check_username (char *str);
char  *remove_spaces (const char *str);
/* Private/local function constructors */

/**
 * @brief Retrieves the username of a client from the socket.
 *
 * @param data Pointer to the server's data structure.
 * @param sock The socket descriptor for the client connection.
 * @return char* The cleaned username if successful, NULL otherwise.
 */
char *getusername(t_data* data, int sock) {

  char   *username, *cleanname  = NULL;
  bool   duplicate  = false;
  size_t len        = MAXNAMESIZE;
  size_t i          = 0;

  username = getmsg(sock, &len); // New client username
  
  if (!username) {
    printf("Connection closed before username received\n");
    log_msg(LOG_ERROR, "Client closed connection without sending a username");
    return NULL; // Failed
  }

  if (strlen(username) == 0) {
    printf("Empty username received\n");
    (void) msgsend(sock, "ERROR: Username cannot be empty\n", Status_ERROR);
    log_msg(LOG_ERROR, "ERROR: Empty username received.");
    free(username);
    return NULL; // Failed
  }

  cleanname = remove_spaces(username);
  if ( !cleanname ) {
    free(username);
    (void) msgsend(sock, "ERROR : Server side error", Status_ERROR);
    return NULL;
  }
  free(username);

  for (i = 0; i < banned_username_len; i++) {
    if (strcmp(cleanname, banned_username[i]) == 0) {
      msgsend(sock, "ERROR: Invalid username", Status_ERROR);
      log_msg(LOG_ERROR, "ERROR: Username in banned username");
      free(cleanname);
      return NULL;
    }
  }

  if (check_username(cleanname)) {
    msgsend(sock, "ERROR: Username can only contain letters and numbers.", Status_ERROR);
    log_msg(LOG_ERROR, "ERROR: The username contains forbidden characters.");
    free(cleanname);
    return NULL;
  }

  pthread_mutex_lock(data->data_mutex);
  for (i = 0; i < MAXCLIENTS; i++) {
    if (data->clients[i]->u) {                                    // Is it used ?
      if (data->clients[i]->username) {                           // Is the username non-null ?
        if (strcmp(data->clients[i]->username, cleanname) == 0) { // Is is it a duplicate ?
          duplicate = true;
          break;
        }
      }
    }
  }
  pthread_mutex_unlock(data->data_mutex);

  if (duplicate) {
    msgsend(sock, "ERROR: Username already taken\n", Status_ERROR);
    log_msg(LOG_ERROR, "ERROR: Another user is already logged in with that username.");
    free(cleanname);
    return NULL; // Failed
  }

  return cleanname;
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
  char *smsg = malloc(MAXNAMESIZE + BUFFSIZE + 5); // message to send
  
  sprintf(smsg, "%s : %s", username, msg);
  for (int i = 0; i<MAXCLIENTS; i++) {
    if ( data->clients[i]->u && data->clients[i]->sock > 0 ) {
      if ( strcmp(data->clients[i]->username, username) != 0 ) {
        if ( msgsend(data->clients[i]->sock, smsg, Status_SUCCESS) != 0 ) {
          cleanup_client(data, data->clients[i]->sock);
        }
      }
    }
  }
  
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
  char    *msg       = NULL;
  ssize_t bytes_recv = 0;
  size_t  length     = BUFFSIZE;
  
  if ( len != NULL ) {
    length = *len;
  }

  if ( length < 1 ) {
    return NULL;
  }

  msg = malloc(length);
  if (!msg) {
      perror("malloc");
      fprintf(stderr, "malloc failed to allocate %zu bytes.\n", length);
      return NULL;
  }
  
  memset(msg, 0, length);

  bytes_recv = recv(sock, msg, length - 1, 0);
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

/* Helper function (Private) */
int check_username(char *str) {
  size_t i = 0;

  for (i = 0; i < strlen(str); ++i) {
    if ( isalnum(str[i]) == 0 && str[i] != 0) {
      log_msg(LOG_DEBUG, "Username contains : \"%c\"", str[i]);
      return 1;
    }
  }

  return 0;
}

/* Helper function (Private) */
char *remove_spaces ( const char *str ) {
  char *result = NULL;
  size_t str_index, result_index = 0;

  result = malloc( strlen(str) + 1 );
  if (!result) {
    perror("malloc");
    log_msg(LOG_ERROR, "malloc failed to allocate %d");
    return NULL;
  }
  
  memset(result, 0, strlen(str) + 1);

  for (str_index = 0; str_index < strlen(str); ++str_index) {
    if ( !isspace(str[str_index]) ) {
      result[result_index] = str[str_index];
      ++result_index;
    }
  }

  result[strlen(str) + 1] = '\0';

  return result;
}

/* @brief The function to call when you need to remove a client.
 *
 * @param data Pointer to the server's data structure.
 * @param sock The socket descriptor of the client to remove.
 */
void cleanup_client (t_data *data, int sock) {
  close(sock);
  removeClient(data, sock);
}

