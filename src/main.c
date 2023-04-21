#include "main.h"

OptionDefinition option_definitions[] = {
    {"quiet", "false", "Whether the program will print feedback after command"},
    {"leader", "\0", "The leader that will printed when waiting for a command"},
    {"enable_cps_register", "false", "Whether the program will store cps into the C register"},
    {"enable_last_location_register", "false", "Whether the program will put the COMMAND for last location of the mouse in the @L register"},
    {"enable_hotkey", "true", "Enable hotkeys"}};

#define OPTCOUNT (sizeof(option_definitions) / sizeof(option_definitions[0]))

Option options[OPTCOUNT];

void init_options()
{
  for (int i = 0; i < OPTCOUNT; ++i)
  {
    Option option = {strdup(option_definitions[i].key), strdup(option_definitions[i].default_value)};
    options[i] = option;
  }
}

void get_option_value(char *key, char **value)
{
  for (int i = 0; i < OPTCOUNT; ++i)
  {
    if (strcmp(options[i].key, key) == 0)
    {
      *value = strdup(options[i].value);
      return;
    }
  }
  *value = strdup("Option not found");
}

CommandDefinition command_definitions[] = {
    {"?", "HELP - Shows helptext", help_handler},
    {"&", "HOTKEY <register: char> <command: string> - Sets a hotkey to a command", hotkey_handler},
    {"@", "RECORD <register: char> <command: string> - Records a command to a register", record_handler},
    {":", "CLONE <register: char> <register: char> - Clones a register to another register", clone_handler},
    {"#", "RECALL <register: char> [register: char] ... - Executes a command from all register(s) listed, in order", recall_handler},
    {"=", "RECALLIF <register: char> <value: string> <register: char> [register: char] ... - Executes a command from all register(s) listed, in order, if the value of the register is equal to the value specified", recallif_handler},
    {"-", "RECALLIFNOT <register: char> <value: string> <register: char> [register: char] ... - Executes a command from all register(s) listed, in order, if the value of the register is not equal to the value specified", recallifnot_handler},
    {"/", "RECALLIFELSE <register: char> <value: string> <register_true: char> <register_false: char> -  Recalls a command from the register specified if the value of the register is equal to the value specified. Otherwise, the command from the register_false will be recalled", recallifelse_handler},
    {"*", "REPEAT <times: int> <register: char> [register: char] ... - Launches a new thread for every register listed to repeat the defined times.", repeat_handler},
    {"^", "WHILE <register: char> <value: string> <register: char> [register: char] ... - Repeats the commands in the register(s) listed, in order, while the value of the register is equal to the value specified", while_handler},
    {"!", "OPT [opt: word] [value: string] - Sets or prints an option", opt_handler},
    {">", "SAVE [filename: string] - Saves the current script options to a file. Defaults to .clickerrc", save_handler},
    {"<", "LOAD <filename: string> - Loads a script from a file", load_handler},
    {"M", "MOVE [x: int] [y: int] - Moves the mouse to the specified coordinates. If nothing is provided, just save the mouse location to the L register if it is enabled", move_handler},
    {"C", "CLICK <button: int> - Clicks the button specified", click_handler},
    {"}", "CLICK_DOWN <button: int> - Clicks the button specified", click_down_handler},
    {"{", "CLICK_UP <button: int> - Clicks the button specified", click_up_handler},
    {"K", "KEY <key: char> - Presses the key specified", key_handler},
    {"]", "KEY_DOWN <key: char> - Presses the key specified", key_down_handler},
    {"[", "KEY_UP <key: char> - Presses the key specified", key_up_handler},
    {"S", "SEQUENCE <keys: string> [keys: string] ... - Presses the specified keys in sequence", sequence_handler},
    {"W", "DELAY <ms: int> - Waits for the specified amount of milliseconds", delay_handler},
    {"P", "PRINT <string: string> - Prints the specified string to the console replaces '@<register: char>' with the value of the register unless escaped", print_handler},
    {"Q", "QUIT - Quits the program", quit_handler},
};

void trim(char *str)
{
  if (str == NULL)
  {
    return;
  }

  int len = strlen(str);
  int start = 0;
  int end = len - 1;

  while (isspace(str[start]))
  {
    start++;
  }

  while (end >= start && isspace(str[end]))
  {
    str[end] = '\0';
    end--;
  }

  if (start > 0)
  {
    for (int i = 0; i <= end - start; i++)
    {
      str[i] = str[start + i];
    }
    str[end - start + 1] = '\0';
  }
}

