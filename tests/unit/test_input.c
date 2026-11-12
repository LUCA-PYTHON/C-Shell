#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"

static int passed = 0, failed = 0;

static void check(const char *name, bool ok)
{
    if (ok)
    {
        passed++;
    }
    else
    {
        failed++;
        printf("  [FAIL] %s\n", name);
    }
}

static int read_test_input(const char *text, char **buffer)
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

static void test_read_normal_input(void)
{
    char *buffer = NULL;

    int result = read_test_input("hello\n", &buffer);

    check("input: normale Zeile",
          result == INPUT_STATUS_SUCCESS &&
          buffer != NULL &&
          strcmp(buffer, "hello") == 0);

    free(buffer);
}

static void test_read_empty_input(void)
{
    char *buffer = NULL;

    int result = read_test_input("\n", &buffer);

    check("input: leere Zeile",
          result == INPUT_STATUS_SUCCESS &&
          buffer != NULL &&
          strcmp(buffer, "") == 0);

    free(buffer);
}

static void test_read_without_newline(void)
{
    char *buffer = NULL;

    int result = read_test_input("hello", &buffer);

    check("input: ohne Zeilenumbruch am Ende",
          result == INPUT_STATUS_SUCCESS &&
          buffer != NULL &&
          strcmp(buffer, "hello") == 0);

    free(buffer);
}

static void test_read_long_input(void)
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

    check("input: lange Zeile (1000 Zeichen)", ok);

    free(buffer);
}

static void test_read_eof(void)
{
    char *buffer = NULL;

    int result = read_test_input("", &buffer);

    check("input: EOF", result == INPUT_STATUS_EOF && buffer == NULL);
}

static void test_read_invalid_argument(void)
{
    check("input: NULL-Puffer -> Fehler",
          read_input(NULL, 0) == INPUT_STATUS_ERROR);
}

int main(void)
{
    setvbuf(stdin, NULL, _IONBF, 0);

    test_read_normal_input();
    test_read_empty_input();
    test_read_without_newline();
    test_read_long_input();
    test_read_eof();
    test_read_invalid_argument();

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "Input-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "Input-Tests", passed, failed);
    return 1;
}