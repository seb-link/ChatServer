#include "tests.h"
#include "crypto.h"
#include "common.h"

int main(void) {
  int a = 64;
  char *rand = malloc(a);
  for (int i = 0; i<TEST_NUM; ++i) {
    bzero(rand,a);
    get_random_bytes(&rand, a);
    print_hex(rand,a); // The only way i found to check if random
  }
  return 0;
}