void quiet_printf(char *format, ...)
{
  char *quiet;
  get_option_value("quiet", &quiet);
  if (strcmp(quiet, "true") == 0)
  {
    return;
  }
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

char *next_token(char **str_ptr, const char *delim)
{
  if (*str_ptr == NULL)
  {
    return NULL;
  }

  char *token_start = *str_ptr;
  char *cur = *str_ptr;
  bool in_quotes = false;

  while (*cur != '\0')
  {
    if (*cur == '\"')
    {
      in_quotes = !in_quotes;
    }
    else if (strchr(delim, *cur) && !in_quotes)
    {
      break;
    }
    cur++;
  }

  if (*cur == '\0')
  {
    *str_ptr = NULL;
  }
  else
  {
    *cur = '\0';
    *str_ptr = cur + 1;
  }

  return token_start;
}

int parse_command(const char *input, Command *cmd)
{
  cmd->argc = 0;

  if (sscanf(input, " %c", &cmd->command) != 1)
  {
    return -1;
  }

  char *input_copy = strdup(input);
  char *cur = input_copy;
  char *token = next_token(&cur, " ");
  while (token != NULL)
  {
    if (*token == '\"')
    {
      token++;
      size_t len = strlen(token);
      if (len > 0 && token[len - 1] == '\"')
      {
        token[len - 1] = '\0';
      }
    }
    cmd->args[cmd->argc++] = strdup(token);
    token = next_token(&cur, " ");
  }

  free(input_copy);

  return 0;
}

void free_command(Command *cmd)
{
  for (int i = 0; i < cmd->argc; ++i)
  {
    free(cmd->args[i]);
  }
}

CommandHandler find_handler(char command)
{
  for (int i = 0; i < sizeof(command_definitions) / sizeof(command_definitions[0]); ++i)
  {
    if (strchr(command_definitions[i].aliases, command) != NULL)
    {
      return command_definitions[i].handler;
    }
  }
  return NULL;
}

void execute_command(Command *cmd)
{
  CommandHandler handler = find_handler(cmd->command);
  if (handler != NULL)
  {
    handler(cmd);
  }
}

int help_handler(const Command *cmd)
{
  for (int i = 0; i < sizeof(command_definitions) / sizeof(command_definitions[0]); ++i)
  {
    printf("%s - %s\n", command_definitions[i].aliases, command_definitions[i].usage);
  }
  return 0;
}

#define HOTKEYCOUNT 95
Register hotkeys[HOTKEYCOUNT];

int get_hotkey_index(char hotkey_name)
{
  if (hotkey_name >= ' ' && hotkey_name <= '~')
  {
    return hotkey_name - ' ';
  }
  return -1;
}

int hotkey_handler(const Command *cmd)
{
  if (cmd->argc < 3)
  {
    quiet_printf("Invalid number of arguments for the HOTKEY command.\n");
    return -1;
  }

  char hotkey_name = cmd->args[1][0];
  int hotkey_index = get_hotkey_index(hotkey_name);
  if (hotkey_index < 0)
  {
    quiet_printf("Invalid hotkey name for the HOTKEY command.\n");
    return -1;
  }

  if (hotkeys[hotkey_index].command)
  {
    free(hotkeys[hotkey_index].command);
  }

  char *command = malloc(1);
  command[0] = '\0';
  for (int i = 2; i < cmd->argc; ++i)
  {
    command = realloc(command, strlen(command) + strlen(cmd->args[i]) + 2);
    strcat(command, cmd->args[i]);
    strcat(command, " ");
  }

  trim(command);
  hotkeys[hotkey_index].register_name = hotkey_name;
  hotkeys[hotkey_index].command = command;

  quiet_printf("Hotkey '%c' set to command: %s\n", hotkey_name, command);
  return 0;
}

#define REGISTERCOUNT 62
Register registers[REGISTERCOUNT];

int get_register_index(char register_name)
{
  if (register_name >= 'a' && register_name <= 'z')
  {
    return register_name - 'a';
  }
  else if (register_name >= 'A' && register_name <= 'Z')
  {
    return 26 + (register_name - 'A');
  }
  else if (register_name >= '0' && register_name <= '9')
  {
    return 52 + (register_name - '0');
  }
  return -1;
}

int record_handler(const Command *cmd)
{
  if (cmd->argc < 3)
  {
    quiet_printf("Invalid number of arguments for the RECORD command.\n");
    return -1;
  }

  char register_name = cmd->args[1][0];
  int register_index = get_register_index(register_name);
  if (register_index < 0)
  {
    quiet_printf("Invalid register name for the RECORD command.\n");
    return -1;
  }

  if (registers[register_index].command)
  {
    free(registers[register_index].command);
  }

  char *command = malloc(1);
  command[0] = '\0';
  for (int i = 2; i < cmd->argc; ++i)
  {
    command = realloc(command, strlen(command) + strlen(cmd->args[i]) + 2);
    strcat(command, cmd->args[i]);
    strcat(command, " ");
  }

  trim(command);
  registers[register_index].register_name = register_name;
  registers[register_index].command = command;

  quiet_printf("Recorded command in register '%c': %s\n", register_name, command);
  return 0;
}

int clone_handler(const Command *cmd)
{
  if (cmd->argc != 3)
  {
    quiet_printf("Invalid number of arguments for the CLONE command.\n");
    return -1;
  }

  int from_register_index = get_register_index(cmd->args[1][0]);
  int to_register_index = get_register_index(cmd->args[2][0]);
  if (from_register_index < 0 || to_register_index < 0)
  {
    quiet_printf("Invalid register name for the CLONE command.\n");
    return -1;
  }

  if (registers[to_register_index].command)
  {
    free(registers[to_register_index].command);
  }

  registers[to_register_index].command = strdup(registers[from_register_index].command);

  quiet_printf("Cloned command from register '%c' to register '%c': %s\n", cmd->args[1][0], cmd->args[2][0], registers[to_register_index].command);

  return 0;
}

int recall_handler(const Command *cmd)
{
  if (cmd->argc < 2)
  {
    quiet_printf("Invalid number of arguments for the RECALL command.\n");
    return -1;
  }

  for (int i = 1; i < cmd->argc; ++i)
  {
    char register_name = cmd->args[i][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      quiet_printf("Invalid register name for the RECALL command.\n");
      return -1;
    }

    if (registers[register_index].command == NULL)
    {
      quiet_printf("No command found in register '%c'\n", register_name);
      return -1;
    }

    quiet_printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
    Command saved_cmd;
    if (parse_command(registers[register_index].command, &saved_cmd) == 0)
    {
      execute_command(&saved_cmd);
    }
  }

  return 0;
}

int recallif_handler(const Command *cmd)
{
  if (cmd->argc < 4)
  {
    quiet_printf("Invalid number of arguments for the RECALLIF command.\n");
    return -1;
  }

  char cond_register_name = cmd->args[1][0];
  int cond_register_index = get_register_index(cond_register_name);
  if (cond_register_index < 0)
  {
    quiet_printf("Invalid register name for the RECALLIF command.\n");
    return -1;
  }

  if (registers[cond_register_index].command == NULL)
  {
    quiet_printf("No value found in register '%c'\n", cond_register_name);
    return -1;
  }

  if (strcmp(registers[cond_register_index].command, cmd->args[2]) == 0)
  {
    for (int i = 3; i < cmd->argc; ++i)
    {
      char register_name = cmd->args[i][0];
      int register_index = get_register_index(register_name);
      if (register_index < 0)
      {
        quiet_printf("Invalid register name for the RECALLIF command.\n");
        return -1;
      }

      if (registers[register_index].command == NULL)
      {
        quiet_printf("No command found in register '%c'\n", register_name);
        return -1;
      }

      quiet_printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
      Command saved_cmd;
      if (parse_command(registers[register_index].command, &saved_cmd) == 0)
      {
        execute_command(&saved_cmd);
      }
    }
  }

  return 0;
}

int recallifnot_handler(const Command *cmd)
{
  if (cmd->argc < 4)
  {
    quiet_printf("Invalid number of arguments for the RECALLIFNOT command.\n");
    return -1;
  }

  char cond_register_name = cmd->args[1][0];
  int cond_register_index = get_register_index(cond_register_name);
  if (cond_register_index < 0)
  {
    quiet_printf("Invalid register name for the RECALLIFNOT command.\n");
    return -1;
  }

  if (registers[cond_register_index].command == NULL)
  {
    quiet_printf("No value found in register '%c'\n", cond_register_name);
    return -1;
  }

  if (strcmp(registers[cond_register_index].command, cmd->args[2]) != 0)
  {
    for (int i = 3; i < cmd->argc; ++i)
    {
      char register_name = cmd->args[i][0];
      int register_index = get_register_index(register_name);
      if (register_index < 0)
      {
        quiet_printf("Invalid register name for the RECALLIFNOT command.\n");
        return -1;
      }

      if (registers[register_index].command == NULL)
      {
        quiet_printf("No command found in register '%c'\n", register_name);
        return -1;
      }

      quiet_printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
      Command saved_cmd;
      if (parse_command(registers[register_index].command, &saved_cmd) == 0)
      {
        execute_command(&saved_cmd);
      }
    }
  }

  return 0;
}

int recallifelse_handler(const Command *cmd)
{
  if (cmd->argc != 5)
  {
    quiet_printf("Invalid number of arguments for the RECALLIFELSE command.\n");
    return -1;
  }

  char cond_register_name = cmd->args[1][0];
  int cond_register_index = get_register_index(cond_register_name);
  if (cond_register_index < 0)
  {
    quiet_printf("Invalid register name for the RECALLIFELSE command.\n");
    return -1;
  }

  if (registers[cond_register_index].command == NULL)
  {
    quiet_printf("No value found in register '%c'\n", cond_register_name);
    return -1;
  }

  if (strcmp(registers[cond_register_index].command, cmd->args[2]) == 0)
  {
    char register_name = cmd->args[3][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      quiet_printf("Invalid register name for the RECALLIFELSE command.\n");
      return -1;
    }

    if (registers[register_index].command == NULL)
    {
      quiet_printf("No command found in register '%c'\n", register_name);
      return -1;
    }

    quiet_printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
    Command saved_cmd;
    if (parse_command(registers[register_index].command, &saved_cmd) == 0)
    {
      execute_command(&saved_cmd);
    }
  }
  else
  {
    char register_name = cmd->args[4][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      quiet_printf("Invalid register name for the RECALLIFELSE command.\n");
      return -1;
    }

    if (registers[register_index].command == NULL)
    {
      quiet_printf("No command found in register '%c'\n", register_name);
      return -1;
    }

    quiet_printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
    Command saved_cmd;
    if (parse_command(registers[register_index].command, &saved_cmd) == 0)
    {
      execute_command(&saved_cmd);
    }
  }

  return 0;
}

typedef struct
{
  char **commands;
  int commandc;
  int times;
} RepeatCommand;

#ifdef _WIN32
DWORD WINAPI repeat_thread(LPVOID arg)
#elif defined(__linux__)
void *repeat_thread(void *arg)
#endif
{
  RepeatCommand *r_cmd = (RepeatCommand *)arg;
  Command saved_cmd[r_cmd->commandc];
  for (int i = 0; i < r_cmd->commandc; ++i)
  {
    if (parse_command(r_cmd->commands[i], &saved_cmd[i]) != 0)
    {
      quiet_printf("Invalid command in REPEAT command: %s\n", r_cmd->commands[i]);
      free(r_cmd->commands);
      free(r_cmd);
      return -1;
    }
  }

  for (int i = 0; i < r_cmd->times; ++i)
  {
    for (int j = 0; j < r_cmd->commandc; ++j)
    {
      execute_command(&saved_cmd[j]);
    }
  }
  free(r_cmd->commands);
  free(r_cmd);
  return 0;
}

int repeat_handler(const Command *cmd)
{
  if (cmd->argc < 3)
  {
    quiet_printf("Invalid number of arguments for the REPEAT command.\n");
    return -1;
  }

  int times = atoi(cmd->args[1]);
  if (times <= 0)
  {
    quiet_printf("Invalid number of times for the REPEAT command.\n");
    return -1;
  }

  RepeatCommand *repeatCommand = (RepeatCommand *)malloc(sizeof(RepeatCommand));

  repeatCommand->commandc = cmd->argc - 2;
  repeatCommand->commands = (char **)malloc(sizeof(char *) * repeatCommand->commandc);

  for (int i = 2, j = 0; i < cmd->argc; ++i, ++j)
  {
    char register_name = cmd->args[i][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      quiet_printf("Invalid register name for the REPEAT command.\n");
      return -1;
    }

    if (registers[register_index].command == NULL)
    {
      quiet_printf("No command found in register '%c'\n", register_name);
      return -1;
    }

    repeatCommand->times = times;
    repeatCommand->commands[j] = strdup(registers[register_index].command);
  }

#ifdef _WIN32
  HANDLE new_thread;
  new_thread = CreateThread(NULL, 0, repeat_thread, repeatCommand, 0, NULL);
  if (new_thread == NULL)
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
  CloseHandle(new_thread);
#elif defined(__linux__)
  init_linux();

  pthread_t thread;
  int err = pthread_create(&thread, NULL, repeat_thread, repeatCommand);
  if (err != 0)
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
  pthread_detach(thread);
#endif

  return 0;
}

typedef struct
{
  char **commands;
  int commandc;
  char reg;
  char *value;
} WhileCommand;

#ifdef _WIN32
DWORD WINAPI while_thread(LPVOID arg)
#elif defined(__linux__)
void *while_thread(void *arg)
#endif
{
  WhileCommand *w_cmd = (WhileCommand *)arg;
  Command *saved_cmds = malloc(sizeof(Command) * w_cmd->commandc);
  for (int i = 0; i < w_cmd->commandc; ++i)
  {
    if (parse_command(w_cmd->commands[i], &saved_cmds[i]) != 0)
    {
      free(saved_cmds);
      free(w_cmd->commands);
      free(w_cmd->value);
      free(w_cmd);
      return -1;
    }
  }

  free(w_cmd->commands);
  int register_index = get_register_index(w_cmd->reg);
  for (;;)
  {
    for (int i = 0; i < w_cmd->commandc; ++i)
    {
      if (registers[register_index].command == NULL || strcmp(registers[register_index].command, w_cmd->value) != 0)
      {
        break;
      }
      execute_command(&saved_cmds[i]);
    }
  }
  free(saved_cmds);
  free(w_cmd->value);
  free(w_cmd);
  return 0;
}

int while_handler(const Command *cmd)
{
  if (cmd->argc < 4)
  {
    quiet_printf("Invalid number of arguments for the WHILE command.\n");
    return -1;
  }

  WhileCommand *whileCommand = (WhileCommand *)malloc(sizeof(WhileCommand));

  whileCommand->commandc = cmd->argc - 3;
  whileCommand->commands = (char **)malloc(sizeof(char *) * whileCommand->commandc);

  whileCommand->reg = cmd->args[1][0];
  whileCommand->value = strdup(cmd->args[2]);

  for (int i = 3, j = 0; i < cmd->argc; ++i, ++j)
  {
    char register_name = cmd->args[i][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      quiet_printf("Invalid register name for the WHILE command.\n");
      free(whileCommand->commands);
      free(whileCommand);
      return -1;
    }

    if (registers[register_index].command == NULL)
    {
      quiet_printf("No command found in register '%c'\n", register_name);
      free(whileCommand->commands);
      free(whileCommand);
      return -1;
    }

    whileCommand->commands[j] = strdup(registers[register_index].command);
  }

#ifdef _WIN32
  HANDLE new_thread;
  new_thread = CreateThread(NULL, 0, while_thread, whileCommand, 0, NULL);
  if (new_thread == NULL)
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
  CloseHandle(new_thread);
#elif defined(__linux__)
  init_linux();

  pthread_t thread;
  int err = pthread_create(&thread, NULL, while_thread, whileCommand);
  if (err != 0)
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
  pthread_detach(thread);
#endif
  return 0;
}

int opt_handler(const Command *cmd)
{
  if (cmd->argc > 3)
  {
    quiet_printf("Invalid number of arguments for the OPT command.\n");
    return -1;
  }

  if (cmd->argc == 1)
  {
    for (int i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
    {
      printf("%s = %s\n", options[i].key, options[i].value);
    }
  }
  else if (cmd->argc == 2)
  {
    char *value;
    get_option_value(cmd->args[1], &value);
    if (strcmp(value, "Option not found") == 0)
    {
      quiet_printf("Option %s not found\n", cmd->args[1]);
      return -1;
    }
    printf("%s = %s\n", cmd->args[1], value);
    free(value);
  }
  else if (cmd->argc == 3)
  {
    for (int i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
    {
      if (strcmp(options[i].key, cmd->args[1]) == 0)
      {
        free(options[i].value);
        options[i].value = strdup(cmd->args[2]);
        quiet_printf("Option %s set to %s\n", cmd->args[1], cmd->args[2]);
        return 0;
      }
    }
    quiet_printf("Option %s not found\n", cmd->args[1]);
  }

  return 0;
}

int save_handler(const Command *cmd)
{
  if (cmd->argc > 3)
  {
    quiet_printf("Invalid number of arguments for the SAVE command.\n");
    return -1;
  }

  char *filename;

  if (cmd->argc == 1)
  {
    filename = DOTFILE;
  }
  else
  {
    filename = cmd->args[1];
  }

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    quiet_printf("Failed to open file %s\n", filename);
    return -1;
  }

  for (int i = 0; i < OPTCOUNT; ++i)
  {
    fprintf(fp, "! %s %s\n", options[i].key, options[i].value);
  }

  for (int i = 0; i < REGISTERCOUNT; ++i)
  {
    if (registers[i].command != NULL)
    {
      fprintf(fp, "@ %c %s\n", registers[i].register_name, registers[i].command);
    }
  }

  for (int i = 0; i < HOTKEYCOUNT; ++i)
  {
    if (hotkeys[i].command != NULL)
    {
      fprintf(fp, "& %s %s\n", hotkeys[i].register_name, hotkeys[i].command);
    }
  }

  fclose(fp);

  return 0;
}

void execute_file(char *filename)
{
  FILE *fp = fopen(filename, "r");

  if (fp == NULL)
  {
    quiet_printf("Failed to open file %s\n", filename);
    return;
  }

  char buf[256];

  while (fgets(buf, 256, fp))
  {
    buf[strcspn(buf, "\r\n")] = 0;
    Command cmd;
    if (parse_command(buf, &cmd) == 0)
    {
      execute_command(&cmd);
    }
  }
  fclose(fp);
}

int load_handler(const Command *cmd)
{
  if (cmd->argc > 3)
  {
    quiet_printf("Invalid number of arguments for the LOAD command.\n");
    return -1;
  }

  char *filename;

  if (cmd->argc == 1)
  {
    filename = DOTFILE;
  }
  else
  {
    filename = cmd->args[1];
  }

  execute_file(filename);

  return 0;
}

int move_handler(const Command *cmd)
{
  if (cmd->argc != 3 && cmd->argc != 1)
  {
    quiet_printf("Invalid number of arguments for the MOVE command.\n");
    return -1;
  }

  char *enable_last_location_register;
  get_option_value("enable_last_location_register", &enable_last_location_register);
  if (strcmp(enable_last_location_register, "true") == 0)
  {
    int index = get_register_index('L');
    if (index != -1)
    {
      free(registers[index].command);
      MousePos pos = getMousePos();
      char *command = malloc(sizeof(char) * (strlen("M 00000 00000") + 1));
      sprintf(command, "M %d %d", pos.x, pos.y);
      registers[index].command = strdup(command);
      free(command);
    }
  }

  if (cmd->argc == 3)
  {
    int x = atoi(cmd->args[1]);
    int y = atoi(cmd->args[2]);

    mouseMove(x, y);
  }

  return 0;
}

int clicks_last_second = 0;
int clicks_tally = 0;

#ifdef _WIN32
DWORD WINAPI click_tally_thread(LPVOID arg)
#elif defined(__linux__)
void *click_tally_thread(void *arg)
#endif
{
  for (;;)
  {
    clicks_last_second = clicks_tally;
    clicks_tally = 0;
    char *enable_cps_register;
    get_option_value("enable_cps_register", &enable_cps_register);
    if (strcmp(enable_cps_register, "true") == 0)
    {
      int index = get_register_index('C');
      if (index != -1)
      {
        free(registers[index].command);
        registers[index].command = malloc(4);
        sprintf(registers[index].command, "%d", clicks_last_second);
      }
    }
    Sleep(1000);
  }
  return 0;
}

int click_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the CLICK command.\n");

    for (int i = 0; i < cmd->argc; ++i)
    {
      quiet_printf("cmd->args[%d] = %s\n", i, cmd->args[i]);
    }

    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    quiet_printf("Invalid button number for the CLICK command.\n");
    return -1;
  }

  mouseDown(button);
  mouseUp(button);

  clicks_tally++;

  return 0;
}

