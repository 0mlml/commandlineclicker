# Command Line Clicker README

This code is for a command line based clicker, which allows users to record and recall sequences of mouse and keyboard clicks. Here's a brief overview of the available options and commands:

## Option Definitions
The following option definitions are available:

| Option Name         | Default Value | Description |
| ------------------- | ------------- | ----------- |
| leader              |               | The leader that will be printed when waiting for a command |
| enable_hotkey       | true		  | Whether or not hotkeys are enabled |
| enable_cps_register | false         | Whether the program will put the CPS in the @C register |
| quiet 	          | false         | Whether the program will print feedback after command |


## Command Definitions
The following command definitions are available:

| Command | Usage | Description |
| ------- | ----- | ----------- |
| ?       | HELP - Shows helptext | Shows the list of available commands and their descriptions |
| &       | HOTKEY \<register: char> \<command: string>  | Sets a hotkey to a command. Whenever the user types the specified character, the command will be executed |
| @       | RECORD \<register: char> \<command: string> | Records a command to a register. Whenever the user types the specified character, the recorded command will be executed |
| #       | RECALL \<register: char> [register: char] ... | Recalls a command from all specified registers, in the order they were listed |
| =       | RECALLIF \<register: char> \<value: string> \<register: char> [register: char] ...| Recalls a command from all register(s) listed, in order, if the value of the register is equal to the value specified |
| -       | RECALLIFNOT \<register: char> \<value: string> \<register: char> [register: char] ...| Recalls a command from all register(s) listed, in order, if the value of the register is not equal to the value specified |
| /       | RECALLIFELSE \<register: char> \<value: string> \<register_true: char> \<register_false: char> | Recalls a command from the register specified if the value of the register is equal to the value specified. Otherwise, the command from the register_false will be recalled |
| *       | REPEAT <times: int> <register: char> [register: char] ... | Launches a new thread for every register listed to repeat the defined times.  |
| ^       | WHILE \<register: char> \<value: string> \<register: char> [register: char] ... | Repeats the commands in the register(s) listed, in order, while the value of the register is equal to the value specified |
| !       | OPT [opt: word] [value: string] | Sets or prints the value of the specified option |
| >       | SAVE [filename: string] | Saves the current script options to a file. If no filename is specified, the default filename ".clickerrc" will be used |
| <       | LOAD \<filename: string> | Loads a script from a file |
| M		  | MOVE \<x: int> \<y: int> | Moves the mouse to the specified coordinates |
| C       | CLICK \<button: int>| Clicks the specified mouse button |
| }       | CLICK_DOWN \<button: int> | Presses and holds the specified mouse button |
| {       | CLICK_UP \<button: int>  | Releases the specified mouse button |
| K       | KEY \<key: char>  | Presses the specified key |
| ]       | KEY_DOWN \<key: char> | Presses and holds the specified key |
| [       | KEY_UP \<key: char>  | Releases the specified key |
| S       | SEQUENCE \<keys: string> [keys: string] ... | Presses the specified keys in the order they were listed |
| W       | DELAY \<ms: int> | Waits for the specified amount of time, in milliseconds |
| P 	  | PRINT \<string: string> | Prints the specified string to the console replaces '@<register: char>' with the value of the register unless escaped|
| Q       | QUIT | Exits the program |
| . 	  | COMMENT | This symbol will be reserved as a no-op |

### Some notes:
 - The RECORD command does not necessarily have to assign a command. It can also be used to assign a value to a register. For example, `@ s 0` will set the value of the register `s` to `0`, though this is just treated as a string, as there is no arithmetic operations.
 - The RECORD and HOTKEY commands have different registers. This means that `@ s 0` and `& s 0` will not conflict with each other.
 - The size of the RECORD registers is a-z, A-Z, and 0-9. The size of the HOTKEY registers is the entire ASCII range.
 - Assigning a hotkey to a capital letter will require SHIFT + KEY, while a lowercase letter simply requires KEY.
 - You cannot chain commands together on one line, but you can assign comands to registers and recall them to a single line.
 - WHILE, REPEAT, and HOTKEY (detection) run on separate threads. This means that they will not block the main thread, and will not interfere with each other.

Please refer to the source code for more detailed information about the implementation of each option and command.

## Auto-load
If there is a `.clickerrc` file in the current directory, it will be loaded automatically when the program is started.

Here is an example of one:
```
. Set options to preferences
! quiet true
! leader >
! enable_hotkey true

. Panic hotkey
& q Q

. Load a script
< clicker
```

## Examples
Here are some examples of scripts. The can all be saved to a file and loaded with the `< <filename>` command.:
```
. Two-hotkey autoclicker
! quiet true
! enable_hotkey true

. Register for clicking the left mouse button
@ c C 0

. Register for autoclicker state: 0 = off, 1 = on
@ s 0

. Register for autoclicker loop
@ a = s 1 c

. Hotkey for toggling the autoclicker on
@ t @ s 1
& o = s 0 t

. Hotkey for toggling the autoclicker off
@ u @ s 0
& p = s 1 u

. Loop to constantly check the state and execute the autoclicker
@ r 0
^ r 0 a
```
```
. Two-hotkey autoclicker (Alternative, more optimized and compact)
! quiet true
! enable_hotkey true

. Register for autoclicker state: 0 = off, 1 = on
@ s 0
. Autoclicker
@ c C 0
@ n ^ s 1 c

. Hotkey for toggling the autoclicker on
@ t @ s 1
& o # t n 

. Hotkey for toggling the autoclicker off
& p @ s 0
```

```
. Toggleable autoclicker
! quiet true
! enable_hotkey true

. Register for autoclicker state: 0 = off, 1 = on
@ s 0
. Clicker thread
@ c C 0
@ n ^ s 1 c

. Register for turning the autoclicker on
@ T @ s 1
@ t # T n

. Register for turning the autoclicker off
@ u @ s 0

. Hotkey for toggling the autoclicker on/off
& p / s 0 t u

. Exclusive toggle off, as the hotkey may be unresponsive and the autoclicker may accidentally be turned on
& q @ s 0
```
```
. Toggleable autoclicker with 100ms delay and CPS printer
! quiet true
! enable_hotkey true
! enable_cps_register true

. Register for autoclicker state: 0 = off, 1 = on
@ s 0
. Clicker thread
@ c C 0
@ d W 100
@ n ^ s 1 d c

. CPS Printer
@ D W 1500
@ p P CPS:@C
@ N ^ s 1 D p

. Register for turning the autoclicker on
@ T @ s 1
@ t # T n N

. Register for turning the autoclicker off
@ u @ s 0

. Hotkey for toggling the autoclicker on/off
& p / s 0 t u
```
```
. Autocomplete
! quiet true
! enable_hotkey true

. Hotkeys do not eat input 
& o S n my way!
```
```
. Fullscreen application closer
! quiet true
! enable_hotkey true

. Move command
@ m M 64800 750
. Click command
@ c C 0

& p # m c
```