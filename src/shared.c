#include "common.h"

int in(char* arr[], ssize_t size, const char* target) {
  for (ssize_t i = 0; i < size; i++) {
    if (arr[i] != NULL && strcmp(arr[i], target) == 0) {
      return 1; 
    }
  }
  return 0;
}

char *strdupli(const char *c)
{
    char *dup = malloc(strlen(c) + 1);

    if (dup != NULL) {
       strcpy(dup, c);
    }else{
      perror("malloc");
    }
    return dup;
}