int click_down_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the CLICK_DOWN command.\n");
    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    quiet_printf("Invalid button number for the CLICK_DOWN command.\n");
    return -1;
  }

  mouseDown(button);
  return 0;
}

int click_up_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the CLICK_UP command.\n");
    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    quiet_printf("Invalid button number for the CLICK_UP command.\n");
    return -1;
  }

  mouseUp(button);
  return 0;
}

int key_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the KEY command.\n");
    return -1;
  }

  keyDown(cmd->args[1][0]);
  keyUp(cmd->args[1][0]);
  return 0;
}

int key_down_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the KEY_DOWN command.\n");
    return -1;
  }

  keyDown(cmd->args[1][0]);
  return 0;
}

int key_up_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the KEY_UP command.\n");
    return -1;
  }

  keyUp(cmd->args[1][0]);
  return 0;
}

int sequence_handler(const Command *cmd)
{
  if (cmd->argc < 2)
  {
    quiet_printf("Invalid number of arguments for the SEQUENCE command.\n");
    return -1;
  }

  for (int i = 1; i < cmd->argc; ++i)
  {
    for (int j = 0; j < strlen(cmd->args[i]); ++j)
    {
      keyDown(cmd->args[i][j]);
      keyUp(cmd->args[i][j]);
    }
    if (cmd->argc > 2 && i < cmd->argc - 1)
    {
      keyDown(' ');
      keyUp(' ');
    }
  }
  return 0;
}

