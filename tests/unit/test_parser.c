
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"

bool test_parse_simple_command(void)
{
    char line[] = "ls";
    char *args[MAX_ARGS];
    int argc = 0;

    bool check = parse_line(line, args, &argc);

    if (check && argc == 1 && strcmp(args[0], "ls") == 0)
    {
        return true;
    }

    printf("[FAIL] Simple command\n");
    return false;
}

bool test_parse_multiple_arguments(void)
{
    char line[] = "ls -l -a";
    char *args[MAX_ARGS];
    int argc = 0;

    bool check = parse_line(line, args, &argc);

    if (check &&
        argc == 3 &&
        strcmp(args[0], "ls") == 0 &&
        strcmp(args[1], "-l") == 0 &&
        strcmp(args[2], "-a") == 0)
    {
        return true;
    }

    printf("[FAIL] Multiple arguments\n");
    return false;
}

bool test_parse_empty_input(void)
{
    char line[] = "";
    char *args[MAX_ARGS];
    int argc = 0;

    bool check = parse_line(line, args, &argc);

    if (check && argc == 0)
    {
        return true;
    }

    printf("[FAIL] Empty input\n");
    return false;
}

bool test_parse_quoted_argument(void)
{
    char line[] = "echo \"Hello World\"";
    char *args[MAX_ARGS];
    int argc = 0;

    bool check = parse_line(line, args, &argc);

    if (check && argc == 2 &&
        strcmp(args[0], "echo") == 0 &&
        strcmp(args[1], "Hello World") == 0)
    {
        return true;
    }

    printf("[FAIL] Quoted argument\n");
    return false;
}

int main(void)
{
    int passed = 0;
    int failed = 0;

    if (test_parse_simple_command())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (test_parse_multiple_arguments())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (test_parse_empty_input())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (test_parse_quoted_argument())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    
    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n","Parser-Tests", passed, failed);

        return 0;
    }
    else
    {
        printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n",    "Parser-Tests", passed, failed);
    }
}
