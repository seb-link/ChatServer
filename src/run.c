#include "common.h"
#include "hcommand.h"
#include "crypto.h"
#include "socket.h"
#include "run.h"
#include "client.h"
#include "log.h"

void* threadTarget(void* data);
void quit(t_data* data);

void run(void) {  
  t_data data;
  pthread_mutex_t data_mutex;
  pthread_mutex_t server_mutex;
  pthread_t threads[MAXCLIENT];
  Client clients[MAXCLIENT];
  Client (*Pclients)[MAXCLIENT] = &clients;
  Client null;
  null.u = false;
  null.sock = 0;
  null.username = NULL;

  pthread_mutex_init(&data_mutex, NULL);
  pthread_mutex_init(&server_mutex, NULL);
  data.data_mutex = &data_mutex;
  data.server_mutex = &server_mutex;
  
  data.clients = Pclients;

  if (init() != 0) {
    printf("FATAL : Cloud not initialize socket.\n");
    return;
  }
  
  if (crypto_init() != 0) {
    printf("FATAL : Cloud not read from keyfile.\n");
    return;
  }
  
  if (log_init("log.log")) {
    printf("FATAL : Cloud not initialize log file.\n");
    return;
  }

  printf("Finished initializing.\n");

  for(int i = 0; i< MAXCLIENT; i++) {
    clients[i] = null;
    pthread_create(&threads[i], NULL, threadTarget, &data);
  }
  
  

  printf("Listening of port : %d...\n",PORT);
  log_msg(LOG_INFO, "Server started and is listening on port %d",PORT);
  for (int i = 0; i<MAXCLIENT; i++) {
    pthread_join(threads[i], 0);
  }

  quit(&data);

  return;
}


void* threadTarget(void* sdata) {
  int exitcode = 0;
  bool running = 1;
  t_data* data;
  data = (t_data*) sdata;
  while (true) {
    int new_sock = getconn(data);
    if (new_sock < 0) {
      perror("accept");
      printf("FATAL : Cloud not get connections !\n");
      log_msg(LOG_FATAL, "Cloud not get connections");
      quit(data);
    }
    
    char ipstr[INET_ADDRSTRLEN];
    if (getpeername(new_sock, (struct sockaddr*)&address, &addrlen) != 0) {
      perror("getpeername");
      log_msg(LOG_ERROR, "Cloud not call getpeername()");
      close(new_sock);
      continue;
    }
    struct sockaddr_in* addr = (struct sockaddr_in*)&address;
    struct in_addr inaddr = addr->sin_addr;
    inet_ntop(AF_INET, &inaddr, ipstr, INET_ADDRSTRLEN);
    printf("Got connection !\n");
    log_msg(LOG_INFO, "Got connection from %s", ipstr);
    char* msg = malloc(sizeof(char) * BUFFSIZE);
    if (!msg) {
      perror("malloc");
      log_msg(LOG_FATAL, "Cloud not allocate memory using malloc");
      quit(data);
    }

    char *username = malloc(MAXNAMSIZE);
    username = getusername(data, new_sock); // New client username
    if (!username) { 
      free(username);
      continue; // Connection closed in getusername
    }
    pthread_mutex_lock(data->data_mutex);
    for(int i = 0; i < MAXCLIENT; i++) {
      if(data->clients[i]->sock == new_sock) {
        data->clients[i]->username = strdupli(username);
        break;
      }
    }
    pthread_mutex_unlock(data->data_mutex);

    // Server authentication
    

    // Client authentication
    challenge* challenge;
    char *result;
    challenge = generate_challenge();
    if (!challenge) { 
      printf("ERROR : Cloud not generate challenge ! Dropping connection...\n");
      send(new_sock, "ERROR : Server side problem !", BUFFSIZE, 0);
      close(new_sock);
      log_msg(LOG_ERROR, "[Auth] Cloud not generate challenge.");
      continue;
    }
    if(msgsend(new_sock, (char* )challenge->rand)) {
      log_msg(LOG_ERROR, "Cloud not send challenge to client");
      close(new_sock);
      removeClient(data, new_sock);
      continue;
    } 
    sleep(1);
    result = getmsg(new_sock);

    // Server verification
    if (CRYPTO_memcmp(challenge->hash, result, SHA256_DIGEST_LENGTH) == 0) {
      printf("Authentication successful!\n");
      log_msg(LOG_INFO, "[Auth] Client \"%s\" has successfully authenticated", username);
    } else {
      printf("Client %s : Authentication failed!\n", username);
      send(new_sock, "ERROR : Invalid HMAC", BUFFSIZE, 0);
      removeClient(data, new_sock);
      log_msg(LOG_INFO, "[Auth] The client \"%s\" has failed authentication");
      close(new_sock);
      continue;
    }   

    printf("New client : %s\n",username);
    while (running) {
      msg = getmsg(new_sock);
      if (msg != NULL) {
        if (!strcmp(&msg[0],"/")) {
          switch(parcmd(&msg,data)) {
            case CMD_INVALID :
              if(msgsend(new_sock, (char* )"WARN : Command not found")) {
                log_msg(LOG_ERROR, "Error sending message to client");
                running = false;
                continue; // Will close socket and remove client
              }
              break;

            case CMD_SYNTAX_ERR :
              if(msgsend(new_sock, (char* )"WARN : Command syntax invalid")) {
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
              if(msgsend(new_sock, (char* )"WARN: user not found")) {
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
    removeClient(data, new_sock);
    log_msg(LOG_INFO, "Client %s disconnected", username);
  } // while (true)
  return (void*) (intptr_t) exitcode;
}
