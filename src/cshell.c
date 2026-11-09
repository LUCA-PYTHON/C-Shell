#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtins.h"
#include "input.h"
#include "parser.h"
#include "shell_status.h"
#include "file_operations.h"

int main(void)
{
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("sigaction");
        return SHELL_STATUS_ERROR;
    }

    char *line = NULL;
    int last_status = SHELL_STATUS_SUCCESS;
    int exit_status = 0;

    int interactive = isatty(STDIN_FILENO);
    if (interactive)
    {
        printf("Welcome to C-Shell! (type exit to quit)\n");
    }

    while (1)
    {
        free(line);
        line = NULL;

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            strcpy(cwd, "?");
        
        if (interactive)
        {
            printf("c-shell [%s]> ", cwd);
            fflush(stdout);
        }
        
        size_t prompt_column = strlen("c-shell [") + strlen(cwd) + strlen("]> ");

        int input_error = read_input(&line, prompt_column);
        if (input_error == INPUT_STATUS_INTERRUPTED)
        {
            last_status = 130;
            continue;
        }

        if (input_error == INPUT_STATUS_EOF)
        {
            free(line);
            break;
        }

        if (input_error == INPUT_STATUS_ERROR)
        {
            free(line);
            perror("read_input");
            break;
        }

        Pipeline pipeline;
        if (!parse_pipeline(line, &pipeline))
        {
            last_status = SHELL_STATUS_PARSE_ERROR;
            continue;
        }

        if (pipeline.commands[0].argc == 0)
        {
            continue;
        }

        if (pipeline.command_count > 1)
        {
            last_status = execute_pipeline(&pipeline);
            continue;
        }

        char **args = pipeline.commands[0].args;
        int argc = pipeline.commands[0].argc;

        if (strcmp(args[0], "exit") == 0)
        {
            if (handle_exit(argc, args, &exit_status) == 0)
            {
                free(line);
                return exit_status;
            }

            last_status = SHELL_STATUS_ERROR;
            continue;
        }

        if (strcmp(args[0], "cd") == 0)
        {
            if (argc > 2)
            {
                fprintf(stderr, "Error: too many arguments\n");
                last_status = SHELL_STATUS_ERROR;
                continue;
            }
            last_status = change_directory(args[1]);
            continue;
        }

        if (strcmp(args[0], "pwd") == 0)
        {
            last_status = print_working_directory(cwd);
            continue;
        }

        if (strcmp(args[0], "file") == 0)
        {
            last_status = handle_file_command(argc, args);
            continue;
        }

        if (strcmp(args[0], "env") == 0)
        {
            last_status = print_environment(argc, args);
            continue;
        }

        if (strcmp(args[0], "export") == 0)
        {
            last_status = export_variable(argc, args);
            continue;
        }

        if (strcmp(args[0], "help") == 0 || strcmp(args[0], "h") == 0)
        {
            last_status = print_help();
            continue;
        }

        if (strcmp(args[0], "echo") == 0)
        {
            last_status = print_echo(args, last_status);
            continue;
        }

        last_status = execute_pipeline(&pipeline);
    }
    return last_status;
}