#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "executor.h"

int execute_external_command(char **args)
{
    int command_status = 0;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(args[0], args);

        if (errno == EACCES)
        {
            fprintf(stderr, "%s: permission denied\n", args[0]);
            _exit(126);
        }
        else if (errno == ENOENT)
        {
            fprintf(stderr, "%s: command not found\n", args[0]);
            _exit(127);
        }
        else
        {
            perror(args[0]);
            _exit(1);
        }
    }

    else if (pid > 0)
    {
        int status;

        if (waitpid(pid, &status, 0) == -1) // Überprüfen ob Kindprozess erfolgreich beendet wurde
        {
            if (errno == ECHILD)
            {
                fprintf(stderr, "%s: waitpid error", args[0]);
            }
            else
            {
                perror("waitpid");
                command_status = 1;
            }
        }
        else if (WIFEXITED(status))
        {
            command_status = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            command_status = 128 + WTERMSIG(status);
        }
    }
    else
    {
        perror("fork");
        command_status = 1;
    }

    return command_status;
}