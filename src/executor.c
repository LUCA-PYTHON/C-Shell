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

static int wait_for_process(pid_t pid, int *status, bool *wait_error)
{
    pid_t result;

    do
    {
        result = waitpid(pid, status, 0);
    } while (result == -1 && errno == EINTR);

    if (result == -1)
    {
        perror("waitpid");
        *wait_error = true;
        return SHELL_STATUS_ERROR;
    }

    if (WIFEXITED(*status))
    {
        return WEXITSTATUS(*status);
    }

    if (WIFSIGNALED(*status))
    {
        return 128 + WTERMSIG(*status);
    }

    return SHELL_STATUS_ERROR;
}

int execute_pipeline(Pipeline *pipeline)
{
    pid_t child_pids[MAX_PIPELINE_COMMANDS];
    int child_count = 0;
    int previous_read_fd = -1;
    bool launch_error = false;
    bool wait_error = false;

    if (pipeline == NULL || pipeline->command_count < 1 ||
        pipeline->command_count > MAX_PIPELINE_COMMANDS)
    {
        return SHELL_STATUS_ERROR;
    }

    for (int i = 0; i < pipeline->command_count; i++)
    {
        int pipe_fd[2] = {-1, -1};
        bool has_next_command = i + 1 < pipeline->command_count;

        if (pipeline->commands[i].args[0] == NULL)
        {
            fprintf(stderr, "Error: missing command in pipeline\n");
            launch_error = true;
            if (previous_read_fd != -1)
            {
                close(previous_read_fd);
            }
            break;
        }

        if (has_next_command && pipe(pipe_fd) == -1)
        {
            perror("pipe");
            launch_error = true;
            if (previous_read_fd != -1)
            {
                close(previous_read_fd);
            }
            break;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            if (previous_read_fd != -1 &&
                dup2(previous_read_fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                _exit(SHELL_STATUS_ERROR);
            }

            if (has_next_command && dup2(pipe_fd[1], STDOUT_FILENO) == -1)
            {
                perror("dup2");
                _exit(SHELL_STATUS_ERROR);
            }

            if (previous_read_fd != -1)
            {
                close(previous_read_fd);
            }
            if (pipe_fd[0] != -1)
            {
                close(pipe_fd[0]);
            }
            if (pipe_fd[1] != -1)
            {
                close(pipe_fd[1]);
            }

            if (apply_redirections(&pipeline->commands[i].redirections) == SHELL_STATUS_ERROR)
            {
                _exit(SHELL_STATUS_ERROR);
            }

            execvp(pipeline->commands[i].args[0], pipeline->commands[i].args);

            if (errno == EACCES)
            {
                fprintf(stderr, "%s: permission denied\n", pipeline->commands[i].args[0]);
                _exit(SHELL_STATUS_NOT_EXECUTABLE);
            }
            if (errno == ENOENT)
            {
                fprintf(stderr, "%s: command not found\n", pipeline->commands[i].args[0]);
                _exit(SHELL_STATUS_COMMAND_NOT_FOUND);
            }

            perror(pipeline->commands[i].args[0]);
            _exit(SHELL_STATUS_ERROR);
        }

        if (pid == -1)
        {
            perror("fork");
            launch_error = true;
            if (previous_read_fd != -1)
            {
                close(previous_read_fd);
                previous_read_fd = -1;
            }
            if (pipe_fd[0] != -1)
            {
                close(pipe_fd[0]);
            }
            if (pipe_fd[1] != -1)
            {
                close(pipe_fd[1]);
            }
            break;
        }

        child_pids[child_count++] = pid;

        if (previous_read_fd != -1)
        {
            close(previous_read_fd);
        }
        if (has_next_command)
        {
            close(pipe_fd[1]);
            previous_read_fd = pipe_fd[0];
        }
    }

    int pipeline_status = SHELL_STATUS_ERROR;
    for (int i = 0; i < child_count; i++)
    {
        int status;
        int command_status = wait_for_process(child_pids[i], &status, &wait_error);
        if (i == child_count - 1)
        {
            pipeline_status = command_status;
        }
    }

    if (launch_error || wait_error)
    {
        return SHELL_STATUS_ERROR;
    }

    return pipeline_status;
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
        bool wait_error = false;
        command_status = wait_for_process(pid, &status, &wait_error);
    }
    else
    {
        perror("fork");
        command_status = SHELL_STATUS_ERROR;
    }

    return command_status;
}