#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "builtins.h"
#include "shell_status.h"

int change_directory(const char *path)
{
    const char *dir = path ? path : getenv("HOME");
    if (dir == NULL)
    {
        fprintf(stderr, "cd: HOME ist nicht gesetzt");
        return SHELL_STATUS_ERROR;
    }
    else if (chdir(dir) != 0)
    {
        perror("cd");
        return SHELL_STATUS_ERROR;
    }

    return SHELL_STATUS_SUCCESS;
}

int print_working_directory(const char *cwd)
{
    if(printf("%s\n", cwd) < 0)
    {
        return SHELL_STATUS_ERROR;
    }
    return SHELL_STATUS_SUCCESS;
}

int print_echo(char **args, int last_status)
{
    char status_text[20];
    char puffer1[1024];
    char puffer2[1024];

    // Convert the last exit status to text
    snprintf(status_text, sizeof(status_text), "%d", last_status);

    for (int i = 1; args[i] != NULL; i++)
    {

        // Copy the argument into the working buffer
        int written = snprintf(puffer1, sizeof(puffer1), "%s", args[i]);
        if (written < 0 || (size_t)written >= sizeof(puffer1))
        {
            fprintf(stderr, "Error: Eingabe zu lang\n");
            return SHELL_STATUS_ERROR;
        }

        // Search for $? in the current text
        char *match = strstr(puffer1, "$?");
        while (match != NULL)
        {
            written = snprintf(puffer2, sizeof(puffer2), "%.*s%s%s",
                                   (int)(match - puffer1),
                                   puffer1,
                                   status_text,
                                   match + 2);

            if (written < 0 || (size_t)written >= sizeof(puffer2))
            {
                fprintf(stderr, "Error: Expanded text is too long\n");
                return SHELL_STATUS_ERROR;
            }

            // Copy the replaced result back into the working buffer
            written = snprintf(puffer1, sizeof(puffer1), "%s", puffer2);
            if (written < 0 || (size_t)written >= sizeof(puffer1))
            {
                fprintf(stderr, "Error: Expanded text is too long\n");
                return SHELL_STATUS_ERROR;
            }

            // Im aktualisierten Text nach dem nächsten $? suchen
            match = strstr(puffer1, "$?");
        }

        // Argumente mit Leerzeichen trennen

        if (i > 1)
        {
            printf(" ");
        }

        printf("%s", puffer1);
        
    }
    printf("\n");
    return SHELL_STATUS_SUCCESS;

}

int print_help()
{
    printf("\n");
    printf("\033[1;33m  C-Shell - Befehlsübersicht\033[0m\n");
    printf("\033[0m");
    printf("  \033[1;32m%-14s\033[0m %s\n", "cd [ordner]", "Verzeichnis wechseln");
    printf("  \033[1;32m%-14s\033[0m %s\n", "pwd", "Aktuelles Verzeichnis anzeigen");
    printf("  \033[1;32m%-14s\033[0m %s\n", "help", "Diese Hilfe anzeigen");
    printf("  \033[1;32m%-14s\033[0m %s\n", "exit", "Shell beenden");
    printf("  \033[1;32m%-14s\033[0m %s\n", "echo $?", "Letzten Exit-Status anzeigen");

    printf("\n  \033[90mWeitere Linux-Befehle wie ls, cat und gcc sind ebenfalls verfügbar.\033[0m\n");
    printf("\n");

    return SHELL_STATUS_SUCCESS;
}