#include "common.h"
#include "config.h"
#include "log.h"
#include "inih/ini.h"

const int logLevelsLen = sizeof(levels) / sizeof(levels[0]); 
Config app_config;

static int handler ( void *user, const char *section, const char *name, const char *value) {
  Config *cfg = (Config *) user;

  if ( strcmp(name,"enable-auth") == 0 ) {
    if ( strcmp(value, "true") == 0 ) {
      cfg->authEnabled = 1;
    } else if ( strcmp(value, "false") == 0 ) {
      cfg->authEnabled = 0;
    } else {
      snprintf(cfg->error, sizeof(cfg->error), 
	      "ERROR : Unknown enable-auth value : %s (allowed : true/false)", value);
      return 0;
    }
    return 1;
  }

  if ( strcmp(name, "log-level") == 0 ) {
   if ( in(levels, logLevelsLen, name) < 0 ) {
      snprintf(cfg->error, sizeof(cfg->error), 
	      "ERROR : Unknown log level value : %s", value);
      return 0;
   }
   cfg->logLevel = in(levels, logLevelsLen, value);
  }


  return 1;
}

int config_init ( const char *configFilePath ) {
   memset(&app_config, 0, sizeof(app_config));
  
  int error_line = ini_parse(configFilePath, handler, &app_config);
  if (error_line < 0) {
    printf("INFO : Config file not found : using default value.\n");
    app_config.authEnabled = 1;
    app_config.logLevel = in(levels, logLevelsLen, "INFO");
  }

  if (error_line > 0) {
    if (strlen(app_config.error) > 0) {
      fprintf(stderr, "Config error (line %d) : %s\n", error_line, app_config.error);
    } else {
      fprintf(stderr, "Unknown error happened in config file at line : %d\n", error_line);
    }
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}

