#ifndef CHATSERVER_LOG_H
#define CHATSERVER_LOG_H

#include <stdarg.h>

typedef enum {
  LOG_FATAL,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO
} LogLevel;

extern char *levels[]; 
extern FILE *log_file;

int  log_init  (const char *filename);
void log_close (void);
void log_msg   (LogLevel level, const char *msg, ...);


#endif /* CHATSERVER_LOG_H */

