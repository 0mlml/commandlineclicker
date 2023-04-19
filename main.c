#include "main.h"

OptionDefinition option_defintions[] = {
    {"leader", "", "The leader that will printed when waiting for a command"},
    {"opt2", "default2", "Option 2 description"},
};

CommandDefinition command_definitions[] = {
    {"?,HELP", "HELP - Shows helptext", help_handler},
    {"@,RECORD", "RECORD <register: char> <command: string> - Records a command to a register", record_handler},
    {"#,RECALL", "RECALL <register: char> - Recalls a command from a register", recall_handler},
    {"!,OPT", "OPT [opt: word] [value: string] - Sets or prints an option", opt_handler},
    {">,SAVE", "SAVE <filename: string> - Saves the current script to a file", save_handler},
    {"<,LOAD", "LOAD <filename: string> - Loads a script from a file", load_handler},
    {"C,CLICK", "CLICK <button: int> - Clicks the button specified", click_handler},
    {"},CLICK_DOWN", "CLICK_DOWN <button: int> - Clicks the button specified", click_down_handler},
    {"{,CLICK_UP", "CLICK_UP <button: int> - Clicks the button specified", click_up_handler},
    {"K,KEY", "KEY <key: char> - Presses the key specified", key_handler},
    {"],KEY_DOWN", "KEY_DOWN <key: char> - Presses the key specified", key_down_handler},
    {"[,KEY_UP", "KEY_UP <key: char> - Presses the key specified", key_up_handler},
};

int parse_command(const char *input, Command *cmd)
{
  cmd->argc = 0;

  if (sscanf(input, " %c", &cmd->command) != 1)
  {
    return -1;
  }

  char *input_copy = strdup(input);
  char *token = strtok(input_copy, " ");
  while (token != NULL)
  {
    cmd->args[cmd->argc++] = strdup(token);
    token = strtok(NULL, " ");
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
  for (int i = 0;
       i < sizeof(command_definitions) / sizeof(command_definitions[0]); ++i)
  {
    if (command_definitions[i].aliases[0] == command)
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
      printf("Command executed successfully.\n");
    }
    else
    {
      printf("Error executing command.\n");
    }
  }
  else
  {
    printf("Invalid command.\n");
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

Register registers[62];

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
  if (cmd->argc != 3)
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

  // Free previous command memory if it exists
  if (registers[register_index].command)
  {
    free(registers[register_index].command);
  }

  // Save the command in the specified register
  registers[register_index].register_name = register_name;
  registers[register_index].command = strdup(cmd->args[2]);

  printf("Recorded command in register '%c': %s\n", register_name, cmd->args[2]);
  return 0;
}

int recall_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the RECALL command.\n");
    return -1;
  }

  char register_name = cmd->args[1][0];
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
  parse_command(registers[register_index].command, &saved_cmd);
  execute_command(&saved_cmd);

  return 0;
}

Option options[sizeof(option_defintions)];

void init_options()
{
  for (int i = 0; i < sizeof(option_defintions) / sizeof(option_defintions[0]); ++i)
  {
    Option option = {strdup(option_defintions[i].key), strdup(option_defintions[i].default_value)};
    options[i] = option;
  }
}

void get_option_value(char *key, char **value)
{
  for (int i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
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
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the SAVE command.\n");
    return -1;
  }

  printf("Save command\n");
  return 0;
}

int load_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the LOAD command.\n");
    return -1;
  }

  printf("Load command\n");
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

int type_handler(const Command *cmd)
{
  if (cmd->argc != 2)
  {
    printf("Invalid number of arguments for the TYPE command.\n");
    return -1;
  }

  for (int i = 0; i < strlen(cmd->args[1]); ++i)
  {
    keyDown(cmd->args[1][i]);
    keyUp(cmd->args[1][i]);
  }
  return 0;
}

int main()
{
#ifdef __linux__
  init_linux();
#endif

  init_options();

  char input[256];
  Command cmd;

  char *leader;
  get_option_value("leader", &leader);

  printf(leader);

  while (fgets(input, sizeof(input), stdin) != NULL)
  {
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
      printf("Invalid command.\n");
    }
    free_command(&cmd);
  }

#ifdef __linux__
  cleanup_linux();
#endif

  return 0;
}