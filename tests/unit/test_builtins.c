#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "builtins.h"
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

static void test_handle_exit(void)
{
    char *args_one[] = {"exit", NULL};
    int status_one = -1;
    int result_one = handle_exit(1, args_one, &status_one);
    check("exit: ohne Argument -> Status 0",
          result_one == SHELL_STATUS_SUCCESS && status_one == 0);

    char *args_two[] = {"exit", "5", NULL};
    int status_two = -1;
    int result_two = handle_exit(2, args_two, &status_two);
    check("exit: Status 5",
          result_two == SHELL_STATUS_SUCCESS && status_two == 5);

    char *args_three[] = {"exit", "255", NULL};
    int status_three = -1;
    int result_three = handle_exit(2, args_three, &status_three);
    check("exit: Status 255 (Obergrenze)",
          result_three == SHELL_STATUS_SUCCESS && status_three == 255);

    char *args_four[] = {"exit", "abc", NULL};
    int status_four = -1;
    int result_four = handle_exit(2, args_four, &status_four);
    check("exit: keine Zahl -> Fehler", result_four == SHELL_STATUS_ERROR);

    char *args_five[] = {"exit", "1", "2", NULL};
    int status_five = -1;
    int result_five = handle_exit(3, args_five, &status_five);
    check("exit: zu viele Argumente -> Fehler", result_five == SHELL_STATUS_ERROR);

    char *args_six[] = {"exit", "256", NULL};
    int status_six = -1;
    int result_six = handle_exit(2, args_six, &status_six);
    check("exit: 256 ist zu gross -> Fehler", result_six == SHELL_STATUS_ERROR);

    char *args_seven[] = {"exit", "-1", NULL};
    int status_seven = -1;
    int result_seven = handle_exit(2, args_seven, &status_seven);
    check("exit: negative Zahl -> Fehler", result_seven == SHELL_STATUS_ERROR);
}

static void test_echo(void)
{
    char *args_one[] = {"echo", "test", NULL};
    check("echo: ein Argument",
          print_echo(args_one, 0) == SHELL_STATUS_SUCCESS);

    char *args_two[] = {"echo", "\"Hello World\"", NULL};
    check("echo: Anfuehrungszeichen",
          print_echo(args_two, 0) == SHELL_STATUS_SUCCESS);

    char *args_three[] = {"echo", NULL};
    check("echo: ohne Argumente",
          print_echo(args_three, 0) == SHELL_STATUS_SUCCESS);
}

static void test_change_directory(void)
{
    char original_directory[1024];
    char current_directory[1024];

    if (getcwd(original_directory, sizeof(original_directory)) == NULL)
    {
        check("cd: getcwd vorher", false);
        return;
    }

    int result = change_directory("/tmp");
    bool success =
        result == SHELL_STATUS_SUCCESS &&
        getcwd(current_directory, sizeof(current_directory)) != NULL &&
        strcmp(current_directory, "/tmp") == 0;

    if (chdir(original_directory) == -1)
    {
        perror("chdir");
    }

    check("cd: wechselt nach /tmp", success);
}

static void test_print_working_directory(void)
{
    char current_directory[1024];

    if (getcwd(current_directory, sizeof(current_directory)) == NULL)
    {
        check("pwd: getcwd", false);
        return;
    }

    check("pwd: Rueckgabewert",
          print_working_directory(current_directory) == SHELL_STATUS_SUCCESS);
}

static void test_export_variable(void)
{
    char argument[] = "CSHELL_TEST=test";
    char *args[] = {"export", argument, NULL};

    int result = export_variable(2, args);
    char *value = getenv("CSHELL_TEST");

    check("export: Variable gesetzt",
          result == SHELL_STATUS_SUCCESS &&
          value != NULL &&
          strcmp(value, "test") == 0);

    unsetenv("CSHELL_TEST");
}

static void test_print_environment(void)
{
    char *args[] = {"env", NULL};

    if (getenv("PATH") == NULL)
    {
        check("env: PATH ist gesetzt", false);
        return;
    }

    check("env: Rueckgabewert",
          print_environment(1, args) == SHELL_STATUS_SUCCESS);
}

int main(void)
{
    test_handle_exit();
    test_echo();
    test_change_directory();
    test_print_working_directory();
    test_export_variable();
    test_print_environment();

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "Builtin-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "Builtin-Tests", passed, failed);
    return 1;
}