
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

bool test_parse_pipeline(void)
{
    char line[] = "printf hello | tr a-z A-Z";
    Pipeline pipeline;

    bool check = parse_pipeline(line, &pipeline);

    if (check &&
        pipeline.command_count == 2 &&
        pipeline.commands[0].argc == 2 &&
        strcmp(pipeline.commands[0].args[0], "printf") == 0 &&
        strcmp(pipeline.commands[0].args[1], "hello") == 0 &&
        pipeline.commands[1].argc == 3 &&
        strcmp(pipeline.commands[1].args[0], "tr") == 0 &&
        strcmp(pipeline.commands[1].args[1], "a-z") == 0 &&
        strcmp(pipeline.commands[1].args[2], "A-Z") == 0)
    {
        return true;
    }

    printf("[FAIL] Pipeline parsing\n");
    return false;
}

bool test_parse_pipeline_redirections(void)
{
    char line[] = "cat < input.txt | wc -l > output.txt";
    Pipeline pipeline;

    bool check = parse_pipeline(line, &pipeline);

    if (check &&
        pipeline.command_count == 2 &&
        pipeline.commands[0].argc == 1 &&
        strcmp(pipeline.commands[0].args[0], "cat") == 0 &&
        strcmp(pipeline.commands[0].redirections.input_file, "input.txt") == 0 &&
        pipeline.commands[1].argc == 2 &&
        strcmp(pipeline.commands[1].args[0], "wc") == 0 &&
        strcmp(pipeline.commands[1].redirections.output_file, "output.txt") == 0)
    {
        return true;
    }

    printf("[FAIL] Pipeline redirections\n");
    return false;
}

bool test_parse_invalid_pipeline(void)
{
    char line[] = "echo hello |";
    Pipeline pipeline;

    if (!parse_pipeline(line, &pipeline))
    {
        return true;
    }

    printf("[FAIL] Invalid pipeline\n");
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

    if (test_parse_pipeline())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (test_parse_pipeline_redirections())
    {
        passed++;
    }
    else
    {
        failed++;
    }

    if (test_parse_invalid_pipeline())
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
        return 1;
    }
}
