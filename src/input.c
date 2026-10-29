#include <stdio.h>
#include <string.h>

#include "input.h"

int read_input(char *buffer, size_t size)
{
    if (buffer == NULL || size < 2)
    {
        return INPUT_STATUS_ERROR;
    }

    if (fgets(buffer, size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return INPUT_STATUS_EOF;
        }

        return INPUT_STATUS_ERROR;
    }

    if (strchr(buffer, '\n') == NULL)
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF)
        {
            while (ch != '\n' && ch != EOF)
            {
                ch = getchar();
            }

            return INPUT_STATUS_TOO_LONG;
        }

        if (ferror(stdin))
        {
            return INPUT_STATUS_ERROR;
        }
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return INPUT_STATUS_SUCCESS;

}