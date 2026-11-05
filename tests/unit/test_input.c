#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"

int read_test_input(const char *text, char **buffer)
{
    FILE *test_file = tmpfile();
    if (test_file == NULL)
    {
        return INPUT_STATUS_ERROR;
    }

    fputs(text, test_file);
    rewind(test_file);

    int tmp_stdin = dup(STDIN_FILENO);
    if (tmp_stdin == -1)
    {
        fclose(test_file);
        return INPUT_STATUS_ERROR;
    }

    if (dup2(fileno(test_file), STDIN_FILENO) == -1)
    {
        close(tmp_stdin);
        fclose(test_file);
        return INPUT_STATUS_ERROR;
    }

    clearerr(stdin);

    int result = read_input(buffer, 0);

    dup2(tmp_stdin, STDIN_FILENO);
    close(tmp_stdin);
    clearerr(stdin);
    fclose(test_file);

    return result;
}

bool test_read_normal_input(void)
{
    char *buffer = NULL;

    int result = read_test_input("hello\n", &buffer);
    bool ok = result == INPUT_STATUS_SUCCESS &&
              buffer != NULL &&
              strcmp(buffer, "hello") == 0;

    free(buffer);
    return ok;
}

bool test_read_empty_input(void)
{
    char *buffer = NULL;

    int result = read_test_input("\n", &buffer);
    bool ok = result == INPUT_STATUS_SUCCESS &&
              buffer != NULL &&
              strcmp(buffer, "") == 0;

    free(buffer);
    return ok;
}

bool test_read_without_newline(void)
{
    char *buffer = NULL;

    int result = read_test_input("hello", &buffer);
    bool ok = result == INPUT_STATUS_SUCCESS &&
              buffer != NULL &&
              strcmp(buffer, "hello") == 0;

    free(buffer);
    return ok;
}

bool test_read_long_input(void)
{
    enum { LENGTH = 1000 };
    char text[LENGTH + 2];

    memset(text, 'a', LENGTH);
    text[LENGTH] = '\n';
    text[LENGTH + 1] = '\0';

    char *buffer = NULL;
    int result = read_test_input(text, &buffer);

    bool ok = result == INPUT_STATUS_SUCCESS &&
              buffer != NULL &&
              strlen(buffer) == LENGTH;

    for (size_t i = 0; ok && i < LENGTH; i++)
    {
        if (buffer[i] != 'a')
        {
            ok = false;
        }
    }

    free(buffer);
    return ok;
}

bool test_read_eof(void)
{
    char *buffer = NULL;

    int result = read_test_input("", &buffer);

    return result == INPUT_STATUS_EOF &&
           buffer == NULL;
}

bool test_read_invalid_argument(void)
{
    return read_input(NULL, 0) == INPUT_STATUS_ERROR;
}

int main(void)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    int passed = 0;
    int failed = 0;

    bool (*tests[])(void) =
    {
        test_read_normal_input,
        test_read_empty_input,
        test_read_without_newline,
        test_read_long_input,
        test_read_eof,
        test_read_invalid_argument
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
        if (tests[i]())
        {
            passed++;
        }
        else
        {
            failed++;
        }
    }

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "Input-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "Input-Tests", passed, failed);
    return 1;
}