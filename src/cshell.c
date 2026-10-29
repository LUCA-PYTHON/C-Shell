#include <stdio.h>    // Standart-Lib
#include <stdlib.h>   // Hilfsfunktionen
#include <string.h>   // Textfunktionen
#include <unistd.h>   // Unix-Systemaufrufe
#include <sys/wait.h> //Prozesse abwarten

#include "executor.h"
#include "builtins.h"
#include "input.h"
#include "parser.h"
#include "shell_status.h"

#define MAX_ARGS 64

int main(void)
{
    char line[1024];
    int last_status = SHELL_STATUS_SUCCESS;

    printf("Willkommen bei C-Shell! (exit zum Beenden)\n"); // Willkommensnachricht

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

        char *args[MAX_ARGS];
        int argc = 0;

        if (!parse_line(line, args, &argc))
        {
            last_status = SHELL_STATUS_PARSE_ERROR;
            continue;
        }

        if (argc == 0)
            continue; // Keine Eingabe wird übersprungen

        if (strcmp(args[0], "exit") == 0)
            break; // "exit" beenden die Shell

        if (strcmp(args[0], "cd") == 0) // position (Ordner) wechseln
        {
            last_status = change_directory(args[1]);
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) // aktuelle Position anzeigen
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
    return SHELL_STATUS_SUCCESS;
}