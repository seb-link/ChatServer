#ifndef CHATSERVER_CRYPTO_H
#define CHATSERVER_CRYPTO_H

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <errno.h>

#include "socket.h"

#define RAND_LEN 64

#define AUTH_SUCCESS 1
#define AUTH_FAILED  0

typedef struct {
  unsigned char *rand;
  unsigned char *hash;
} challenge;

extern char *sharkey;  // Declare sharkey as extern

int        crypto_init        (void);
challenge *generate_challenge (void);
size_t     authenticate_user  ( t_data *data , int client_sock );

#endif /* CHATSERVER_CRYPTO_H */
