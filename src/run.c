#include "common.h"
#include "hcommand.h"
#include "crypto.h"
#include "socket.h"
#include "run.h"
#include "client.h"
#include "log.h"
#include "config.h"

void *threadTarget(void *data);

t_data data;

void run(const char *configFilePath) {  
  pthread_mutex_t data_mutex;
  pthread_mutex_t server_mutex;
  pthread_mutex_t log_mutex;

  pthread_t threads[MAXCLIENTS];

  Client clients[MAXCLIENTS] = { 0 };

  size_t i;
  
  memset(clients, 0, MAXCLIENTS * sizeof(Client)); /* Redundant but harmless */

  pthread_mutex_init(&data_mutex, NULL);
  pthread_mutex_init(&server_mutex, NULL);
  pthread_mutex_init(&log_mutex, NULL);
  
  data.data_mutex = &data_mutex;
  data.server_mutex = &server_mutex;
  data.log_mutex = &log_mutex;
  
  memcpy(data.clients, clients, sizeof(Client) * MAXCLIENTS);

  if (init() != EXIT_SUCCESS) {
    printf("FATAL : Cloud not initialize socket.\n");
    return;
  }

  if (config_init(configFilePath) != EXIT_SUCCESS) {
    printf("FATAL : Error happened during the config file parsing.\n");
    return;
  }
 
  if (app_config.authEnabled == 1) {
    if (crypto_init() != EXIT_SUCCESS) {
      printf("FATAL : Cloud not read from keyfile.\n");
      return;
    }
  }

  if (log_init("log.log") != EXIT_SUCCESS) {
    printf("FATAL : Cloud not initialize log file.\n");
    return;
  }

  printf("Finished initializing.\n");

  for (i = 0; i < MAXCLIENTS; i++) {
    pthread_create(&threads[i], NULL, threadTarget, &data);
  }

  printf("Listening of port : %d...\n",PORT);
  log_msg(LOG_INFO, "Server started and is listening on port %d",PORT);
  
  for (i = 0; i < MAXCLIENTS; i++) {
    pthread_join(threads[i], 0);
  }

  quit();

  return;
}


void *threadTarget(void* sdata) {
  int exitcode = 0;
  bool running = 1;

  int new_sock = 0;	
  char ipstr[INET_ADDRSTRLEN];
  struct sockaddr_in *addr = NULL; 
  struct in_addr inaddr; 
  char *msg, *username = NULL;

  while (true) {
    new_sock = getconn();

    if (new_sock == ERROR_SERVER_FULL) {
      printf("ERROR : The server is full !\n");
      log_msg(LOG_ERROR, "The server is full, dropping connection !");
      (void) msgsend(new_sock, "ERROR : The server is full", Status_ERROR);
      continue;
    }

    if (new_sock == ERROR_ACCEPT_ERROR) {
      printf("FATAL : Cloud not get connections (Error on accept) !\n");
      log_msg(LOG_FATAL, "Cloud not get connections (Error on accept)");
      quit();
    }

    if (new_sock == ERROR_NULL_DATA) {
      printf("FATAL : NULL passed in the treadTarget function !!\n");
      log_msg(LOG_FATAL, "NULL passed in the treadTarget function !!");
      pthread_exit( (intptr_t*) 1);
    }
    
    if ( getpeername(new_sock, (struct sockaddr*)&address, &addrlen) != 0 ) {
      perror("getpeername");
      log_msg(LOG_ERROR, "Cloud not call getpeername()");
      close(new_sock);
      continue;
    }
    addr = (struct sockaddr_in*)&address;
    inaddr = addr->sin_addr;
    
    inet_ntop(AF_INET, &inaddr, ipstr, INET_ADDRSTRLEN);
    
    printf("Got connection !\n");
    log_msg(LOG_INFO, "Got connection from %s", ipstr);
    
    msg = malloc(sizeof(char) * BUFFSIZE);
    if (!msg) {
      perror("malloc");
      log_msg(LOG_FATAL, "Cloud not allocate memory using malloc");
      quit();
    }

    username = getusername(new_sock); // New client username
    if (!username) {
      free(msg);
      removeClient(new_sock); 
      continue; 
    }

    pthread_mutex_lock(data.data_mutex);
    for (int i = 0; i < MAXCLIENTS; i++) {
      if (data.clients[i].sock == new_sock) {
        data.clients[i].username = strdupli(username);
        break;
      }
    }
    pthread_mutex_unlock(data.data_mutex);

    if (app_config.authEnabled == 1) {
      // Client authentication
      if ( authenticate_user(new_sock) != AUTH_SUCCESS ) {
        log_msg(LOG_INFO, "[Auth] The client \"%s\" has failed authentication", username);
        free(username);
        free(msg);
        removeClient(new_sock);
        break; 
      }
    }

    printf("New client : %s\n",username);
    while (running) {
      msg = getmsg(new_sock, NULL);
      if (msg != NULL) {
        if (msg[0] == '/') {
          switch (parcmd(msg)) {
            case CMD_INVALID :
              if(msgsend(new_sock, (char* )"WARN  : Command not found", Status_WARNING)) {
                log_msg(LOG_ERROR, "Error sending message to client");
                running = false;
                continue; // Will close socket and remove client
              }
              break;

            case CMD_SYNTAX_ERR :
              if(msgsend(new_sock, (char* )"WARN  : Command syntax invalid", Status_WARNING)) {
                log_msg(LOG_ERROR, "Error sending message to client");
                running = false;
                continue; // Will close socket and remove client
              }
              break; 

            case CLI_EXIT:
              free(msg);
              printf("Client Exited.\n");
              running = false; // Client exited so bye bye
              continue;
              break; // Never reached but for good practice

            case KICK_NOTFOUND :
              if(msgsend(new_sock, (char* )" WARN  : user not found", Status_WARNING)) {
                log_msg(LOG_ERROR, "Error sending message to client");
                running = false;
                continue; // will close socket and remove client
              }
              break;

            case QUIT:
              printf("Stopping server...\n");
              log_msg(LOG_INFO,"The client \"%s\" has stoped the server.",username);
              quit();
              break; // Never reached but for good practice

            default :
              break;
          } // switch -> command output
        } else { // if (strcmp(...)) -> if it's a command
          printf("%s : %s\n", username, msg);
          broadcast(msg, username); 
        } 
      } else {
        log_msg(LOG_ERROR, "Client broke connection");
        running = false;
        continue; // Will close the socket
      } // If msg = 0 then no msg was received
    } // while (running) -> while connection alive
    removeClient(new_sock);
    log_msg(LOG_INFO, "Client %s disconnected", username);
  } // while (true)
  return (void*) (intptr_t) exitcode;
}
