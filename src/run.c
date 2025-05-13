#include "common.h"
#include "hcommand.h"
#include "crypto.h"
#include "socket.h"
#include "run.h"
#include "client.h"
#include "log.h"
#include "config.h"

void* threadTarget(void* data);

void run(const char *configFilePath) {  
  t_data data;
  pthread_mutex_t data_mutex;
  pthread_mutex_t server_mutex;
  pthread_t threads[MAXCLIENTS];
  Client clients[MAXCLIENTS];
  Client (*Pclients)[MAXCLIENTS] = &clients;
  size_t i = 0; 

  pthread_mutex_init(&data_mutex, NULL);
  pthread_mutex_init(&server_mutex, NULL);
  data.data_mutex = &data_mutex;
  data.server_mutex = &server_mutex;
  
  data.clients = Pclients;

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

  memset(clients, 0, MAXCLIENTS * sizeof(Client));

  for (i = 0; i < MAXCLIENTS; i++) {
    pthread_create(&threads[i], NULL, threadTarget, &data);
  }

  printf("Listening of port : %d...\n",PORT);
  log_msg(LOG_INFO, "Server started and is listening on port %d",PORT);
  
  for (i = 0; i < MAXCLIENTS; i++) {
    pthread_join(threads[i], 0);
  }

  quit(&data);

  return;
}


void *threadTarget(void* sdata) {
  int exitcode = 0;
  bool running = 1;
  t_data* data;
  data = (t_data*) sdata;

  int new_sock = 0;	
  char ipstr[INET_ADDRSTRLEN];
  struct sockaddr_in *addr = NULL; 
  struct in_addr inaddr; 
  char *msg, *username = NULL;

  while (true) {
    new_sock = getconn(data);

    if (new_sock == ERROR_SERVER_FULL) {
      printf("ERROR : The server is full !\n");
      log_msg(LOG_ERROR, "The server is full, dropping connection !");
      (void) msgsend(new_sock, "ERROR : The server is full", Status_ERROR);
      continue;
    }

    if (new_sock == ERROR_ACCEPT_ERROR) {
      printf("FATAL : Cloud not get connections (Error on accept) !\n");
      log_msg(LOG_FATAL, "Cloud not get connections (Error on accept)");
      quit(data);
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
      quit(data);
    }

    username = getusername(data, new_sock); // New client username
    if (!username) {
      free(msg);
      cleanup_client(data, new_sock); 
      continue; 
    }

    pthread_mutex_lock(data->data_mutex);
    for (int i = 0; i < MAXCLIENTS; i++) {
      if (data->clients[i]->sock == new_sock) {
        data->clients[i]->username = strdupli(username);
        break;
      }
    }
    pthread_mutex_unlock(data->data_mutex);

    if (app_config.authEnabled == 1) {
      // Client authentication
      if ( authenticate_user(data, new_sock) != AUTH_SUCCESS ) {
        log_msg(LOG_INFO, "[Auth] The client \"%s\" has failed authentication", username);
        free(username);
        free(msg);
        cleanup_client(data, new_sock);
        break; 
      }
    }

    printf("New client : %s\n",username);
    while (running) {
      msg = getmsg(new_sock, NULL);
      if (msg != NULL) {
        if (strcmp(&msg[0],"/") == 0) {
          switch(parcmd(&msg,data)) {
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
              close(new_sock);
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
              quit(data);
              break; // Never reached but for good practice
            default :
              break;
          } // switch -> command output
        } else { // if (strcmp(...)) -> if it's a command
          printf("%s : %s\n", username, msg);
          broadcast(data, msg, username); 
        } 
      } /* if (msg != NULL) -> if there were no error */ else if (msg != 0) {
        log_msg(LOG_ERROR, "Client broke connection");
        running = false;
        continue; // Will close the socket
      } // If msg = 0 then no msg was received
    } // while (running) -> while connection alive
    close(new_sock);
    cleanup_client(data, new_sock);
    log_msg(LOG_INFO, "Client %s disconnected", username);
  } // while (true)
  return (void*) (intptr_t) exitcode;
}
