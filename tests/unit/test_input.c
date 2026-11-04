#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "input.h"

int read_test_input(const char *text, char *buffer, size_t size)
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

    int result = read_input(buffer, size);
    dup2(tmp_stdin, STDIN_FILENO);
    close(tmp_stdin);
    clearerr(stdin);
    fclose(test_file);

    return result;
}

bool test_read_normal_input(void)
{
    char buffer[32];

    int result = read_test_input("hello\n", buffer, sizeof(buffer));

    return 
    result == INPUT_STATUS_SUCCESS &&
    strcmp(buffer, "hello") == 0;
}

bool test_read_empty_input(void)
{
    char buffer[32];

    int result = read_test_input("\n", buffer, sizeof(buffer));

    return
    result == INPUT_STATUS_SUCCESS &&
    strcmp(buffer, "") == 0;
}

bool test_too_long_input(void)
{
    char buffer[2];

    int result = read_test_input("hello\n", buffer, sizeof(buffer));

    return
    result == INPUT_STATUS_TOO_LONG;
}

bool test_read_eof(void)
{
    char buffer[32];

    int result = read_test_input("", buffer, sizeof(buffer));

    return
    result == INPUT_STATUS_EOF;
}

bool test_read_invalid_argument(void)
{
    char buffer[32];

    int result = read_input(NULL, sizeof(buffer));
    int result2 = read_input(buffer, 1);

    return
    result == INPUT_STATUS_ERROR &&
    result2 == INPUT_STATUS_ERROR;
}

int main(void)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    int passed = 0;
    int failed = 0;

    if (test_read_normal_input())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_read_empty_input())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_too_long_input())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_read_eof())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_read_invalid_argument())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n","Input-Tests", passed, failed);
        return 0;
    }
    else
    {
        printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n",    "Input-Tests", passed, failed);
        return 1;
    }

}