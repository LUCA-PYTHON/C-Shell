#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "executor.h"
#include "shell_status.h"

int execute_external_command(char **args)
{
    int command_status = SHELL_STATUS_SUCCESS;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(args[0], args);

        if (errno == EACCES)
        {
            fprintf(stderr, "%s: permission denied\n", args[0]);
            _exit(SHELL_STATUS_NOT_EXECUTABLE);
        }
        else if (errno == ENOENT)
        {
            fprintf(stderr, "%s: command not found\n", args[0]);
            _exit(SHELL_STATUS_COMMAND_NOT_FOUND);
        }
        else
        {
            perror(args[0]);
            _exit(SHELL_STATUS_ERROR);
        }
    }

    else if (pid > 0)
    {
        int status;
        pid_t result;

        do
        {
            result = waitpid(pid, &status, 0);
        }while (result == -1 && errno == EINTR);

        if (result == -1)
        {
            perror("waitpid");
            command_status = SHELL_STATUS_ERROR;
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
        command_status = SHELL_STATUS_ERROR;
    }

    return command_status;
}