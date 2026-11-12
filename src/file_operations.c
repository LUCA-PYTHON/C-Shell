#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "file_operations.h"
#include "shell_status.h"

int handle_file_command(int argc, char **args)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: file <-o|-c|-w|-a|-d|-D|-r> <path>\n");
        return SHELL_STATUS_ERROR;
    }

    if (strcmp(args[1], "-o") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing file path\n");
            return SHELL_STATUS_ERROR;
        }
        else if (argc != 3)
        {
            fprintf(stderr, "Usage: file -o <path>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_open(args[2]);
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

    if (strcmp(args[1], "-r") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing file path\n");
            return SHELL_STATUS_ERROR;  
        }
        else if (argc != 3)
        {
            fprintf(stderr, "Usage: file -r <path>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_read(args[2]);
    }

    if (strcmp(args[1], "-d") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing file path\n");
            return SHELL_STATUS_ERROR;  
        }
        else if (argc != 3)
        {
            fprintf(stderr, "Usage: file -d <path>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_delete(args[2]);
    }

    if (strcmp(args[1], "-D") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "file: missing directory path\n");
            return SHELL_STATUS_ERROR;  
        }
        else if (argc != 3)
        {
            fprintf(stderr, "Usage: file -D <directory>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_delete_directory(args[2]);
    }

    if (strcmp(args[1], "-a") == 0)
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
            fprintf(stderr, "Usage: file -a <path> <content>\n");
            return SHELL_STATUS_ERROR;
        }
        return file_append(args[2], args[3]);
    }

    fprintf(stderr, "file: unknown option '%s'\n", args[1]);
    return SHELL_STATUS_ERROR;
}

static int make_dir(const char *path)
{
    char tmp[PATH_MAX];
    if (strlen(path) >= sizeof(tmp))
    {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(tmp, path, strlen(path) + 1);

    for (char *p = tmp +1; *p != '\0'; p++)
    {
        if (*p != '/')
        {
            continue;
        }

        *p = '\0';
        if (mkdir(tmp, 0755) == -1 && errno != EEXIST)
        {
            return -1;
        }
        *p = '/';
    }

    return SHELL_STATUS_SUCCESS;
}

int file_open(const char *path)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    if (access(path, F_OK) == -1 || access(path, R_OK) == -1)
    {
        fprintf(stderr, "file: %s: %s\n", path, strerror(errno));
        return SHELL_STATUS_ERROR;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return SHELL_STATUS_ERROR;
    }

    if (pid == 0)
    {
        execlp("xdg-open", "xdg-open", path, (char*)NULL);
        perror("xdg-open");
        _exit(SHELL_STATUS_ERROR);
    }

    return SHELL_STATUS_SUCCESS;
}

int file_create(const char *path)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    if (make_dir(path) == -1)
    {
        fprintf(stderr, "file: %s: %s\n", path, strerror(errno));
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

int file_read(const char *path)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    int file = open(path, O_RDONLY);
    if (file == -1)
    {
        perror("open");
        return SHELL_STATUS_ERROR; 
    }

    char buffer[4096];
    ssize_t bytes_read;
    char last_char = '\n';

    while ((bytes_read = read(file, buffer, sizeof(buffer)))> 0)
    {
        ssize_t written = 0;
        while (written < bytes_read)
        {
            ssize_t count = write(STDOUT_FILENO, buffer + written, (size_t)(bytes_read - written));

            if (count == -1)
            {
                perror("write");
                close(file);
                return SHELL_STATUS_ERROR;
            }

            written += count;
        }

        last_char = buffer[bytes_read -1];
    }

    if (bytes_read == -1)
    {
        perror("read");
        close(file);
        return SHELL_STATUS_ERROR;
    }

    if (close(file) == -1)
    {
        perror("close");
        return SHELL_STATUS_ERROR;
    }

    if (last_char != '\n')
    {
        write(STDOUT_FILENO, "\n", 1);
    }

    return SHELL_STATUS_SUCCESS;
}

int file_delete(const char *path)
{
        if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    if (unlink(path) == -1)
    {
        perror(path);
        return SHELL_STATUS_ERROR; 
    }

    return SHELL_STATUS_SUCCESS;
}

int file_delete_directory(const char *path)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing directory path\n");
        return SHELL_STATUS_ERROR;
    }

    if (rmdir(path) == -1)
    {
        perror(path);
        return SHELL_STATUS_ERROR;
    }

    return SHELL_STATUS_SUCCESS;
}

int file_append(const char *path, const char *content)
{
    if (path == NULL || path[0] == '\0')
    {
        fprintf(stderr, "file: missing file path\n");
        return SHELL_STATUS_ERROR;
    }

    if (content == NULL)
    {
        fprintf(stderr, "file: missing content\n");
        return SHELL_STATUS_ERROR;
    }

    int file = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1)
    {
        perror(path);
        return SHELL_STATUS_ERROR;
    }

    size_t total_length = strlen(content);
    size_t total_written = 0;
    while (total_written < total_length)
    {
        ssize_t bytes_written = write(file, content + total_written, total_length - total_written);
        if (bytes_written <= 0)
        {
                if (bytes_written == -1)
                {
                perror("write");
            }
            else
            {
                fprintf(stderr, "file: could not write to file\n");
            }
            close(file);
            return SHELL_STATUS_ERROR;
        }

        total_written += (size_t)bytes_written;
    }

    if (close(file) == -1)
    {
        perror("close");
        return SHELL_STATUS_ERROR;
    }

    return SHELL_STATUS_SUCCESS;
}