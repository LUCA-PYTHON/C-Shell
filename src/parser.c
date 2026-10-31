#include <stdio.h>
#include <stdbool.h>

#include "parser.h"
#include "shell_status.h"

bool parse_line(char *line, char **args, int *argc_out)
{
    int argc = 0;
    int write = 0;
    bool in_quotes = false;
    bool in_token = false;

    for (int read = 0; line[read] != '\0'; read++)
    {
        if (line[read] == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }

        if ((line[read] == ' ' || line[read] == '\t') && !in_quotes)
        {
            if (in_token)
            {
                line[write] = '\0';
                in_token = false;
                write++;
            }
            continue;
        }

        if (!in_token)
        {
            if (argc >= MAX_ARGS - 1)
            {
                fprintf(stderr, "Error: too many arguments\n");
                return false;
            }

            args[argc] = &line[write];
            argc++;
            in_token = true;
        }
        line[write] = line[read];
        write++;
    }

    if (in_quotes)
    {
        fprintf(stderr, "Error: Unclosed quotation mark\n");
        return false;
    }
    line[write] = '\0';
    args[argc] = NULL;
    *argc_out = argc;

    return true;
}