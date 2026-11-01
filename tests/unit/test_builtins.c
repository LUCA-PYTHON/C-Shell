#include <stdio.h>
#include <stdbool.h>

#include "builtins.h"
#include "shell_status.h"

bool test_handle_exit(void)
{

}

bool test_echo(void)
{
    int passed = 0;
    int failed = 0;

    char *args[] = {"echo", "test", NULL};
    int result = print_echo(args, 0);

    if (result == SHELL_STATUS_SUCCESS)
    {
        passed++;
    }
    else
    {
        failed++;
    }

    printf("Echo tests: %d passed, %d failed\n", passed, failed);

    return failed == 0;
}

bool test_change_directory(void)
{

}

bool test_print_working_directory(void)
{

}

bool test_export_variable(void)
{

}

bool test_printf_environment(void)
{

}

bool test_printf_help(void)
{

}

int main(void)
{
    int passed = 0;
    int failed = 0;

    if (test_handle_exit())
        passed++;
    else
        failed++;

    if (test_echo())
        passed++;
    else
        failed++;

}