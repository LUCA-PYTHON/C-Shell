#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "shell_status.h"

int main(void)
{
    char line[] = "ls -l";
    char *args[MAX_ARGS];
    int argc = 0;

    bool ok = parse_line(line, args, &argc);

    if (ok && argc == 2 &&
        strcmp(args[0], "ls") == 0 &&
        strcmp(args[1], "-l") == 0)
    {
        printf("Test bestanden\n");
        return SHELL_STATUS_SUCCESS;
    }

    printf("Test fehlgeschlagen\n");
    return SHELL_STATUS_ERROR
}