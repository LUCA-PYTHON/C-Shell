#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "builtins.h"
#include "shell_status.h"

bool test_handle_exit(void)
{

    char *args_one[] = {"exit", NULL};
    int status_one = -1;
    int result_one = handle_exit(1, args_one, &status_one);

    char *args_two[] = {"exit", "5", NULL};
    int status_two = -1;
    int result_two = handle_exit(2, args_two, &status_two);

    char *args_three[] = {"exit", "255", NULL};
    int status_three = -1;
    int result_three = handle_exit(2, args_three, &status_three);

    char *args_four[] = {"exit", "abc", NULL};
    int status_four = -1;
    int result_four = handle_exit(2, args_four, &status_four);

    char *args_five[] = {"exit", "1", "2", NULL};
    int status_five = -1;
    int result_five = handle_exit(3, args_five, &status_five);

    return 
    result_one == SHELL_STATUS_SUCCESS && status_one == 0 &&
    result_two == SHELL_STATUS_SUCCESS && status_two == 5 &&
    result_three == SHELL_STATUS_SUCCESS && status_three == 255 &&
    result_four == SHELL_STATUS_ERROR &&
    result_five == SHELL_STATUS_ERROR;

}

bool test_echo(void)
{

    char *args_one[] = {"echo", "test", NULL};
    int result_one = print_echo(args_one, 0);

    char *args_two[] = {"echo", "\"Hello World\"", NULL};
    int result_two = print_echo(args_two, 0);

    char *args_three[] = {"echo", NULL};
    int result_three = print_echo(args_three, 0);

    return 
    result_one == SHELL_STATUS_SUCCESS && 
    result_two == SHELL_STATUS_SUCCESS && 
    result_three == SHELL_STATUS_SUCCESS;

}

bool test_change_directory(void)
{
    return false;
}

bool test_print_working_directory(void)
{
    return false;
}

bool test_export_variable(void)
{
    return false;
}

bool test_print_environment(void)
{
    return false;
}

bool test_print_help(void)
{
    return false;
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

    printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n",
       "Builtin-Tests", passed, failed);
    return failed == 0 ? 0 : 1;

}