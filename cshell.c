#include <stdio.h> // Standart-Lib
#include <stdlib.h> // Hilfsfunktionen
#include <string.h> // Textfunktionen
#include <unistd.h> // Unix-Systemaufrufe
#include <sys/wait.h> //Prozesse abwarten

#define MAX_ARGS 64

int main(void)
{
    char line[1024];
    int last_status = 0;

    printf("Willkommen bei C-Shell! (exit zum Beenden)\n");  // Willkommensnachricht

    while (1)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
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

        if (argc == 0) continue;  // Keine Eingabe wird übersprungen

        if (strcmp(args[0], "exit") == 0) break;  // "exit" beenden die Shell

        if (strcmp(args[0], "cd") == 0)  // position (Ordner) wechseln
        {
            const char *dir = args[1] ? args[1] : getenv("HOME");
            if (chdir(dir) != 0) perror("cd");
            continue;
        }

        if (strcmp(args[0], "pwd") == 0)  // aktuelle Position anzeigen
        {
            printf("%s\n", cwd);
            continue;
        }

        char status_text[20];
        char neuer_text[1024];
        snprintf(status_text, sizeof(status_text), "%d", last_status);  // Letzten exit code abfragen

        for (int i = 0; i < argc; i++)  // Eingabe zerlegen und immer bei $? -> Exit code
        {

            char *fundstelle = strstr(args[i], "$?");
            
            if (fundstelle != NULL)
            {
                snprintf(neuer_text, sizeof(neuer_text), "%.*s%s%s", (int)(fundstelle -args[i]),
                args[i],
                status_text,
                fundstelle +2);

                args[i] = neuer_text;
            }
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
            
            if (waitpid(pid, &status, 0) == -1)  // Überprüfen ob Kindprozess erfolgreich beendet wurde
            {
                perror("waitpid");
            }
            else if (WIFEXITED(status))
            {
                last_status =  WEXITSTATUS(status);
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