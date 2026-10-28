#ifndef BUILTINS_H
#define BUILTINS_H

int change_directory(const char *path);
void print_working_directory(const char *cwd);
void print_echo(char **args, int last_status);
void print_help();

#endif