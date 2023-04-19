#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mkb.h"

typedef struct
{
  char command;
  char *args[3];
  int argc;
} Command;

typedef int (*CommandHandler)(const Command *cmd);

typedef struct
{
  const char *aliases;
  const char *usage;
  CommandHandler handler;
} CommandDefinition;

int help_handler(const Command *cmd);
int record_handler(const Command *cmd);
int recall_handler(const Command *cmd);
int opt_handler(const Command *cmd);
int save_handler(const Command *cmd);
int load_handler(const Command *cmd);
int click_handler(const Command *cmd);
int click_down_handler(const Command *cmd);
int click_up_handler(const Command *cmd);
int key_handler(const Command *cmd);
int key_down_handler(const Command *cmd);
int key_up_handler(const Command *cmd);
int type_handler(const Command *cmd);

typedef struct
{
  char *key;
  char *value;
} Option;

typedef struct
{
  const char *key;
  const char *default_value;
  const char *description;
} OptionDefinition;

typedef struct
{
  char register_name;
  char *command;
} Register;