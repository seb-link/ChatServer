#ifndef SERVER_CRYPTO_H
#define SERVER_CRYPTO_H

#include <openssl/sha.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
  unsigned char* rand;
  unsigned char* hash;
} challenge;

char* key;

int crypto_init(void);
challenge generate_challenge(void);

#endif
