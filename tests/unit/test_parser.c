#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"

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

static void test_parse_simple_command(void)
{
    char line[] = "ls";
    char *args[MAX_ARGS];
    int argc = 0;

    bool ok = parse_line(line, args, &argc);

    check("parse_line: einfacher Befehl",
          ok && argc == 1 && strcmp(args[0], "ls") == 0);
}

static void test_parse_multiple_arguments(void)
{
    char line[] = "ls -l -a";
    char *args[MAX_ARGS];
    int argc = 0;

    bool ok = parse_line(line, args, &argc);

    check("parse_line: mehrere Argumente",
          ok &&
          argc == 3 &&
          strcmp(args[0], "ls") == 0 &&
          strcmp(args[1], "-l") == 0 &&
          strcmp(args[2], "-a") == 0);
}

static void test_parse_empty_input(void)
{
    char line[] = "";
    char *args[MAX_ARGS];
    int argc = 0;

    bool ok = parse_line(line, args, &argc);

    check("parse_line: leere Eingabe", ok && argc == 0);
}

static void test_parse_quoted_argument(void)
{
    char line[] = "echo \"Hello World\"";
    char *args[MAX_ARGS];
    int argc = 0;

    bool ok = parse_line(line, args, &argc);

    check("parse_line: Argument in Anfuehrungszeichen",
          ok &&
          argc == 2 &&
          strcmp(args[0], "echo") == 0 &&
          strcmp(args[1], "Hello World") == 0);
}

static void test_parse_unclosed_quote(void)
{
    char line[] = "echo \"Hello";
    char *args[MAX_ARGS];
    int argc = 0;

    check("parse_line: offenes Anfuehrungszeichen -> Fehler",
          !parse_line(line, args, &argc));
}

static void test_parse_pipeline(void)
{
    char line[] = "printf hello | tr a-z A-Z";
    Pipeline pipeline;

    bool ok = parse_pipeline(line, &pipeline);

    check("parse_pipeline: zwei Befehle",
          ok &&
          pipeline.command_count == 2 &&
          pipeline.commands[0].argc == 2 &&
          strcmp(pipeline.commands[0].args[0], "printf") == 0 &&
          strcmp(pipeline.commands[0].args[1], "hello") == 0 &&
          pipeline.commands[1].argc == 3 &&
          strcmp(pipeline.commands[1].args[0], "tr") == 0 &&
          strcmp(pipeline.commands[1].args[1], "a-z") == 0 &&
          strcmp(pipeline.commands[1].args[2], "A-Z") == 0);
}

static void test_parse_pipeline_redirections(void)
{
    char line[] = "cat < input.txt | wc -l > output.txt";
    Pipeline pipeline;

    bool ok = parse_pipeline(line, &pipeline);

    check("parse_pipeline: Umleitungen",
          ok &&
          pipeline.command_count == 2 &&
          pipeline.commands[0].argc == 1 &&
          strcmp(pipeline.commands[0].args[0], "cat") == 0 &&
          pipeline.commands[0].redirections.input_file != NULL &&
          strcmp(pipeline.commands[0].redirections.input_file, "input.txt") == 0 &&
          pipeline.commands[1].argc == 2 &&
          strcmp(pipeline.commands[1].args[0], "wc") == 0 &&
          pipeline.commands[1].redirections.output_file != NULL &&
          strcmp(pipeline.commands[1].redirections.output_file, "output.txt") == 0);
}

static void test_parse_invalid_pipeline(void)
{
    char line[] = "echo hello |";
    Pipeline pipeline;

    check("parse_pipeline: Pipe ohne zweiten Befehl -> Fehler",
          !parse_pipeline(line, &pipeline));
}

int main(void)
{
    test_parse_simple_command();
    test_parse_multiple_arguments();
    test_parse_empty_input();
    test_parse_quoted_argument();
    test_parse_unclosed_quote();
    test_parse_pipeline();
    test_parse_pipeline_redirections();
    test_parse_invalid_pipeline();

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "Parser-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "Parser-Tests", passed, failed);
    return 1;
}