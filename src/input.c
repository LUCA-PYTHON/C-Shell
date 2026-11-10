#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "input.h"

#define HISTORY_SIZE 50
#define INPUT_SIZE 1024
#define INITIAL_INPUT_SIZE 128
#define INPUT_GROWTH 128

static char history[HISTORY_SIZE][INPUT_SIZE];
static int history_count = 0;
static size_t previous_length = 0;
static size_t previous_cursor = 0;

static struct termios original;
static struct termios raw;

static void add_history(const char *command);
static void get_cursor_position(size_t position, size_t terminal_width, size_t *row, size_t *column);
static void redraw_input(const char *buffer, size_t length, size_t cursor, size_t prompt_column);
static int read_input_non_interactive(char **buffer, size_t *capacity);
static int read_input_interactive(char **buffer, size_t *capacity, size_t prompt_column);

static int grow_buffer(char **buffer, size_t *capacity)
{
    if (buffer == NULL || *buffer == NULL || capacity == NULL)
    {
        return INPUT_STATUS_ERROR;
    }

    size_t new_capacity = *capacity + INPUT_GROWTH;
    char *new_buffer = realloc(*buffer, new_capacity);

    if (new_buffer == NULL)
    {
        return INPUT_STATUS_ERROR;
    }

    *buffer = new_buffer;
    *capacity = new_capacity;

    return INPUT_STATUS_SUCCESS;
}

static int ensure_capacity(char **buffer, size_t *capacity, size_t required)
{
    while (*capacity < required)
    {
        if (grow_buffer(buffer, capacity) != INPUT_STATUS_SUCCESS)
        {
            return INPUT_STATUS_ERROR;
        }
    }

    return INPUT_STATUS_SUCCESS;
}

static int is_interactive(void)
{
    return isatty(STDIN_FILENO);
}

int get_terminal_width(void)
{
    struct winsize window;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == -1)
    {
        return 0;
    }

    return window.ws_col;
}

static void get_cursor_position(size_t position, size_t terminal_width, size_t *row, size_t *column)
{
    *row = position / terminal_width;
    *column = position % terminal_width;
}

static void redraw_input(const char *buffer, size_t length, size_t cursor, size_t prompt_column)
{
    if (buffer == NULL || cursor > length)
    {
        return;
    }

    int width = get_terminal_width();

    if (width <= 0)
    {
        return;
    }

    size_t terminal_width = (size_t)width;
    size_t start_row;
    size_t start_column;
    size_t old_row;
    size_t old_column;

    get_cursor_position(prompt_column, terminal_width, &start_row, &start_column);
    get_cursor_position( prompt_column + previous_cursor, terminal_width, &old_row, &old_column);

    write(STDOUT_FILENO, "\r", 1);

    if (old_row > start_row)
    {
        char sequence[32];

        int count = snprintf(
            sequence,
            sizeof(sequence),
            "\033[%zuA",
            old_row - start_row);

        if (count > 0 && (size_t)count < sizeof(sequence))
        {
            write(STDOUT_FILENO, sequence, (size_t)count);
        }
    }

    if (start_column > 0)
    {
        char sequence[32];

        int count = snprintf(
            sequence,
            sizeof(sequence),
            "\033[%zuC",
            start_column);

        if (count > 0 && (size_t)count < sizeof(sequence))
        {
            write(STDOUT_FILENO, sequence, (size_t)count);
        }
    }

    write(STDOUT_FILENO, "\033[J", 3);

    if (length > 0)
    {
        write(STDOUT_FILENO, buffer, length);
    }

    size_t end_position = prompt_column + length;
    size_t target_position = prompt_column + cursor;
    if (length > 0 && end_position % terminal_width == 0)
    {
        write(STDOUT_FILENO, "\n", 1);
    }

    size_t end_row;
    size_t end_column;
    size_t target_row;
    size_t target_column;

    get_cursor_position(end_position, terminal_width, &end_row, &end_column);
    get_cursor_position(target_position, terminal_width, &target_row, &target_column);

    write(STDOUT_FILENO, "\r", 1);

    if (end_row > target_row)
    {
        char sequence[32];

        int count = snprintf(
            sequence,
            sizeof(sequence),
            "\033[%zuA",
            end_row - target_row);

        if (count > 0 && (size_t)count < sizeof(sequence))
        {
            write(STDOUT_FILENO, sequence, (size_t)count);
        }
    }
    else if (target_row > end_row)
    {
        char sequence[32];

        int count = snprintf(
            sequence,
            sizeof(sequence),
            "\033[%zuB",
            target_row - end_row);

        if (count > 0 && (size_t)count < sizeof(sequence))
        {
            write(STDOUT_FILENO, sequence, (size_t)count);
        }
    }

    if (target_column > 0)
    {
        char sequence[32];

        int count = snprintf(
            sequence,
            sizeof(sequence),
            "\033[%zuC",
            target_column);

        if (count > 0 && (size_t)count < sizeof(sequence))
        {
            write(STDOUT_FILENO, sequence, (size_t)count);
        }
    }

    previous_length = length;
    previous_cursor = cursor;
}

