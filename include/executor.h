#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

#include "parser.h"
#include "redirections.h"

int execute_pipeline(Pipeline *pipeline);
int execute_external_command(char **args, const Redirections *redirections);

#endif