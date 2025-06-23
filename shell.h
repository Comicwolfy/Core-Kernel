#ifndef SHELL_H
#define SHELL_H

// Shell function declarations
void start_shell(void);
void shell_init(void);
void shell_run(void);
void shell_prompt(void);

// Command function declarations
void cmd_help(int argc, char** argv);
void cmd_clear(int argc, char** argv);
void cmd_echo(int argc, char** argv);
void cmd_version(int argc, char** argv);
void cmd_reboot(int argc, char** argv);
void cmd_halt(int argc, char** argv);

// Terminal functions (if you want to use them elsewhere)
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_writestring(const char* data);

#endif // SHELL_H