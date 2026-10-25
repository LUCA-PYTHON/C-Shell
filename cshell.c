#include <stdio.h> // Standart-Lib
#include <stdlib.h> // Hilfsfunktionen
#include <string.h> // Textfunktionen
#include <unistd.h> // Unix-Systemaufrufe
#include <sys/wait.h> //Prozesse abwarten

#define MAX_ARGS 64

int main(void)
{
    char line[1024];
    printf("Willkommen bei C-Shell! (exit zum Beenden)\n");

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
        char *token = strtok(line, "\t");
        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        if (strcmp(args[0], "exit") == 0) break;

        if (strcmp(args[0], "cd") == 0)
        {
            const char *dir = args[1] ? args[1] : getenv("HOME");
            if (chdir(dir) != 0) perror("cd");
            continue;
        }

        if (strcmp(args[0], "pwd") == 0)
        {
            printf("%s\n", cwd);
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
            waitpid(pid, NULL, 0);
        }
        else
        {
            perror("fork");
        }
        
    }
    return 0;
    
}

/*
sudo apt install build-essential
gcc -Wall -Wextra -o cshell cshell.c

mkdir -p ~/.local/bin ~/.local/share/applications
cp cshell ~/.local/bin/

cat > ~/.local/share/applications/c-shell.desktop << EOF
[Desktop Entry]
Type=Application
Name=C-Shell
Comment=Meine selbstgebaute Shell in C
Exec=$HOME/.local/bin/cshell
Terminal=true
Categories=System;TerminalEmulator;
EOF

update-desktop-database ~/.local/share/applications
*/