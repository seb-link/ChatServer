#include "passgen/common.h"
#include "passgen/password.h"

#include "common.h"
#include "crypto.h"


char* sharkey = NULL;

int get_random_bytes(unsigned char **buffer, size_t length);

// read key
int crypto_init(void) {
  sharkey = malloc(64);
  if (!sharkey) {
    perror("malloc");
    return 1;
  }
  FILE *file = fopen("key.key","r");
  if (!file) {
    perror("fopen");
    return 1;
  }
  size_t s = fread(sharkey, 1, 64, file);
  if (s != 64)
    return 1;
  return 0;
}

challenge* generate_challenge(void) {
  static challenge result;
  result.hash = NULL;
  result.rand = NULL;

  // Server generates random challenge (64 bytes)
  static unsigned char *random;
  charset_t charset = 7; // 7 = 0111
  random = generate_password(charset, RAND_LEN);
  
  // Server computes HMAC-SHA256
  unsigned char* server_hmac   = malloc(SHA256_DIGEST_LENGTH);
  unsigned int   hmac_len      = SHA256_DIGEST_LENGTH;
  bzero(server_hmac, hmac_len);

  if (!server_hmac) {
    perror("malloc");
    return NULL;
  }

  unsigned char* hmac = HMAC(
    EVP_sha256(),                                   // Hash algorithm
    (const unsigned char*)sharkey, strlen(sharkey), // Key and its length
    (const unsigned char*)random,  64,              // Data and its length
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

  result.hash = server_hmac;
  result.rand = random;
  return &result;
}


