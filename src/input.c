#include <stdio.h>
#include <string.h>

#include "input.h"
#include "shell_status.h"

int read_input(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        return SHELL_STATUS_ERROR;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return SHELL_STATUS_SUCCESS;
}