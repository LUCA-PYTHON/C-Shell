#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "redirections.h"

#define MAX_ARGS 64
#define MAX_PIPELINE_COMMANDS 16

typedef struct
{
    char *args[MAX_ARGS];
    int argc;
    Redirections redirections;
} PipelineCommand;

typedef struct
{
    PipelineCommand commands[MAX_PIPELINE_COMMANDS];
    int command_count;
} Pipeline;

bool parse_line(char *line, char **args, int *argc_out);
bool parse_pipeline(char *line, Pipeline *pipeline);
bool extract_redirections(char **args, int *argc, Redirections *redirections);

#endif