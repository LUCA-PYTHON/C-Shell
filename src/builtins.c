#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "builtins.h"

int change_directory(const char *path)
{
    const char *dir = path ? path : getenv("HOME");
    if (dir == NULL)
    {
        fprintf(stderr, "cd: HOME ist nicht gesetzt");
        return 1;
    }
    else if (chdir(dir) != 0)
    {
        perror("cd");
        return 1;
    }

    return 0;
}

void print_working_directory(const char *cwd)
{
    printf("%s\n", cwd);
}

void print_echo(char **args, int last_status)
{
    char status_text[20];
    char puffer1[1024];
    char puffer2[1024];

    // Convert the last exit status to text
    snprintf(status_text, sizeof(status_text), "%d", last_status);

    for (int i = 1; args[i] != NULL; i++)
    {

        // Copy the argument into the working buffer
        int geschrieben = snprintf(puffer1, sizeof(puffer1), "%s", args[i]);
        if (geschrieben < 0 || (size_t)geschrieben >= sizeof(puffer1))
        {
            fprintf(stderr, "Error: Eingabe zu lang\n");
            continue;
        }

        // Search for $? in the current text
        char *fundstelle = strstr(puffer1, "$?");

        while (fundstelle != NULL)
        {
            geschrieben = snprintf(puffer2, sizeof(puffer2), "%.*s%s%s",
                                   (int)(fundstelle - puffer1),
                                   puffer1,
                                   status_text,
                                   fundstelle + 2);

            if (geschrieben < 0 || (size_t)geschrieben >= sizeof(puffer2))
            {
                fprintf(stderr, "Error: Ersetzter Text zu lang\n");
                break;
            }

            // Copy the replaced result back into the working buffer
            geschrieben = snprintf(puffer1, sizeof(puffer1), "%s", puffer2);
            if (geschrieben < 0 || (size_t)geschrieben >= sizeof(puffer1))
            {
                fprintf(stderr, "Error: Ersetzter Text zu lange\n");
                break;
            }

            // Im aktualisierten Text nach dem nächsten $? suchen
            fundstelle = strstr(puffer1, "$?");
        }

        // Argumente mit Leerzeichen trennen
        if (i > 1)
        {
            printf(" ");
        }
        printf("%s", puffer1);
    }
    printf("\n");

}

void print_help()
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
}