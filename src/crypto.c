#include "passgen/common.h"
#include "passgen/password.h"

#include "common.h"
#include "crypto.h"


char* sharkey = NULL;

int get_random_bytes(unsigned char **buffer, size_t length);

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

challenge* generate_challenge(void) {
  unsigned char *server_hmac = NULL;
  unsigned char *hmac = NULL;
  unsigned int  hmac_len = SHA256_DIGEST_LENGTH;
  static unsigned char *random = NULL;
  charset_t charset = 0;
  static challenge result;
  result.hash = NULL;
  result.rand = NULL;


  // Server generates random challenge (64 bytes)
  charset = 7; // 7 = 0111
  random = generate_password(charset, RAND_LEN);
  
  // Server computes HMAC-SHA256
  server_hmac = malloc(hmac_len);
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

  result.hash = server_hmac;
  result.rand = random;
  return &result;
}


