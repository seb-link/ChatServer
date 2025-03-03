#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <errno.h>


typedef struct {
  unsigned char (*rand)[64];
  unsigned char* hash;
} challenge;

extern char* sharkey;  // Declare sharkey as extern

int        crypto_init        (void);
challenge* generate_challenge (void);

#endif // CRYPTO_H
