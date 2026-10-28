#include <stdio.h>    // Standart-Lib
#include <stdlib.h>   // Hilfsfunktionen
#include <string.h>   // Textfunktionen
#include <unistd.h>   // Unix-Systemaufrufe
#include <sys/wait.h> //Prozesse abwarten
#include <errno.h>

#include "executor.h"
#include "builtins.h"
#include "input.h"

#define MAX_ARGS 64

int main(void)
{
    char line[1024];
    int last_status = 0;

    printf("Willkommen bei C-Shell! (exit zum Beenden)\n"); // Willkommensnachricht

    while (1)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            strcpy(cwd, "?");
        printf("c-shell [%s]>", cwd);
        fflush(stdout);

        int input_error = read_input(line, sizeof(line));
        if (input_error == -1)
        {
            break;
        }


        char *args[MAX_ARGS];
        int argc = 0;
        char *token = strtok(line, " \t");
        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }
        args[argc] = NULL;

        if (argc == 0)
            continue; // Keine Eingabe wird übersprungen

        if (strcmp(args[0], "exit") == 0)
            break; // "exit" beenden die Shell

        if (strcmp(args[0], "cd") == 0) // position (Ordner) wechseln
        {
            change_directory(args[1]);
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) // aktuelle Position anzeigen
        {
            print_working_directory(cwd);
            continue;
        }

        if (strcmp(args[0], "help") == 0 || strcmp(args[0], "h") == 0)
        {
            print_help();
            continue;
        }

        if (strcmp(args[0], "echo") == 0)
        {
            print_echo(args, last_status);
            continue;
        }

        last_status = execute_external_command(args);
    }
    return 0;
}