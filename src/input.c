#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "input.h"

#define HISTORY_SIZE 50
#define INPUT_SIZE 1024
static char history[HISTORY_SIZE][INPUT_SIZE];
static int history_count = 0;
static void add_history(const char *command);
static int is_interactive(void)
{
    return isatty(STDIN_FILENO);
}
static int read_input_non_interactive(char *buffer, size_t size)
{
    size_t position = 0;
    int input_too_long = 0;
    char key;

    while (1)
    {
        int status = read_key(&key);
        if (status == INPUT_STATUS_ERROR)
        {
            return INPUT_STATUS_ERROR;
        }
        else if (status == INPUT_STATUS_EOF)
        {
            if (position == 0 && !input_too_long)
            {
                return INPUT_STATUS_EOF;
            }

            break;
        }

        if (key == '\n')
        {
            break;
        }

        if (position < size -1 && !input_too_long)
        {
            buffer[position] = key;
            position++;
        }
        else
        {
            input_too_long = 1;
        }
    }

    buffer[position] = '\0';

    if (input_too_long)
    {
        return INPUT_STATUS_TOO_LONG;
    }

    return INPUT_STATUS_SUCCESS;
}

int read_input(char *buffer, size_t size)
{
    if (buffer == NULL || size < 2)
    {
        return INPUT_STATUS_ERROR;
    }

    if (!is_interactive())
    {
        return read_input_non_interactive(buffer, size);
    }

    if (enable_raw_mode() != INPUT_STATUS_SUCCESS)
    {
        return INPUT_STATUS_ERROR;
    }

    size_t position = 0;
    char key;
    int result = INPUT_STATUS_SUCCESS;
    int input_too_long = 0;
    int history_index = history_count;

    while (1)
    {
        int status = read_key(&key);

        if (status == INPUT_STATUS_ERROR)
        {
            result = INPUT_STATUS_ERROR;
            goto cleanup;
        }
        else if (status == INPUT_STATUS_EOF)
        {
            result = INPUT_STATUS_EOF;
            goto cleanup;
        }

        if (key == 27)
        {
            char sequence[2];

            int first_status = read_key(&sequence[0]);

            if (first_status != INPUT_STATUS_SUCCESS)
            {
                result = first_status;
                goto cleanup;
            }

            int second_status = read_key(&sequence[1]);

            if (second_status != INPUT_STATUS_SUCCESS)
            {
                result = second_status;
                goto cleanup;
            }

            if (sequence[0] == '[')
            {
                if (sequence[1] == 'A')
                {
                    if (history_index > 0)
                    {
                        history_index--;

                        while (position > 0)
                        {
                            write(STDOUT_FILENO, "\b \b", 3);
                            position--;
                        }

                        snprintf(buffer, size, "%s", history[history_index]);

                        position = strlen(buffer);
                        write(STDOUT_FILENO, buffer, position);
                    }
                }
                else if (sequence[1] == 'B')
                {
                    if (history_index < history_count)
                    {
                        history_index++;

                        while (position > 0)
                        {
                            write(STDOUT_FILENO, "\b \b", 3);
                            position--;
                        }

                        if (history_index < history_count)
                        {
                            snprintf(buffer, size, "%s", history[history_index]);
                            position = strlen(buffer);
                            write(STDOUT_FILENO, buffer, position);
                        }
                        else
                        {
                            buffer[0] = '\0';
                        }
                    }
                }
            }

            continue;
        }
        else if (key == '\n')
        {
            write(STDOUT_FILENO, "\n", 1);
            break;
        }
        else if (key == 127 || key == 8)
        {
            if (position > 0)
            {
                position--;
                buffer[position] = '\0';
                write(STDOUT_FILENO, "\b \b", 3);
            }
            continue;
        }

        if (position < size - 1)
        {
            if (!input_too_long)
            {
                buffer[position] = key;
                position++;

                write(STDOUT_FILENO, &key, 1);
            }
        }
        else
        {
            input_too_long = 1;
        }
    }

    if (input_too_long)
    {
        result = INPUT_STATUS_TOO_LONG;
    }

    buffer[position] = '\0';

    if (result == INPUT_STATUS_SUCCESS)
    {
        add_history(buffer);
    }

cleanup:
    if (disable_raw_mode() != INPUT_STATUS_SUCCESS)
    {
        return INPUT_STATUS_ERROR;
    }

    return result;
}

struct termios original;
struct termios raw;
int enable_raw_mode(void)
{
    int result = tcgetattr(STDIN_FILENO, &original);
    if (result == -1)
    {
        perror("tcgatatter");
        return INPUT_STATUS_ERROR;
    }

    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);

    int changed = tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    if (changed == -1)
    {
        perror("tcsetattr");
        return INPUT_STATUS_ERROR;
    }

    return INPUT_STATUS_SUCCESS;
}

int disable_raw_mode(void)
{
    int changed = tcsetattr(STDIN_FILENO, TCSANOW, &original);
    if (changed == -1)
    {
        perror("tcsetattr");
        return INPUT_STATUS_ERROR;
    }

    return INPUT_STATUS_SUCCESS;
}

int read_key(char *key)
{
    if (key == NULL)
    {
        return INPUT_STATUS_ERROR;
    }
    ssize_t bytes_read = read(STDIN_FILENO, key, 1);
    if (bytes_read == -1)
    {
        perror("read");
        return INPUT_STATUS_ERROR;
    }
    else if (bytes_read == 0)
    {
        return INPUT_STATUS_EOF;
    }

    return INPUT_STATUS_SUCCESS;
}

static void add_history(const char *command)
{
    if (command == NULL || command[0] == '\0')
    {
        return;
    }

    if (HISTORY_SIZE > history_count)
    {
        snprintf(history[history_count], INPUT_SIZE, "%s", command);
        history_count++;
    }

    else if (history_count == HISTORY_SIZE)
    {
        for (int i = 0; i < HISTORY_SIZE - 1; i++)
        {
            memmove(history[i], history[i + 1], INPUT_SIZE);
        }

        snprintf(history[HISTORY_SIZE - 1], INPUT_SIZE, "%s", command);
    }
}