#include "main.h"

OptionDefinition option_definitions[] = {
    {"leader", "\0", "The leader that will printed when waiting for a command"},
    {"enable_hotkey", "true", "Enable hotkeys"}};

#define OPTCOUNT (sizeof(option_definitions) / sizeof(option_definitions[0]))

CommandDefinition command_definitions[] = {
    {"?", "HELP - Shows helptext", help_handler},
    {"&", "HOTKEY <register: char> <command: string> - Sets a hotkey to a command", hotkey_handler},
    {"@", "RECORD <register: char> <command: string> - Records a command to a register", record_handler},
    {"#", "RECALL <register: char> [register: char] ... - Recalls a command from all register(s) listed, in order", recall_handler},
    {"!", "OPT [opt: word] [value: string] - Sets or prints an option", opt_handler},
    {">", "SAVE [filename: string] - Saves the current script options to a file. Defaults to .clickerrc", save_handler},
    {"<", "LOAD <filename: string> - Loads a script from a file", load_handler},
    {"M", "MOVE <x: int> <y: int> - Moves the mouse to the specified coordinates", move_handler},
    {"C", "CLICK <button: int> - Clicks the button specified", click_handler},
    {"}", "CLICK_DOWN <button: int> - Clicks the button specified", click_down_handler},
    {"{", "CLICK_UP <button: int> - Clicks the button specified", click_up_handler},
    {"K", "KEY <key: char> - Presses the key specified", key_handler},
    {"]", "KEY_DOWN <key: char> - Presses the key specified", key_down_handler},
    {"[", "KEY_UP <key: char> - Presses the key specified", key_up_handler},
    {"S", "SEQUENCE <keys: string> [keys: string] ... - Presses the specified keys in sequence", sequence_handler},
    {"W", "DELAY <ms: int> - Waits for the specified amount of milliseconds", delay_handler},
    {"Q", "QUIT - Quits the program", quit_handler},
};

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
    if (handler(cmd) == 0)
    {
      // printf("Command executed successfully.\n");
    }
    else
    {
      // printf("Error executing command.\n");
    }
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
Register hotkeys[HOTKEYCOUNT]; // Update the size to accommodate all printable ASCII characters

int get_hotkey_index(char hotkey_name)
{
  if (hotkey_name >= ' ' && hotkey_name <= '~')
  {
    return hotkey_name - ' ';
  }
  return -1; // Return -1 if the character is not in the printable ASCII range
}

int hotkey_handler(const Command *cmd)
{
  if (cmd->argc < 3)
  {
    printf("Invalid number of arguments for the HOTKEY command.\n");
    return -1;
  }

  char hotkey_name = cmd->args[1][0];
  int hotkey_index = get_hotkey_index(hotkey_name);
  if (hotkey_index < 0)
  {
    printf("Invalid hotkey name for the HOTKEY command.\n");
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

  hotkeys[hotkey_index].register_name = hotkey_name;
  hotkeys[hotkey_index].command = command;

  printf("Hotkey '%c' set to command: %s", hotkey_name, command);
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
    printf("Invalid number of arguments for the RECORD command.\n");
    return -1;
  }

  char register_name = cmd->args[1][0];
  int register_index = get_register_index(register_name);
  if (register_index < 0)
  {
    printf("Invalid register name for the RECORD command.\n");
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

  registers[register_index].register_name = register_name;
  registers[register_index].command = command;

  printf("Recorded command in register '%c': %s\n", register_name, command);
  return 0;
}

int recall_handler(const Command *cmd)
{
  if (cmd->argc < 2)
  {
    printf("Invalid number of arguments for the RECALL command.\n");
    return -1;
  }

  for (int i = 1; i < cmd->argc; ++i)
  {
    char register_name = cmd->args[i][0];
    int register_index = get_register_index(register_name);
    if (register_index < 0)
    {
      printf("Invalid register name for the RECALL command.\n");
      return -1;
    }

    // Check if there is a command saved in the specified register
    if (registers[register_index].command == NULL)
    {
      printf("No command found in register '%c'\n", register_name);
      return -1;
    }

    // Execute the saved command
    printf("Recalling command in register '%c': %s\n", register_name, registers[register_index].command);
    Command saved_cmd;
    if (parse_command(registers[register_index].command, &saved_cmd) == 0)
    {
      execute_command(&saved_cmd);
    }
  }

  return 0;
}

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

int opt_handler(const Command *cmd)
{
  if (cmd->argc > 3)
  {
    printf("Invalid number of arguments for the OPT command.\n");
    // print out args?
    for (int i = 0; i < cmd->argc; ++i)
    {
      printf("[%s]", cmd->args[i]);
    }
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
      printf("Option %s not found\n", cmd->args[1]);
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
        printf("Option %s set to %s\n", cmd->args[1], cmd->args[2]);
        return 0;
      }
    }
    printf("Option %s not found\n", cmd->args[1]);
  }

  return 0;
}

