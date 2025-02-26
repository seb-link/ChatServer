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
  size_t len = 64;
  unsigned char* hash =   NULL;
  unsigned char* rand =   malloc(len);
  challenge* result;
  result->rand = NULL;
  result->hash = NULL;
  // Generate 64 bytes of random data
  int a = get_random_bytes(&rand, len);
  if (a < 0)
    return NULL;
  result->rand = rand;
  // Hash(key + random_data)
  strcat(rand,sharkey);
  //SHA256(rand, strlen(rand), hash); // it works when it wants.
  hash = "haram";
  result->hash = hash;
  // return Hash + random_data
  return result;
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
