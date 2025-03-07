#include "common.h"
#include "log.h"

char* levels[] = {
	"[FATAL] ", 
	"[ERROR] ", 
	"[WARN] ", 
	"[INFO] "};
FILE* log_file = NULL;

int log_init(const char* filename) {
  log_file = fopen(filename, "a");
  if (!log_file) {
    perror("fopen");
    return EXIT_FAILURE;
  }
  fprintf(log_file, "\n");
  fflush(log_file);
  return EXIT_SUCCESS;
}


void log_close(void) {
  if (log_file) {
    fclose(log_file);
  }
}

void log_msg(LogLevel level, const char* msg, ...) {
  if (!log_file) return;
  
  time_t now;
  time(&now);
  char timest[20];
  strftime(timest, sizeof(timest), "%Y-%m-%d %H:%M:%S", localtime(&now));

  va_list args;
  va_start(args, msg);
  fprintf(log_file, "%s %s", timest, levels[level]);
  vfprintf(log_file, msg, args);
  fprintf(log_file, "\n");
  va_end(args);

  fflush(log_file);

  return;
}
