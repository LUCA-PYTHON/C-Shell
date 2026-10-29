#include <stdio.h>
#include <string.h>

#include "input.h"
#include "shell_status.h"

int read_input(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return INPUT_STATUS_EOF;
        }

        return INPUT_STATUS_ERROR;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return INPUT_STATUS_SUCCESS;
}