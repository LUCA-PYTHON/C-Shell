#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "executor.h"
#include "parser.h"

static bool is_name_start(char c)
{
    if (isalpha((unsigned char)c) || c == '_')
    {
        return true;
    }

    return false;
}

static bool is_name_char(char c)
{
    if (isalnum((unsigned char)c) || c == '_')
    {
        return true;
    }

    return false;
}

bool expand_arg(const char *args, char *out, size_t out_size)
{
    size_t zero = 0;
    size_t i = 0;

    if (args[0] == '~' && (args[1] == '/' || args[1] == '\0'))
    {
        const char *home = getenv("HOME");
        if (home != NULL)
        {
            size_t len_home = strlen(home);
            if (len_home >= out_size)
            {
                return false;
            }

            memcpy(out, home, len_home);
            zero = len_home;
            i = 1;
        }
    }

    while (args[i] != '\0')
    {
        if (args[i] == '$' && is_name_start(args[i + 1]))
        {
            char name[256];
            size_t n = 0;
            size_t ii = i + 1;

            while (is_name_char(args[ii]))
            {
                if (n >= sizeof(name) - 1)
                {
                    return false;
                }

                name[n++] = args[ii++];
            }
            name[n] = '\0';

            const char *value = getenv(name);
            if (value != NULL)
            {
                size_t len_value = strlen(value);
                if (zero + len_value >= out_size)
                {
                    return false;
                }

                memcpy(&out[zero], value, len_value);
                zero += len_value;
            }

            i = ii;
            continue;
        }

        if (zero + 1 >= out_size)
        {
            return false;
        }

        out[zero++] = args[i++];
    }

    out[zero] = '\0';
    return true;
}

static bool needs_expansion(const char *arg)
{
    if(strchr(arg, '$') != NULL || (arg[0] == '~' && (arg[1] == '/' || arg[1] == '\0')))
    {
        return true;
    }

    return false;
}

static bool expand_in_pipeline(Pipeline *pipeline, const char **args)
{
    if (*args == NULL || !needs_expansion(*args))
    {
        return true;
    }

    char *target = &pipeline->expanded[pipeline->expanded_used];
    size_t space = EXPAND_BUFFER_SIZE - pipeline->expanded_used;
    if (!expand_arg(*args, target, space))
    {
        fprintf(stderr, "Error: expanded argument too long\n");
        return false;
    }

    pipeline->expanded_used += strlen(target) + 1;
    *args = target;
    return true;
}

bool expand_pipeline(Pipeline *pipeline)
{
    pipeline->expanded_used = 0;
    for (int c = 0; c < pipeline->command_count; c++)
    {
        PipelineCommand *command = &pipeline->commands[c];
        for (int i = 0; i < command->argc; i++)
        {
            const char *args = command->args[i];
            if (!expand_in_pipeline(pipeline, &args))
            {
                return false;
            }

            command->args[i] = (char *)args;
        }

        if (!expand_in_pipeline(pipeline, &command->redirections.input_file) || !expand_in_pipeline(pipeline, &command->redirections.output_file))
        {
            return false;
        }
    }

    return true;
}

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

bool parse_pipeline(char *line, Pipeline *pipeline)
{
    bool inside_quotes = false;
    bool token_started = false;

    char *read = line;
    char *write = line;

    pipeline->command_count = 1;
    PipelineCommand *command = &pipeline->commands[0];
    command->argc = 0;
    command->args[0] = NULL;
    command->redirections = (Redirections){NULL, NULL, false};

    while(*read != '\0')
    {
        if (*read == '"')
        {
            if (!token_started)
            {
                if (command->argc >= MAX_ARGS -1)
                {
                    fprintf(stderr, "Error: too many arguments\n");
                    return false;
                }

                command->args[command->argc++] = write;
                token_started = true;
            }

            inside_quotes = !inside_quotes;
            read++;
            continue;
        }

        if (!inside_quotes && isspace((unsigned char)*read))
        {
            if (token_started)
            {
                *write = '\0';
                write++;
                token_started = false;
            }

            read++;
            continue;
        }

        if (!inside_quotes && *read == '|')
        {
            if (token_started)
            {
                *write = '\0';
                write++;
                token_started = false;
            }

            if (command->argc == 0)
            {
                fprintf(stderr, "Error: empty pipeline command\n");
                return false;
            }

            command->args[command->argc] = NULL;
            if (!extract_redirections(command->args, &command->argc, &command->redirections))
            {
                return false;
            }

            if (pipeline->command_count >= MAX_PIPELINE_COMMANDS)
            {
                fprintf(stderr, "Error: too many pipeline commands\n");
                return false;
            }

            command = &pipeline->commands[pipeline->command_count];
            command->argc = 0;
            command->args[0] = NULL;
            command->redirections = (Redirections){NULL, NULL, false};

            pipeline->command_count++;
            read++;
            continue;
        }

        if (!token_started)
        {
            if (command->argc >= MAX_ARGS -1)
            {
                fprintf(stderr, "Error: too many arguments\n");
                return false;
            }

            command->args[command->argc++] = write;
            token_started = true;
        }

        *write = *read;
        write++;
        read++;
    }

    if (inside_quotes)
    {
        fprintf(stderr, "Error: unmatched quotation marks\n");
        return false;
    }

    if (token_started)
    {
        *write = '\0';
        write++;
    }

    if (command->argc == 0)
    {
        fprintf(stderr, "Error: empty pipeline command\n");
        return false;
    }

    command->args[command->argc] = NULL;
    return extract_redirections(command->args, &command->argc, &command->redirections);
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

        char *file = args[i + 1];
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