#include "tests.h"
#include "common.h"
#include "client.h"

#include <assert.h>

int test_check_username (void)
{
  char *test1 = "abc";
  char *test2 = "1+2test";
  char *test3 = "test123";
  char *test4 = "test!";

  int result[4];

  result[0] = check_username(test1);
  result[1] = check_username(test2);
  result[2] = check_username(test3);
  result[3] = check_username(test4);

  assert(result[0] == 0);
  assert(result[1] == 1);
  assert(result[2] == 0);
  assert(result[3] == 1);

  return 0;
}

int main (void)
{
  test_check_username();
  return 0;
}

