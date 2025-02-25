#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAXCLIENT 2
#define PORT      8080

int   in    (char* arr[], ssize_t size, const char* target);
char* strdupli (const char* s);

#endif // COMMON_H
