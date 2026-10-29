#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#define MAX_ARGS 64

bool parse_line(char *line, char **args, int *argc_out);

#endif