int save_handler(const Command *cmd)
{
  if (cmd->argc > 3)
  {
    printf("Invalid number of arguments for the SAVE command.\n");
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
    printf("Failed to open file %s\n", filename);
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
    printf("Failed to open file %s\n", filename);
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
    printf("Invalid number of arguments for the LOAD command.\n");
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
  if (cmd->argc != 3)
  {
    printf("Invalid number of arguments for the MOVE command.\n");
    return -1;
  }

  int x = atoi(cmd->args[1]);
  int y = atoi(cmd->args[2]);

  mouseMove(x, y);

  return 0;
}

int click_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the CLICK command.\n");
    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    printf("Invalid button number for the CLICK command.\n");
    return -1;
  }

  mouseDown(button);
  mouseUp(button);
  return 0;
}

int click_down_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the CLICK_DOWN command.\n");
    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    printf("Invalid button number for the CLICK_DOWN command.\n");
    return -1;
  }

  mouseDown(button);
  return 0;
}

int click_up_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the CLICK_UP command.\n");
    return -1;
  }

  int button = atoi(cmd->args[1]);
  if (button != 0 && button != 1 && button != 2)
  {
    printf("Invalid button number for the CLICK_UP command.\n");
    return -1;
  }

  mouseUp(button);
  return 0;
}

int key_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the KEY command.\n");
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
    printf("Invalid number of arguments for the KEY_DOWN command.\n");
    return -1;
  }

  keyDown(cmd->args[1][0]);
  return 0;
}

int key_up_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the KEY_UP command.\n");
    return -1;
  }

  keyUp(cmd->args[1][0]);
  return 0;
}

int sequence_handler(const Command *cmd)
{
  if (cmd->argc < 2)
  {
    printf("Invalid number of arguments for the SEQUENCE command.\n");
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
    printf("Invalid number of arguments for the WAIT command.\n");
    return -1;
  }

  int ms = atoi(cmd->args[1]);
  if (ms < 0)
  {
    printf("Invalid number of milliseconds for the WAIT command.\n");
    return -1;
  }

  usleep(ms * 1000);
  return 0;
}

int quit_handler(const Command *cmd)
{
  if (cmd->argc != 1)
  {
    printf("Invalid number of arguments for the QUIT command.\n");
    return -1;
  }

  exit(0);
  return 0;
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
    for (char key = ' '; key <= '~'; key++)
    {
      if (GetAsyncKeyState(key) & 0x8000)
      {
        int hotkey_index = get_hotkey_index(key);
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
    Sleep(50); // Adjust sleep duration to control polling frequency
  }
#elif defined(__linux__)
  XEvent event;
  XKeyEvent *key_event;

  for (;;)
  {
    char *hotkey_enable;
    get_option_value("enable_hotkey", &hotkey_enable);
    if (hotkey_enable == NULL || strcmp(hotkey_enable, "true") != 0)
    {
      Sleep(100);
      continue;
    }
    XNextEvent(display, &event);
    if (event.type == KeyPress)
    {
      key_event = (XKeyEvent *)&event;
      KeySym keysym = XLookupKeysym(key_event, 0);
      char buf[2];
      int len = XLookupString(key_event, buf, sizeof(buf), &keysym, NULL);
      buf[len] = '\0';
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
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
#elif defined(__linux__)
  init_linux();

  pthread_t thread;
  int err = pthread_create(&thread, NULL, detect_keypresses_thread, registers);
  if (err != 0)
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
#endif

  init_options();

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
      printf("Invalid command. Parsing failed.\n");
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