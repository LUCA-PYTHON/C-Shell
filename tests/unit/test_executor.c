#include <stdio.h>
#include <stdbool.h>

#include "executor.h"
#include "shell_status.h"

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

static void test_execute_success(void)
{
    char *args[] = {"/bin/true", NULL};

    check("execute: Erfolg -> Status 0",
          execute_external_command(args, NULL) == SHELL_STATUS_SUCCESS);
}

static void test_execute_exit_status(void)
{
    char *args[] = {"/bin/sh", "-c", "exit 5", NULL};

    check("execute: Exit-Status 5 wird durchgereicht",
          execute_external_command(args, NULL) == 5);
}

static void test_execute_command_not_found(void)
{
    char *args[] = {"not_exist", NULL};

    check("execute: unbekannter Befehl -> 127",
          execute_external_command(args, NULL) == SHELL_STATUS_COMMAND_NOT_FOUND);
}

int main(void)
{
    test_execute_success();
    test_execute_exit_status();
    test_execute_command_not_found();

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "Execute-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "Execute-Tests", passed, failed);
    return 1;
}