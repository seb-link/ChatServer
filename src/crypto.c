#include "passgen/common.h"
#include "passgen/password.h"

#include "common.h"
#include "crypto.h"

#include "log.h"
#include "client.h"
#include "socket.h"


char* sharkey = NULL;

// read key
int crypto_init(void) {
  FILE *file = NULL;
  size_t s = 0;

  sharkey = malloc(64);

  if (!sharkey) {
    perror("malloc");
    return EXIT_FAILURE;
  }
  
  file = fopen("key.key","r");
  if (!file) {
    perror("fopen");
    return EXIT_FAILURE;
  }
  
  s = fread(sharkey, 1, 64, file);
  if (s != 64) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

challenge *generate_challenge(void) {
  unsigned char *server_hmac = NULL;
  unsigned char *hmac = NULL;
  unsigned int  hmac_len = SHA256_DIGEST_LENGTH;
  unsigned char *random = NULL;

  charset_t charset = 0;
  
  challenge *result = malloc(sizeof(challenge));
  result->hash = NULL;
  result->rand = NULL;

  // Server generates random challenge (64 bytes)
  charset = 7; // 7 = 0111
  random = generate_password(charset, RAND_LEN);
  
  // Server computes HMAC-SHA256
  server_hmac = malloc(hmac_len + 1);
  if (!server_hmac) {
    perror("malloc");
    return NULL;
  }

  memset(server_hmac, 0, hmac_len); 
  
  hmac = HMAC(
    EVP_sha256(),                                   // Hash algorithm
    (const unsigned char*)sharkey, strlen(sharkey), // Key and its length
    (const unsigned char*)random,  RAND_LEN,              // Data and its length
    server_hmac, &hmac_len                          // Output buffer and size
  );
  
  if (server_hmac == 0) {
    fprintf(stderr, "HMAC computation failed!\n");
    return NULL;
  }
  
  if (!hmac) {
    perror("HMAC");
    return NULL;
  }

  server_hmac[hmac_len] = '\0';

  print_hex(server_hmac, 32);

  result->hash = server_hmac;
  result->rand = random;
  return result;
}


/**
 * @brief Authenticates a user by generating a challenge, sending it to the client, 
 *        and verifying the client's response.
 *
 * @param data Pointer to the server's data structure.
 * @param client_sock The socket descriptor for the client connection.
 * @return size_t Returns AUTH_SUCCESS if authentication is successful, AUTH_FAILED otherwise.
 */
size_t authenticate_user( int client_sock ) 
{
  challenge* challenge = NULL;
  char *result = NULL;

  challenge = generate_challenge();
  if ( !challenge ) { 
    printf("ERROR : Cloud not generate challenge ! Dropping connection...\n");
    (void) msgsend(client_sock, "ERROR : Server side problem !", Status_ERROR);
    log_msg(LOG_ERROR, "[Auth] Cloud not generate challenge.");
    return AUTH_FAILED;
  }
  
  /* Send the random data to the client */
  if( msgsend(client_sock, (char* )challenge->rand, Status_SUCCESS) != EXIT_SUCCESS ) {
    log_msg(LOG_ERROR, "[Auth] Cloud not send challenge to client");
    /* Free allocated memory */
    free(challenge->hash);
    free(challenge->rand);
    free(challenge);
    return AUTH_FAILED;
  } 

  sleep(1);
  result = getmsg(client_sock, NULL);

  if ( !result ) {
    log_msg(LOG_ERROR, "[Auth] Failed to receive response from client");
    /* Free allocated memory */
    free(challenge->hash);
    free(challenge->rand);
    free(challenge);
    return AUTH_FAILED;
  }

  /* Server verification */
  if ( CRYPTO_memcmp(challenge->hash, result, SHA256_DIGEST_LENGTH) != 0 ) {
    (void) msgsend(client_sock, "ERROR : Invalid HMAC", Status_ERROR);
    log_msg(LOG_ERROR, "[Auth] The client sent a wrong HMAC");
    /* Free allocated memory */
    free(result);
    free(challenge->hash);
    free(challenge->rand);
    free(challenge);
    return AUTH_FAILED;
  }

  free(result);
  free(challenge->hash);
  free(challenge->rand);
  free(challenge);
  
  return AUTH_SUCCESS;
}
