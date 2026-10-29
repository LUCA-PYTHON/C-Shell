#ifndef BUILTINS_H
#define BUILTINS_H

int change_directory(const char *path);
int print_working_directory(const char *cwd);
int print_echo(char **args, int last_status);
int print_help();

#endif