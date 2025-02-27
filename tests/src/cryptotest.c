#include "common.h"
#include "crypto.h"

#include "tests.h"

challenge* custom_generate_challenge(unsigned char random[64]) {
    static challenge result;
    result.hash = NULL;
    result.rand = NULL;
    size_t key_len = strlen(sharkey);
  
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
  
    result.hash = server_hmac;
    result.rand = &random;
    return &result;
  }

int test(void) {
    challenge* a;
    if (!QUIET_MODE) printf("INFO : Generating challenge...\n");
    a = generate_challenge();
    if (!QUIET_MODE) printf("INFO : Challenge generated !\n");
    if (!QUIET_MODE) {
      printf("Hash data : ");
      puts(a->hash);
      print_hex(a->hash, strlen(a->hash));
      printf("Rand data : ");
      puts(a->rand);
      print_hex(a->rand, strlen(a->rand));
    }

    challenge* b;
    if (!QUIET_MODE) printf("INFO : Generating challenge...\n");
    b = custom_generate_challenge(a->rand);
    if (!QUIET_MODE) printf("INFO : Challenge generated !\n");
    if (!QUIET_MODE) {
      printf("Hash data : ");
      puts(b->hash);
      print_hex(b->hash, strlen(b->hash));
      printf("Rand data : ");
      puts(b->rand);
      print_hex(a->rand, strlen(b->rand));
    }


    if (CRYPTO_memcmp(a->hash, b->hash, SHA256_DIGEST_LENGTH)) {
        free(a->hash);
        free(b->hash);
        printf("HASH MISMATCH ");
        return 0;
    }
    free(a->hash);
    free(b->hash);
    return 1;
}

int main(void) {
    printf("TEST : CRYPTO : generate_challenge()\n");
    if (!QUIET_MODE) printf("INFO : Initing crypto...\n");
    if (crypto_init() != 0) {
        printf("FATAL : CRYPTO : Cloud not initialize crypto !\n");
        return 1;
    }
    int result = 0;
    for (int i = 0; i<TEST_NUM; i++) {
        result += test();
    } 
    printf("Got %d good on %d tries\n", result, TEST_NUM);
    printf("That would be %.4lf percent \n", (double) result / TEST_NUM * 100);
    return 0;
}