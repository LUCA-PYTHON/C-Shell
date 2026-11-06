#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

typedef struct
{
    const char *input_file;  /*"< datei"*/
    const char *output_file;  /*"> datei" oder ">> Datei"*/
    bool append;  /*True bei ">>", false bei ">"*/
} Redirections;

int execute_external_command(char **args, const Redirections *redirections);

#endif