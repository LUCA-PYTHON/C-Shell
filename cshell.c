#include <stdio.h>    // Standart-Lib
#include <stdlib.h>   // Hilfsfunktionen
#include <string.h>   // Textfunktionen
#include <unistd.h>   // Unix-Systemaufrufe
#include <sys/wait.h> //Prozesse abwarten

#define MAX_ARGS 64

void hilfe()
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

        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = '\0';

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
            const char *dir = args[1] ? args[1] : getenv("HOME");
            if (dir == NULL)
            {
                fprintf(stderr, "cd: HOME ist nicht gesetzt");
            }
            else if (chdir(dir) != 0)
            {
                perror("cd");
            }

            continue;
        }

        if (strcmp(args[0], "pwd") == 0) // aktuelle Position anzeigen
        {
            printf("%s\n", cwd);
            continue;
        }

        if (strcmp(args[0], "help") == 0 || strcmp(args[0], "h") == 0)
        {
            hilfe();
            continue;
        }

        if (strcmp(args[0], "echo") == 0)
        {
            char status_text[20];
            char puffer1[1024];
            char puffer2[1024];

            // Letzten Exit-Status in Text umwandeln
            snprintf(status_text, sizeof(status_text), "%d", last_status); // Letzten Exit-Status abfragen
            
            for (int i = 1; i < argc; i++)
            {

                // Argument in den Arbeitspuffer kopieren
                int geschrieben = snprintf(puffer1, sizeof(puffer1), "%s", args[i]);
                if (geschrieben < 0 || (size_t)geschrieben >= sizeof(puffer1))
                {
                    fprintf(stderr, "Error: Eingabe zu lang\n");
                    continue;
                }

                // Nach $? im aktuellen Text suchen
                char *fundstelle = strstr(puffer1, "$?");
                
                while(fundstelle != NULL)
                {
                    // Text vor $? + Exit-Status + Text nach $?
                    geschrieben = snprintf(puffer2, sizeof(puffer2), "%.*s%s%s",
                        (int)(fundstelle - puffer1),
                        puffer1,
                        status_text,
                        fundstelle +2);

                    if (geschrieben < 0 || (size_t)geschrieben >= sizeof(puffer2))
                    {
                        fprintf(stderr, "Error: Ersetzter Text zu lang\n");
                        break;
                    }

                    // Ersetztes Ergebnis zurück in den Arbeitspuffer kopieren
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

            continue;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);
            perror(args[0]);
            exit(1);
        }
        else if (pid > 0)
        {
            int status;

            if (waitpid(pid, &status, 0) == -1) // Überprüfen ob Kindprozess erfolgreich beendet wurde
            {
                perror("waitpid");
            }
            else if (WIFEXITED(status))
            {
                last_status = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
                last_status = 128 + WTERMSIG(status);
            }
        }
        else
        {
            perror("fork");
        }
    }
    return 0;
}