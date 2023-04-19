# Command Line Clicker README

This code is for a command line based clicker, which allows users to record and recall sequences of mouse and keyboard clicks. Here's a brief overview of the available options and commands:

## Option Definitions
The following option definitions are available:

| Option Name   | Default Value | Description |
| ------------- | ------------- | ----------- |
| leader        | >             | The leader that will be printed when waiting for a command |
| enable_hotkey | true		   | Whether or not hotkeys are enabled |

## Command Definitions
The following command definitions are available:

| Command | Usage | Description |
| ------- | ----- | ----------- |
| ?       | HELP - Shows helptext | Shows the list of available commands and their descriptions |
| &       | HOTKEY \<register: char> \<command: string> - Sets a hotkey to a command | Sets a hotkey to a command. Whenever the user types the specified character, the command will be executed |
| @       | RECORD \<register: char> \<command: string> - Records a command to a register | Records a command to a register. Whenever the user types the specified character, the recorded command will be executed |
| #       | RECALL \<register: char> [register: char] ... - Recalls a command from all register(s) listed, in order | Recalls a command from all specified registers, in the order they were listed |
| !       | OPT [opt: word] [value: string] - Sets or prints an option | Sets or prints the value of the specified option |
| >       | SAVE [filename: string] - Saves the current script options, registers, and hotkeys to a file. Defaults to .clickerrc | Saves the current script options to a file. If no filename is specified, the default filename ".clickerrc" will be used |
| <       | LOAD \<filename: string> - Loads a script from a file | Loads a script from a file |
| C       | CLICK \<button: int> - Clicks the button specified | Clicks the specified mouse button |
| }       | CLICK_DOWN \<button: int> - Clicks the button specified | Presses and holds the specified mouse button |
| {       | CLICK_UP \<button: int> - Clicks the button specified | Releases the specified mouse button |
| K       | KEY \<key: char> - Presses the key specified | Presses the specified key |
| ]       | KEY_DOWN \<key: char> - Presses the key specified | Presses and holds the specified key |
| [       | KEY_UP \<key: char> - Presses the key specified | Releases the specified key |
| S       | SEQUENCE \<keys: string> [keys: string] ... - Presses the specified keys in sequence | Presses the specified keys in the order they were listed |
| W       | DELAY \<ms: int> - Waits for the specified amount of milliseconds | Waits for the specified amount of time, in milliseconds |
| Q       | QUIT - Quits the program | Exits the program |

Please refer to the source code for more detailed information about the implementation of each option and command.
