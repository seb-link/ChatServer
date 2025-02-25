#include "common.h"
#include "crypto.h"

int get_random_bytes(void *buffer, size_t length);

// read key
int crypto_init(void) {
  key = malloc(64);
  if (!key) {
    perror("malloc");
    return 1;
  }
  FILE *file = fopen("key.key","r");
  if (!file) {
    perror("fopen");
    return 1;
  }
  size_t s = fread(key, 1, 64, file);
  if (s != 64)
    return 1;
  return 0;
}

challenge generate_challenge(void) {
  unsigned char* hash = NULL;
  unsigned char* rand = NULL;
  challenge result;
  // Generate 64 bytes of random data
  randgen(rand, 64);
  result.rand = rand;
  // Hash(key + random_data)
  strcat(rand,key);
  //SHA256(rand, strlen(rand), hash);
  result.hash = hash;
  // return Hash + random_data
  return result;
}

// Generate cryptographically secure random bytes
int get_random_bytes(void *buffer, size_t length) {
  int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
      perror("open");
      return -1;
  }

  ssize_t bytes_read = 0;
  while (bytes_read < length) {
      ssize_t result = read(fd, (char*)buffer + bytes_read, length - bytes_read);
      if (result == -1) {
          if (errno == EINTR) continue;  // Retry if interrupted
          perror("read");
          close(fd);
          return -1;
      } else if (result == 0) {
          fprintf(stderr, "Unexpected EOF reading /dev/urandom\n");
          close(fd);
          return -1;
      }
      bytes_read += result;
  }

  close(fd);
  return 0;
}