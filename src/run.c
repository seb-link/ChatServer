#include "common.h"
#include "hcommand.h"
#include "crypto.h"
#include "socket.h"
#include "run.h"
#include "client.h"

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
  null.username = "";

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
  printf("Finished initializing.\n");

  for(int i = 0; i< MAXCLIENT; i++) {
    clients[i] = null;
    pthread_create(&threads[i], NULL, threadTarget, &data);
  }
  

  printf("Listening of port : %d...\n",PORT);
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
      quit(data);
    }

    printf("Got connection !\n");
    char* msg = malloc(sizeof(char) * BUFFSIZE);
    if (!msg) {
      perror("malloc");
      quit(data);
    }

    char *username = malloc(MAXNAMSIZE);
    username = getusername(data, new_sock); // New client username
    printf("DEBUG : \"%s\"\n",username);
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
      close(new_sock);
      continue;
    }

    if (strcmp(result, challenge->hash) != 0) {
      send(new_sock, "Cloudn't identify client", BUFFSIZE, 0); 
      close(new_sock);
      continue;
    }


    printf("New client : %s\n",username);
    while (running) {
      msg = getmsg(new_sock);
      if (msg != 0) {
        printf("%s : %s\n", username, msg);
        broadcast(data, msg, username);
        if (strcmp(&msg[0],"/")) {
          switch(parcmd(&msg,data)) {
            case CLI_EXIT:
              free(msg);
              close(new_sock);
              printf("Client Exited.\n");
              continue;
              break; // Never reached but for good practice

            case KICK_NOTFOUND :
              send(new_sock, "ERROR : user not found", BUFFSIZE, 0);
              break;

            case QUIT:
              printf("Stopping server...\n");
              quit(data);
              break; // Never reached but for good practice
            default :
              break;
          } // switch -> command output
        } // if (strcmp(...)) -> if it's a command
      } // if (msg != 0) -> if there is a msg
      free(msg);
    } // while (running) -> while connection alive
    close(new_sock);
  } // while (true)
  return (void*) (intptr_t) exitcode;
}