int read_input(char **buffer, size_t prompt_column)
{
    if (buffer == NULL)
    {
        return INPUT_STATUS_ERROR;
    }

    *buffer = NULL;

    size_t capacity = INITIAL_INPUT_SIZE;
    *buffer = malloc(capacity);

    if (*buffer == NULL)
    {
        return INPUT_STATUS_ERROR;
    }

    (*buffer)[0] = '\0';
    int result;

    if (!is_interactive())
    {
        result = read_input_non_interactive(buffer, &capacity);
    }
    else
    {
        result = read_input_interactive(buffer, &capacity, prompt_column);
    }

    if (result != INPUT_STATUS_SUCCESS)
    {
        free(*buffer);
        *buffer = NULL;
    }

    return result;
}

static int read_input_non_interactive(char **buffer, size_t *capacity)
{
    size_t position = 0;
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
            if (position == 0)
            {
                return INPUT_STATUS_EOF;
            }

            break;
        }

        if (key == '\n')
        {
            break;
        }

        if (position + 1 >= *capacity)
        {
            if (grow_buffer(buffer, capacity) != INPUT_STATUS_SUCCESS)
            {
                return INPUT_STATUS_ERROR;
            }
        }

        (*buffer)[position] = key;
        position++;
    }

    (*buffer)[position] = '\0';

    return INPUT_STATUS_SUCCESS;
}

