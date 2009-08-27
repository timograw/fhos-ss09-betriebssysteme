#ifndef SHELL_H
#define SHELL_H

/* shell.h */

#define INPUTLINEBUFFER_LENGTH 256
#define PARAMETERS_MAX 50

#define RET_EXIT -1

#define ERROR_MALLOCFAILED -1
#define ERROR_NULLPOINTER -1
#define ERROR_NODIRSTRING 1

#define PATH_DEPTH 256
#define PATH_BUFFERLENGTH 512

int_fast16_t prompt();
int fork_process(char *input_string);
int handle_command(char *input_string);
int32_t parse_parameters(char *command_string, char *parameters[PARAMETERS_MAX]);
int execute_command(char *path, char *parameters[PARAMETERS_MAX]);
int resolve_variable(char **parameter);
int change_directory(char *directory_string);

#endif
