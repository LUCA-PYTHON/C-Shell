#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "executor.h"
#include "shell_status.h"

static int redirect_fd(const char *path, int flags, int target_fd)
{
    int fd = open(path, flags, 0644);
    if (fd == -1)
    {
        perror(path);
        return SHELL_STATUS_ERROR;
    }

    if (dup2(fd, target_fd) == -1)
    {
        perror("dup2");
        close(fd);
        return SHELL_STATUS_ERROR;
    }

    close(fd);
    return SHELL_STATUS_SUCCESS;
}

static int apply_redirections(const Redirections *redirections)
{
    if (redirections == NULL)
    {
        return SHELL_STATUS_SUCCESS;
    }

    if (redirections->input_file != NULL)
    {
        if (redirect_fd(redirections->input_file, O_RDONLY, STDIN_FILENO) == SHELL_STATUS_ERROR)
        {
            return SHELL_STATUS_ERROR;
        }
    }

    if (redirections->output_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT | (redirections->append ? O_APPEND : O_TRUNC);
        if (redirect_fd(redirections->output_file, flags, STDOUT_FILENO) == SHELL_STATUS_ERROR)
        {
            return SHELL_STATUS_ERROR;
        }
    }

    return SHELL_STATUS_SUCCESS;
}

int execute_external_command(char **args, const Redirections *redirections)
{
    int command_status = SHELL_STATUS_SUCCESS;

    pid_t pid = fork();
    if (pid == 0)
    {
        if (apply_redirections(redirections) == SHELL_STATUS_ERROR)
        {
            _exit(SHELL_STATUS_ERROR);
        }

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