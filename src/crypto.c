#include "common.h"
#include "crypto.h"


challenge generate_challenge(void) {
  unsigned int   seed = time(NULL) ^ getpid(); // semi-unique seed 
  unsigned char* hash = NULL;
  unsigned char* rand = NULL;
  challenge result;
  // Generate 64 bytes of random data
  for( int i = 0; i < 64; ++i){
    rand[i] = '0' + rand_r(&seed)%72; // starting on '0', ending on '}'
  }
  result.rand = rand;
  // Hash(key + random_data)
  strcat(rand,key);
  SHA256(rand, strlen(rand), hash);
  result.hash = hash;
  // return Hash + random_data
  return result;
}

