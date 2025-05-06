#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#define MAXCLIENT 2
#define PORT      8090

#include "socket.h"

int   in       (char *arr[], ssize_t size, const char *target);
void  quit     (t_data *data);
char *strdupli (const char *s);
void  print_hex(const unsigned char *data, size_t len);

#endif // COMMON_H
