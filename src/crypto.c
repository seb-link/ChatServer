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

challenge* generate_challenge() {
  static challenge result;
  result.hash = NULL;
  result.rand = NULL;
  size_t key_len = strlen(sharkey);

  // Server generates random challenge (64 bytes)
  static unsigned char random[64] = { 0 };
  get_random_bytes(&random,64);
  printf("RAND: \"");
  print_hex(random,64);
  // Server computes HMAC-SHA256
  unsigned char* server_hmac = malloc(SHA256_DIGEST_LENGTH);
  unsigned int   hmac_len    = SHA256_DIGEST_LENGTH;
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

  printf("Good HMAC : \"");
  print_hex(server_hmac, SHA256_DIGEST_LENGTH);
  printf("\"\n");
  result.hash = server_hmac;
  result.rand = &random;
  return &result;
}


// Generate cryptographically secure random bytes
int get_random_bytes(unsigned char **buffer, size_t length) {
  *buffer = malloc(length);
  if (*buffer == NULL) {
      perror("malloc");
      return -1;
  }

  int fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
      perror("open");
      free(*buffer);
      return -1;
  }

  ssize_t result = read(fd, *buffer, length);
  if (result == -1) {
       perror("read");
       close(fd);
       free(*buffer);
       return -1;
  } else if (result == 0) {
    fprintf(stderr, "Unexpected EOF reading /dev/urandom\n");
    close(fd);
    free(*buffer);
    return -1;
  }

  if (result != length) {
    free(*buffer);
    close(fd);
    return -1;
  }
  
  close(fd);
  return 0;
}
