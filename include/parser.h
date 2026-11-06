#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "executor.h"

#define MAX_ARGS 64

bool parse_line(char *line, char **args, int *argc_out);
bool extract_redirections(char **args, int *argc, Redirections *redirections);

#endif