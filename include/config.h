#ifndef CHATSERVER_CONFIG_H
#define CHATSERVER_CONFIG_H

typedef struct {
  unsigned int logLevel;
  int authEnabled;
  char error[256];
} Config;

extern Config app_config;

int config_init ( const char *configFilePath );

#endif /* CHATSERVER_CONFIG_H */