int delay_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the WAIT command.\n");
    return -1;
  }

  int ms = atoi(cmd->args[1]);
  if (ms < 0)
  {
    quiet_printf("Invalid number of milliseconds for the WAIT command.\n");
    return -1;
  }

  usleep(ms * 1000);
  return 0;
}

void find_register_references(const char *input, char **output)
{
  int i = 0;
  int j = 0;

  while (input[i] != '\0')
  {
    if (input[i] == '@' && isalpha(input[i + 1]))
    {
      (*output)[j++] = input[i + 1];
      i += 2;
    }
    else
    {
      i++;
    }
  }
  (*output)[j] = '\0';
}

void replace_register_references(const char *input, char **output, Register *registers)
{
  find_register_references(input, output);

  char *temp = (char *)malloc(512);
  if (temp == NULL)
  {
    printf("Memory allocation failed.\n");
    return;
  }

  const char *src = input;
  char *dst = temp;

  while (*src != '\0')
  {
    if (*src == '@' && get_register_index(src[1]) != -1)
    {
      int index = get_register_index(src[1]);

      if (index == -1)
      {
        break;
      }

      char *value = registers[index].command;
      if (value == NULL)
      {
        break;
      }

      strcpy(dst, value);
      dst += strlen(value);
      src += 2;
    }
    else
    {
      *dst++ = *src++;
    }
  }

  *dst = '\0';
  strcpy(*output, temp);
  free(temp);
}

