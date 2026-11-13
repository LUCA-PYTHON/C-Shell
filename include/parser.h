#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "redirections.h"

#define MAX_ARGS 64
#define MAX_PIPELINE_COMMANDS 16
#define EXPAND_BUFFER_SIZE 4096

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
    char expanded[EXPAND_BUFFER_SIZE];
    size_t expanded_used;
} Pipeline;

bool expand_arg(const char *args, char *out, size_t out_size);
bool expand_pipeline(Pipeline *pipeline);
bool parse_line(char *line, char **args, int *argc_out);
bool parse_pipeline(char *line, Pipeline *pipeline);
bool extract_redirections(char **args, int *argc, Redirections *redirections);

#endif