#include <stdio.h>
#include <stdbool.h>

#include "executor.h"
#include "shell_status.h"

bool test_execute_success(void)
{
    char *args[] = {"/bin/true", NULL};

    int result = execute_external_command(args, NULL);

    return result == SHELL_STATUS_SUCCESS;
}

bool test_execute_exit_status(void)
{
    char *args[] = {"/bin/sh", "-c", "exit 5", NULL};

    int result = execute_external_command(args, NULL);

    return result == 5;
}

bool test_execute_command_not_found(void)
{
    char *args[] = {"not_exist", NULL};

    int result = execute_external_command(args, NULL);

    return result == SHELL_STATUS_COMMAND_NOT_FOUND;
}

int main(void)
{
    int passed = 0;
    int failed = 0;

    if (test_execute_success())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_execute_exit_status())
    {
        passed++;
    }
    else
    {
        failed++;
    }
    if (test_execute_command_not_found())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n","Execute-Tests", passed, failed);
        return 0;
    }
    else
    {
        printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n",    "Execute-Tests", passed, failed);
        return 1;
    }

}