static int read_input_interactive(char **buffer, size_t *capacity, size_t prompt_column)
{
    if (enable_raw_mode() != INPUT_STATUS_SUCCESS)
    {
        return INPUT_STATUS_ERROR;
    }

    size_t cursor = 0;
    size_t length = 0;

    char key;

    int result = INPUT_STATUS_SUCCESS;
    int history_index = history_count;

    previous_length = 0;
    previous_cursor = 0;

    while (1)
    {
        int status = read_key(&key);

        if (status == INPUT_STATUS_ERROR)
        {
            result = INPUT_STATUS_ERROR;
            goto cleanup;
        }

        if (status == INPUT_STATUS_EOF)
        {
            result = INPUT_STATUS_EOF;
            goto cleanup;
        }

        if (key == 3)
        {
            write(STDOUT_FILENO, "\n", 1);
            (*buffer)[0] = '\0';
            result = INPUT_STATUS_INTERRUPTED;
            goto cleanup;
        }

        if (key == 27)
        {
            char sequence[2];

            if (read_key(&sequence[0]) != INPUT_STATUS_SUCCESS ||
                read_key(&sequence[1]) != INPUT_STATUS_SUCCESS)
            {
                result = INPUT_STATUS_ERROR;
                goto cleanup;
            }

            if (sequence[0] == '[')
            {
                if (sequence[1] == 'A')
                {
                    if (history_index > 0)
                    {
                        history_index--;

                        size_t new_length =
                            strlen(history[history_index]);

                        if (ensure_capacity(
                                buffer,
                                capacity,
                                new_length + 1) != INPUT_STATUS_SUCCESS)
                        {
                            result = INPUT_STATUS_ERROR;
                            goto cleanup;
                        }

                        memcpy(
                            *buffer,
                            history[history_index],
                            new_length + 1);

                        length = new_length;
                        cursor = length;

                        redraw_input(
                            *buffer,
                            length,
                            cursor,
                            prompt_column);
                    }
                }
                else if (sequence[1] == 'B')
                {
                    if (history_index < history_count)
                    {
                        history_index++;

                        length = 0;
                        cursor = 0;

                        if (history_index < history_count)
                        {
                            size_t new_length =
                                strlen(history[history_index]);

                            if (ensure_capacity(
                                    buffer,
                                    capacity,
                                    new_length + 1) != INPUT_STATUS_SUCCESS)
                            {
                                result = INPUT_STATUS_ERROR;
                                goto cleanup;
                            }

                            memcpy(
                                *buffer,
                                history[history_index],
                                new_length + 1);

                            length = new_length;
                            cursor = length;
                        }
                        else
                        {
                            (*buffer)[0] = '\0';
                        }

                        redraw_input(
                            *buffer,
                            length,
                            cursor,
                            prompt_column);
                    }
                }
                else if (sequence[1] == 'D')
                {
                    if (cursor > 0)
                    {
                        cursor--;

                        redraw_input(
                            *buffer,
                            length,
                            cursor,
                            prompt_column);
                    }
                }
                else if (sequence[1] == 'C')
                {
                    if (cursor < length)
                    {
                        cursor++;

                        redraw_input(
                            *buffer,
                            length,
                            cursor,
                            prompt_column);
                    }
                }
            }

            continue;
        }

        if (key == '\n' || key == '\r')
        {
            if (cursor != length)
            {
                cursor = length;
                redraw_input(*buffer, length, cursor, prompt_column);
            }

            int width = get_terminal_width();
            int already_wrapped = width > 0 && length > 0 && (prompt_column + length) % (size_t)width == 0;

            if (!already_wrapped)
            {
                write(STDOUT_FILENO, "\n", 1);
            }
            break;
        }

        if (key == 4)
        {
            if (length == 0)
            {
                write(STDOUT_FILENO, "\n", 1);
                result = INPUT_STATUS_EOF;
                goto cleanup;
            }

            if (cursor < length)
            {
                memmove(&(*buffer)[cursor],  &(*buffer)[cursor + 1], length - cursor);
                length--;
                redraw_input(*buffer, length, cursor, prompt_column);
            }

            continue;
        }

        if (key == 127 || key == 8)
        {
            if (cursor > 0)
            {
                memmove(
                    &(*buffer)[cursor - 1],
                    &(*buffer)[cursor],
                    length - cursor + 1);

                cursor--;
                length--;

                redraw_input(
                    *buffer,
                    length,
                    cursor,
                    prompt_column);
            }

            continue;
        }

        if ((unsigned char)key < 32)
        {
            continue;
        }

        if (length + 1 >= *capacity)
        {
            if (grow_buffer(buffer, capacity) != INPUT_STATUS_SUCCESS)
            {
                result = INPUT_STATUS_ERROR;
                goto cleanup;
            }
        }

        memmove(
            &(*buffer)[cursor + 1],
            &(*buffer)[cursor],
            length - cursor + 1);

        (*buffer)[cursor] = key;

        length++;
        cursor++;

        redraw_input(
            *buffer,
            length,
            cursor,
            prompt_column);
    }

    (*buffer)[length] = '\0';

    add_history(*buffer);

cleanup:

    if (disable_raw_mode() != INPUT_STATUS_SUCCESS)
    {
        return INPUT_STATUS_ERROR;
    }

    return result;
}

int enable_raw_mode(void)
{
    int result = tcgetattr(STDIN_FILENO, &original);

    if (result == -1)
    {
        perror("tcgetattr");
        return INPUT_STATUS_ERROR;
    }

    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);

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
