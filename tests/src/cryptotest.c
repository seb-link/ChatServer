#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <openssl/hmac.h> // Include OpenSSL for HMAC computation
#include "tests.h"
#include "crypto.h"       // Include the header for the function being tested

#define MEMCMP_ERROR       1
#define RANDLEN_ERROR      2
#define HASHLEN_ERROR      3
#define INIT_ERROR         4
#define RESULT_NULL        5
#define RESULT_HASH_NULL   6
#define RESULT_RAND_NULL   7

int test_generate_challenge() {
    // Initialize the cryptographic key

    // Call the function to generate a challenge
    challenge *result = generate_challenge();

    // Assert that the result is not NULL
    if (result == NULL) return RESULT_NULL;
    if (result->rand == NULL) {
      free(result); 
      return RESULT_RAND_NULL;
    }
    if (result->hash == NULL) {
      free(result); 
      return RESULT_HASH_NULL;
    }

    ssize_t _rand_len = strlen((char *)result->rand);
    ssize_t _hash_len = strlen((char *)result->hash); 

    // Assert that the random string is of the expected length
    if (!QUIET_MODE) printf("Rand len = %d\n", _rand_len);
    if (!QUIET_MODE) printf("Rand = \"%s\"\n", result->rand);
    if (_rand_len != _rand_len) {
      free(result->rand);
      free(result->hash);
      free(result);
      return RANDLEN_ERROR;
    }

    // Assert that the hash is of the expected length (SHA256_DIGEST_LENGTH)
    if (!QUIET_MODE) printf("Hash len = %d\n", _hash_len);
    // printf("Hash = \"%s\"\n", result->hash);
    if (_hash_len != SHA256_DIGEST_LENGTH) {
      free(result->rand);
      free(result->hash);
      free(result);
      return HASHLEN_ERROR;
    }

    // Recompute the HMAC-SHA256 hash
    unsigned char recomputed_hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len = 0;

    HMAC(
        EVP_sha256(),                  // Hash algorithm
        sharkey, strlen(sharkey),      // Key and its length
        result->rand, _rand_len,        // Data and its length
        recomputed_hash, &hash_len     // Output buffer and size
    );

    // Assert that the recomputed hash matches the hash in the challenge
    if( memcmp(result->hash, recomputed_hash, SHA256_DIGEST_LENGTH) != 0) {
      free(result->rand);
      free(result->hash);
      free(result);
      return MEMCMP_ERROR;
    } 

    if (!QUIET_MODE) printf("test_generate_challenge passed!\n");

    // Free allocated memory
    free(result->rand);
    free(result->hash);
    free(result);

    return EXIT_SUCCESS;
}

int main ( void ) {
  int result = 0, current = 0;
  if (crypto_init() != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to initialize crypto.\n");
    return INIT_ERROR;
  }


  // Counters for each error type
  int init_error_count = 0;
  int result_null_count = 0;
  int result_rand_null_count = 0;
  int result_hash_null_count = 0;
  int randlen_error_count = 0;
  int hashlen_error_count = 0;
  int memcmp_error_count = 0;

  for (int i = 0; i < TEST_NUM; i++) {
    current = test_generate_challenge();

    switch (current) {
      case EXIT_SUCCESS:
        result++;
        break;
      case INIT_ERROR:
        init_error_count++;
        break;
      case RESULT_NULL:
        result_null_count++;
        break;
      case RESULT_RAND_NULL:
        result_rand_null_count++;
        break;
      case RESULT_HASH_NULL:
        result_hash_null_count++;
        break;
      case RANDLEN_ERROR:
        randlen_error_count++;
        break;
      case HASHLEN_ERROR:
        hashlen_error_count++;
        break;
      case MEMCMP_ERROR:
        memcmp_error_count++;
        break;
      default:
        printf("Error: Unknown error occurred.\n");
        break;
    }
  }

  // Print results
  printf("Test Passed : %d/%d\n", result, TEST_NUM);
  printf("That's %lf%%.\n", (double) result / TEST_NUM * 100);

  // Print error percentages
  printf("INIT_ERROR: %lf%%\n", (double) init_error_count / TEST_NUM * 100);
  printf("RESULT_NULL: %lf%%\n", (double) result_null_count / TEST_NUM * 100);
  printf("RESULT_RAND_NULL: %lf%%\n", (double) result_rand_null_count / TEST_NUM * 100);
  printf("RESULT_HASH_NULL: %lf%%\n", (double) result_hash_null_count / TEST_NUM * 100);
  printf("RANDLEN_ERROR: %lf%%\n", (double) randlen_error_count / TEST_NUM * 100);
  printf("HASHLEN_ERROR: %lf%%\n", (double) hashlen_error_count / TEST_NUM * 100);
  printf("MEMCMP_ERROR: %lf%%\n", (double) memcmp_error_count / TEST_NUM * 100);

  return 0;
}