int print_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    quiet_printf("Invalid number of arguments for the PRINT command.\n");

    return -1;
  }

  char *input = malloc(1);
  input[0] = '\0';
  for (int i = 1; i < cmd->argc; ++i)
  {
    input = realloc(input, strlen(input) + strlen(cmd->args[i]) + 2);
    strcat(input, cmd->args[i]);
    strcat(input, " ");
  }

  trim(input);

  int max_output_size = 512;

  char *output = (char *)malloc(max_output_size * sizeof(char));
  if (output == NULL)
  {
    quiet_printf("Memory allocation failed in PRINT.\n");
    return 1;
  }

  replace_register_references(input, &output, registers);

  printf("%s\n", output);

  free(input);
  free(output);

  return 0;
}

int quit_handler(const Command *cmd)
{
  if (cmd->argc != 1)
  {
    quiet_printf("Invalid number of arguments for the QUIT command.\n");
    return -1;
  }

  exit(0);
  return 0;
}

char shift_char(char actual_key, short shiftState)
{
  if (actual_key >= 'A' && actual_key <= 'Z' && !shiftState)
  {
    return actual_key + ('a' - 'A');
  }

  if (shiftState)
  {
    int shifted_ascii;
    switch (actual_key)
    {
    case '1':
      shifted_ascii = '!';
      break;
    case '2':
      shifted_ascii = '@';
      break;
    case '3':
      shifted_ascii = '#';
      break;
    case '4':
      shifted_ascii = '$';
      break;
    case '5':
      shifted_ascii = '%';
      break;
    case '6':
      shifted_ascii = '^';
      break;
    case '7':
      shifted_ascii = '&';
      break;
    case '8':
      shifted_ascii = '*';
      break;
    case '9':
      shifted_ascii = '(';
      break;
    case '0':
      shifted_ascii = ')';
      break;
    case '-':
      shifted_ascii = '_';
      break;
    case '=':
      shifted_ascii = '+';
      break;
    case '[':
      shifted_ascii = '{';
      break;
    case ']':
      shifted_ascii = '}';
      break;
    case '\\':
      shifted_ascii = '|';
      break;
    case ';':
      shifted_ascii = ':';
      break;
    case '\'':
      shifted_ascii = '"';
      break;
    case ',':
      shifted_ascii = '<';
      break;
    case '.':
      shifted_ascii = '>';
      break;
    case '/':
      shifted_ascii = '?';
      break;
    case '`':
      shifted_ascii = '~';
      break;
    default:
      shifted_ascii = actual_key;
      break;
    }
    return (char)shifted_ascii;
  }
  else
  {
    return actual_key;
  }
}

