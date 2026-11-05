#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "file_operations.h"
#include "shell_status.h"

int handle_file_command(int argc, char **args)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: file <-c|-w|-a|-d|-r> <path>\n");
        return SHELL_STATUS_ERROR;
    }

    if (strcmp(args[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing file path\n");
            return SHELL_STATUS_ERROR;
        }
        else if (argc != 3)
        {
            fprintf(stderr, "Usage: file -c <path>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_create(args[2]);
    }

    if (strcmp(args[1], "-w") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing file path\n");
            return SHELL_STATUS_ERROR;
        }
        else if (argc < 4)
        {
            fprintf(stderr, "file: missing file content\n");
            return SHELL_STATUS_ERROR;
        }
        else if (argc != 4)
        {
            fprintf(stderr, "Usage: file -w <path> <content>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_write(args[2], args[3]);
    }
    fprintf(stderr, "file: unknown option '%s'\n", args[1]);
    return SHELL_STATUS_ERROR;
}

int file_create(const char *path)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    int file = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (file == -1)
    {
        if (errno == EEXIST)
        {
            fprintf(stderr, "Error: File already exists\n");
        }
        else
        {
            perror("open");
        }
        return SHELL_STATUS_ERROR;
    }

    if (close(file) == -1)
    {
        perror("close");
        return SHELL_STATUS_ERROR;
    }

    return SHELL_STATUS_SUCCESS;
}

int file_write(const char *path, const char *content)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing path\n");
        return SHELL_STATUS_ERROR;
    }

    if (content == NULL)
    {
        fprintf(stderr, "file: missing content\n");
        return SHELL_STATUS_ERROR;
    }

    int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1)
    {
        perror("open");
        return SHELL_STATUS_ERROR;
    }

    size_t total_length = strlen(content);
    size_t total_written = 0;
    while (total_written < total_length)
    {
        ssize_t bytes_written = write(file, content + total_written, total_length - total_written);
        if (bytes_written == -1)
        {
            perror("write");
            if (close(file) == -1)
            {
                perror("close");
                return SHELL_STATUS_ERROR;
            }
            return SHELL_STATUS_ERROR;
        }

        if (bytes_written <= 0)
        {
            fprintf(stderr, "file: could not write to file\n");
            if (close(file) == -1)
            {
                perror("close");
                return SHELL_STATUS_ERROR;
            }
            return SHELL_STATUS_ERROR;
        }
        total_written = total_written + bytes_written;
    }

    if (close(file) == -1)
    {
        perror("close");
        return SHELL_STATUS_ERROR;
    }

    return SHELL_STATUS_SUCCESS;
}
