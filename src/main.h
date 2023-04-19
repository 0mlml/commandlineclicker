#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mkb.h"

#ifdef _WIN32
#define HOME getenv("USERPROFILE")

#define THREAD_FUNC_RETURN_TYPE DWORD WINAPI
#define THREAD_FUNC_ARG_TYPE LPVOID
#elif define(__linux__)
#define HOME getenv("HOME")

#include <pthread.h>
#include <unistd.h>
#define THREAD_FUNC_RETURN_TYPE void *
#define THREAD_FUNC_ARG_TYPE void *
#endif

#define DOTFILE ".clickerrc"

typedef struct
{
  char command;
  char *args[16];
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
int hotkey_handler(const Command *cmd);
int record_handler(const Command *cmd);
int recall_handler(const Command *cmd);
int recallif_handler(const Command *cmd);
int repeat_handler(const Command *cmd);
int opt_handler(const Command *cmd);
int save_handler(const Command *cmd);
int load_handler(const Command *cmd);
int move_handler(const Command *cmd);
int click_handler(const Command *cmd);
int click_down_handler(const Command *cmd);
int click_up_handler(const Command *cmd);
int key_handler(const Command *cmd);
int key_down_handler(const Command *cmd);
int key_up_handler(const Command *cmd);
int sequence_handler(const Command *cmd);
int delay_handler(const Command *cmd);
int quit_handler(const Command *cmd);

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