void detect_keypresses()
{
#ifdef _WIN32
  for (;;)
  {
    char *hotkey_enable;
    get_option_value("enable_hotkey", &hotkey_enable);
    if (hotkey_enable == NULL || strcmp(hotkey_enable, "true") != 0)
    {
      Sleep(100);
      continue;
    }

    short shiftState = GetAsyncKeyState(VK_SHIFT) & 0x8000;

    for (char key = ' '; key <= '~'; key++)
    {
      short keyState = GetAsyncKeyState(key);
      if (keyState & 0x8000)
      {
        char actual_key = shift_char(key, shiftState);

        int hotkey_index = get_hotkey_index(actual_key);
        if (hotkey_index != -1 && hotkeys[hotkey_index].command != NULL)
        {
          Command saved_cmd;
          if (parse_command(hotkeys[hotkey_index].command, &saved_cmd) == 0)
          {
            execute_command(&saved_cmd);
          }
        }
      }
    }

    Sleep(50);
  }
#elif defined(__linux__)
  XSelectInput(get_display(), get_window(), KeyPressMask);

  XEvent ev;
  KeySym ks;
  char buf[32];
  Status status;

  for (;;)
  {
    char *hotkey_enable;
    get_option_value("enable_hotkey", &hotkey_enable);
    if (hotkey_enable == NULL || strcmp(hotkey_enable, "true") != 0)
    {
      Sleep(100);
      continue;
    }
    XNextEvent(get_display(), &ev);
    if (ev.type == KeyPress)
    {
      XKeyPressedEvent *kev = (XKeyPressedEvent *)&ev;
      int len = Xutf8LookupString(get_input_context(), kev, buf, sizeof(buf), &ks, &status);
      if (len > 0)
      {
        int hotkey_index = get_hotkey_index(buf[0]);
        if (hotkey_index != -1)
        {
          Command saved_cmd;
          if (parse_command(hotkeys[hotkey_index].command, &saved_cmd) == 0)
          {
            execute_command(&saved_cmd);
          }
        }
      }
    }
  }
#endif
}

