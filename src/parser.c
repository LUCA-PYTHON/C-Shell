#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "executor.h"
#include "parser.h"

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
            if (!in_token)
            {
                args[argc] = &line[write];
                argc++;
                in_token = true;
            }
            
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

static bool is_redirection_operator(const char *token)
{
    return strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || strcmp(token, ">>") == 0;
}

bool extract_redirections(char **args, int *argc, Redirections *redirections)
{
    int kept = 0;
    bool found = false;

    for (int i = 0; i < *argc; i++)
    {
        if (!is_redirection_operator(args[i]))
        {
            args[kept] = args[i];
            kept++;
            continue;
        }

        if (i + 1 >= *argc || is_redirection_operator(args[i + 1]))
        {
            fprintf(stderr, "Error: missing file after '%s'\n", args[i]);
            return false;
        }

        const char *file = args[i + 1];
        if (strcmp(args[i], "<") == 0)
        {
            redirections->input_file = file;
        }
        else
        {
            redirections->output_file = file;
            redirections->append = (strcmp(args[i], ">>") == 0);
        }

        found = true;
        i++;
    }

    args[kept] = NULL;
    *argc = kept;

    if (found && kept == 0)
    {
        fprintf(stderr, "Error: missing command\n");
        return false;
    }

    return true;
}