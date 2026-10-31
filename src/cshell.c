#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtins.h"
#include "input.h"
#include "parser.h"
#include "shell_status.h"

int main(void)
{
    char line[1024];
    int last_status = SHELL_STATUS_SUCCESS;
    int exit_status = 0;

    printf("Willkommen bei C-Shell! (exit zum Beenden)\n");

    while (1)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            strcpy(cwd, "?");
        printf("c-shell [%s]>", cwd);
        fflush(stdout);

        int input_error = read_input(line, sizeof(line));
        if (input_error == INPUT_STATUS_EOF)
        {
            break;
        }

        if (input_error == INPUT_STATUS_ERROR)
        {
            perror("read_input");
            break;
        }

        if (input_error == INPUT_STATUS_TOO_LONG)
        {
            fprintf(stderr, "Error: Input is too long\n");
            last_status = SHELL_STATUS_PARSE_ERROR;
            continue;
        }

        char *args[MAX_ARGS];
        int argc = 0;

        if (!parse_line(line, args, &argc))
        {
            last_status = SHELL_STATUS_PARSE_ERROR;
            continue;
        }

        if (argc == 0)
            continue;  // Skip empty input

        if (strcmp(args[0], "exit") == 0)
        {
            if (handle_exit(argc, args, &exit_status) == 0)
            {
                return exit_status;
            }

            last_status = SHELL_STATUS_ERROR;
            continue;
        }

        if (strcmp(args[0], "cd") == 0)  // Change the current directory
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

        if (strcmp(args[0], "pwd") == 0)  // Print the current working directory
        {
            last_status = print_working_directory(cwd);
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

        last_status = execute_external_command(args);
    }
    return last_status;
}