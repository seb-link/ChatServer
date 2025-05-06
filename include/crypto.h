#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <errno.h>

#define RAND_LEN 64

typedef struct {
  unsigned char *rand;
  unsigned char *hash;
} challenge;

extern char *sharkey;  // Declare sharkey as extern

int        crypto_init        (void);
challenge *generate_challenge (void);
int        get_random_bytes   (unsigned char **buffer, size_t length);

#endif // CRYPTO_H
