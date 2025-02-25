#ifndef SERVER_CRYPTO_H
#define SERVER_CRYPTO_H

#include <openssl/sha.h>

typedef struct {
  unsigned char* rand;
  unsigned char* hash;
} challenge;

char* key;

challenge generate_challenge(void);

#endif
