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

    printf("Got connection !\n");
    char* msg = malloc(sizeof(char) * BUFFSIZE);
    if (!msg) {
      perror("malloc");
      log_msg(LOG_FATAL, "Cloud not allocate memory using malloc");
      quit(data);
    }

    char *username = malloc(MAXNAMSIZE);
    username = getusername(data, new_sock); // New client username
    if (!username) 
      continue;
    
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
    send(new_sock, challenge->rand, 64, 0);
    sleep(1);
    result = getmsg(new_sock);

    // Server verification
    if (CRYPTO_memcmp(challenge->hash, result, SHA256_DIGEST_LENGTH) == 0) {
      printf("Authentication successful!\n");
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
      if (msg != 0) {
        if (strcmp(&msg[0],"/")) {
          switch(parcmd(&msg,data)) {
	   case CMD_INVALID :
	     send(new_sock, "WARN : Command not found", BUFFSIZE, 0);
             break;

	   case CMD_SYNTAX_ERR :
	     send(new_sock, "WARN : Command syntax invalid", BUFFSIZE, 0);
	     break; 

	   case CLI_EXIT:
             free(msg);
             close(new_sock);
             printf("Client Exited.\n");
             running = false; // Client exited so bye bye
	     continue;
             break; // Never reached but for good practice

           case KICK_NOTFOUND :
             send(new_sock, "WARN: user not found", BUFFSIZE, 0);
             break;

           case QUIT:
             printf("Stopping server...\n");
	     log_msg(LOG_INFO,"The client \"%s\" has stoped the server.",username);
             quit(data);
             break; // Never reached but for good practice
           default :
             break;
          } // switch -> command output
        }else { // if (strcmp(...)) -> if it's a command
	  printf("%s : %s\n", username, msg);
          broadcast(data, msg, username); 
	} 
      } // if (msg != 0) -> if there is a msg
      free(msg);
    } // while (running) -> while connection alive
    close(new_sock);
  } // while (true)
  return (void*) (intptr_t) exitcode;
}