#ifdef _WIN32
DWORD WINAPI detect_keypresses_thread(LPVOID arg)
#elif defined(__linux__)
void *detect_keypresses_thread(void *arg)
#endif
{
  detect_keypresses();
  return 0;
}

int main()
{
#ifdef _WIN32
  HANDLE hotkey_thread;
  hotkey_thread = CreateThread(NULL, 0, detect_keypresses_thread, NULL, 0, NULL);
  if (hotkey_thread == NULL)
  {
    fprintf(stderr, "Error creating hotkey_thread\n");
    return 1;
  }
  HANDLE cps_thread;
  cps_thread = CreateThread(NULL, 0, click_tally_thread, NULL, 0, NULL);
  if (cps_thread == NULL)
  {
    fprintf(stderr, "Error creating cps_thread\n");
    return 1;
  }

#elif defined(__linux__)
  init_linux();

  pthread_t hotkey_thread;
  int err = pthread_create(&hotkey_thread, NULL, detect_keypresses_thread, registers);
  if (err != 0)
  {
    fprintf(stderr, "Error creating hotkey_thread\n");
    return 1;
  }
  pthread_t cps_thread;
  err = pthread_create(&cps_thread, NULL, click_tally_thread, NULL);
  if (err != 0)
  {
    fprintf(stderr, "Error creating cps_thread\n");
    return 1;
  }
#endif

  init_options();

  if (access(DOTFILE, F_OK) != -1)
  {
    execute_file(DOTFILE);
  }

  char input[256];
  Command cmd = {0};

  char *leader;
  get_option_value("leader", &leader);

  printf(leader);

  while (fgets(input, sizeof(input), stdin) != NULL)
  {
    input[strcspn(input, "\r\n")] = 0;

    if (strncmp(input, leader, strlen(leader)) == 0)
    {
      memmove(input, input + strlen(leader), strlen(input) - strlen(leader) + 1);
    }

    if (parse_command(input, &cmd) == 0)
    {
      execute_command(&cmd);
    }
    else
    {
      quiet_printf("Invalid command. Parsing failed.\n");
    }
    free_command(&cmd);

    get_option_value("leader", &leader);
    printf(leader);
  }

  free(leader);

#ifdef _WIN32
  WaitForSingleObject(hotkey_thread, INFINITE);
  CloseHandle(hotkey_thread);
#elif defined(__linux__)
  pthread_join(thread, NULL);
  cleanup_linux();
#endif

  return 0;
}